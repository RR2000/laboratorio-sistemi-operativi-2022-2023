#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <time.h>
#include <unistd.h>

void
offer_demand_generator(Port port, Lot_array *offer, Lot_array *demand, int SO_MERCI,
                       int SO_MIN_VITA,
                       int SO_MAX_VITA, int SO_CAPACITY) {

  unsigned int state = time(NULL) ^ getpid();
  int i, sum, max_step, min_step, has_offer = 0, has_demand = 0;

  int *is_offer_array = calloc(SO_MERCI, sizeof(int));

  for (i = 0; i < SO_MERCI; i++) {
    int coin = get_random_int(0, 1, &state);
    if (coin) {
      has_offer += 1;
    } else {
      has_demand += 1;
    }
    is_offer_array[i] = coin;
  }

  sum = 0;
  i = 0;

  min_step = 1;
  max_step = SO_CAPACITY;

  while (sum < port.initial_offer_tons && has_offer > 0 && offer->size < MAX_LOT_ARRAY - 1) {
    if (is_offer_array[i] == 1) {
      Lot element;
      element.good_id = i;
      element.expire_in = get_random_int(SO_MIN_VITA, SO_MAX_VITA, &state);
      element.quantity = get_random_int(min_step, max_step, &state);
      add_lot_array_element(offer, element);
      sum += element.quantity;
    }
    i = get_random_int(0, SO_MERCI, &state);
  }

  if (offer->size == MAX_LOT_ARRAY - 1)
    fprintf(stdout, "ATTENZIONE! L'ARRAY DEL PORTO %d E' STATO RIEMPITO!\n AUMENTARE SO_CAPACITY!\n", port.port_id);

  sum = 0;
  i = 0;

  min_step = 1;
  max_step = SO_CAPACITY;


  while (i < SO_MERCI && has_demand > 0) {
    if (!is_offer_array[i]) {
      Lot element;
      element.good_id = i;
      element.expire_in = -1;
      element.quantity = 0;
      add_lot_array_element(demand, element);
      sum += element.quantity;
    }
    i = i + 1;
  }

  i = 0;
  while (sum < port.initial_demand_tons && has_demand > 0) {
    Lot *element = &demand->lots[i];
    sum -= element->quantity;
    element->quantity += get_random_int(min_step, max_step, &state);
    sum += element->quantity;
    i = get_random_int(0, demand->size, &state);
  }

  free(is_offer_array);
}

int main(int argc, char *argv[]) {
  int port_id = (int) strtol(argv[0], NULL, 10);
  int SO_MERCI = (int) strtol(argv[1], NULL, 10);
  int SO_CAPACITY = (int) strtol(argv[2], NULL, 10);
  int SO_MIN_VITA = (int) strtol(argv[3], NULL, 10);
  int SO_MAX_VITA = (int) strtol(argv[4], NULL, 10);
  int total_processes = (int) strtol(argv[5], NULL, 10);
  int barrier_semaphore_id = (int) strtol(argv[6], NULL, 10);

  int i, j, k;
  Port *this_port = &get_shared_port_array()[port_id];
  Ship *ships = get_shared_ship_array();

  Lot_array *port_demand, *port_offer, *port_shipped, *port_received, *ship_cargo;
  port_demand = &this_port->demand;
  port_offer = &this_port->offer;
  port_shipped = &this_port->shipped;
  port_received = &this_port->received;

  port_demand->size = 0;
  port_offer->size = 0;
  port_shipped->size = 0;
  port_received->size = 0;

  offer_demand_generator(*this_port, &this_port->offer, &this_port->demand, SO_MERCI, SO_MIN_VITA, SO_MAX_VITA,
                         SO_CAPACITY);

  unlock_ledger(barrier_semaphore_id);
  while (get_value_mutex(barrier_semaphore_id) < total_processes);

  while (1) {
    int ship_id = -1;
    int tons_moved = 0;
    Ship *docked_ship;

    ship_id = receive_docked_ship_message(this_port->port_id);

    lock_ledger(port_id);

    docked_ship = &ships[ship_id];
    docked_ship->state = DOCKED;

    ship_cargo = &docked_ship->cargo;

    if (ship_cargo->size > 0) {
      for (i = 0; i < port_demand->size; i++) {
        Lot *good_demand = &port_demand->lots[i];
        for (j = 0; j < ship_cargo->size; j++) {
          Lot *this_ship_lot = &ship_cargo->lots[j];
          if (good_demand->good_id == this_ship_lot->good_id && this_ship_lot->expire_in > 0) {
            Lot extracted_from_ship = extract_lot_array_element(ship_cargo, j);
            good_demand->quantity -= extracted_from_ship.quantity;
            docked_ship->loaded_tons -= extracted_from_ship.quantity;
            tons_moved += extracted_from_ship.quantity;

            add_lot_array_element(port_received, extracted_from_ship);
            if (good_demand->quantity <= 0) {
              extract_lot_array_element(port_demand, i);
              break;
            }
          }
        }

      }
    }

    /*Add things on the ship*/
    i = 0;
    while (this_port->offer.size > 0 && i < this_port->offer.size) {
      Lot load_this = get_lot_array_element(&this_port->offer, i);

      if (load_this.quantity + docked_ship->loaded_tons > SO_CAPACITY)
        break;

      if (load_this.expire_in <= 0) {
        i++;
      } else {
        Lot extracted_from_port = extract_lot_array_element(&this_port->offer, i);
        add_lot_array_element(&docked_ship->cargo, extracted_from_port);
        docked_ship->loaded_tons += extracted_from_port.quantity;
        tons_moved += extracted_from_port.quantity;
        add_lot_array_element(port_shipped, extracted_from_port);
      }
    }

    unlock_ledger(port_id);

    send_tons_loaded_message(this_port->port_id, tons_moved);
  }

  exit(EXIT_SUCCESS);
}

