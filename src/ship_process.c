#include <stdlib.h>
#include "utils.h"
#include <time.h>
#include <unistd.h>
#include <math.h>

double distance_between(Ship ship, Port port) {
  return sqrt(abs(ship.x - port.x) * abs(ship.x - port.x) + abs(ship.y - port.y) * abs(ship.y - port.y));
}

void move_ship(Ship *ship, Port port, int SO_SPEED) {
  struct timespec *sleep_per_day = malloc(sizeof(struct timespec));
  double sleep_per_day_nsec;

  sleep_per_day_nsec = distance_between(*ship, port) / SO_SPEED;
  sleep_per_day->tv_sec = (int) sleep_per_day_nsec;
  sleep_per_day->tv_nsec = (long) ((sleep_per_day_nsec - (int) sleep_per_day_nsec) * 1000000000);

  nanosleep(sleep_per_day, NULL);

  ship->x = port.x;
  ship->y = port.y;
}

int position_is_equal(Ship ship, Port port) {
  if (ship.x == port.x && ship.y == port.y)
    return 1;
  else
    return 0;
}


Port *choose_destination_port(Port *ports, Ship ship, int SO_PORTI, unsigned int *state) {
  return &ports[get_random_int(0, SO_PORTI - 1, state)];
}

int main(int argc, char *argv[]) {
  int ship_id = (int) strtol(argv[0], NULL, 10);
  int SO_PORTI = (int) strtol(argv[1], NULL, 10);
  int SO_SPEED = (int) strtol(argv[2], NULL, 10);
  int SO_LATO = (int) strtol(argv[3], NULL, 10);
  int SO_LOADSPEED = (int) strtol(argv[4], NULL, 10);
  int total_processes = (int) strtol(argv[5], NULL, 10);
  int barrier_semaphore_id = (int) strtol(argv[6], NULL, 10);
  int c = 0, moved_tons = 0;

  struct timespec sleep_per_day;
  double sleep_per_day_nsec;

  Port *ports = get_shared_port_array();
  Ship *ships = get_shared_ship_array();

  unsigned int state = time(NULL) ^ getpid();

  Port *dest_port = NULL;
  Ship *ship = &ships[ship_id];
  ship->x = get_random_int(0, SO_LATO - 1, &state);
  ship->y = get_random_int(0, SO_LATO - 1, &state);
  ship->state = EMPTY_SEA;
  ship->loaded_tons = 0;

  dest_port = choose_destination_port(ports, *ship, SO_PORTI, &state);


  unlock_ledger(barrier_semaphore_id);
  while (get_value_mutex(barrier_semaphore_id) < total_processes);


  while (1) {
    if (position_is_equal(*ship, *dest_port)) {
      dock_at_the_port(dest_port->port_id);
      ship->state = DOCKED;

      send_docked_ship_message(dest_port->port_id, ship_id);

      moved_tons = receive_tons_loaded_message(dest_port->port_id);

      if (moved_tons > 0) {

        sleep_per_day_nsec = ((double) moved_tons / (double) SO_LOADSPEED);
        sleep_per_day.tv_sec = (int) sleep_per_day_nsec;
        sleep_per_day.tv_nsec = (long) ((sleep_per_day_nsec - (int) sleep_per_day_nsec) * 1000000000);

        nanosleep(&sleep_per_day, NULL);
      }

      undock_from_the_port(dest_port->port_id);
      if (ship->cargo.size > 0)
        ship->state = FULL_SEA;
      else
        ship->state = EMPTY_SEA;

      dest_port = choose_destination_port(ports, *ship, SO_PORTI, &state);
    } else {
      move_ship(ship, *dest_port, SO_SPEED);
    }
    c++;
  }

  exit(EXIT_SUCCESS);
}

