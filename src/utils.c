#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include "utils.h"

struct message {
  long mtype;
  int value;
};

void create_queues(void)
{
  int queue_id;

  if ((queue_id = msgget(DOCKED_MESSAGE, 0666 | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  if ((queue_id = msgget(LOADED_LOTS_MESSAGE, 0666 | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

}

void send_tons_loaded_message(long port_id, int tons)
{
  int queue_id;
  struct message msg;

  if ((queue_id = msgget(DOCKED_MESSAGE, 0)) == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  msg.mtype = port_id + 1;
  msg.value = tons;

  if (msgsnd(queue_id, &msg, sizeof(struct message), 0) == -1) {
    perror("msgsnd");
    exit(EXIT_FAILURE);
  }
}

int receive_tons_loaded_message(long port_id)
{
  int queue_id;
  struct message msg;

  if ((queue_id = msgget(DOCKED_MESSAGE, 0)) == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  if (msgrcv(queue_id, &msg, sizeof(struct message), port_id + 1, 0) == -1) {
    perror("msgrcv");
    exit(EXIT_FAILURE);
  }

  return msg.value;
}


void send_docked_ship_message(long port_id, int ship_id)
{
  int queue_id;
  struct message msg;

  if ((queue_id = msgget(DOCKED_MESSAGE, 0)) == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  msg.mtype = port_id + 1;
  msg.value = ship_id;

  if (msgsnd(queue_id, &msg, sizeof(struct message), 0) == -1) {
    perror("msgsnd");
    exit(EXIT_FAILURE);
  }
}

int receive_docked_ship_message(long port_id)
{
  int queue_id;
  struct message msg;

  if ((queue_id = msgget(DOCKED_MESSAGE, 0)) == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  if (msgrcv(queue_id, &msg, sizeof(struct message), port_id + 1, 0) == -1) {
    perror("msgrcv");
    exit(EXIT_FAILURE);
  }

  return msg.value;
}

int destroy_queues(void)
{
  int queue_id;

  if ((queue_id = msgget(DOCKED_MESSAGE, 0)) == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
    perror("msgctl");
    exit(EXIT_FAILURE);
  }

  if ((queue_id = msgget(LOADED_LOTS_MESSAGE, 0)) == -1) {
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
    perror("msgctl");
    exit(EXIT_FAILURE);
  }

  return 0;
}

/*PORT SUPPORT*/
int create_shared_port_array(int array_size) {
  key_t key = PORT_KEY;
  int shm_id = shmget(key, sizeof(Port) * array_size, 0666 | IPC_EXCL | IPC_CREAT);
  if (shm_id == -1) {
    perror("create_shared_port_array failed");
    exit(EXIT_FAILURE);
  }
  return shm_id;
}

Port *get_shared_port_array(void) {
  key_t key = PORT_KEY;
  Port *shm_addr;

  int shm_id = shmget(key, 0, 0666);
  if (shm_id == -1) {
    perror("get_shared_port_array failed");
    exit(EXIT_FAILURE);
  }

  shm_addr = shmat(shm_id, (void *) 0, 0);
  if (shm_addr == (void *) -1) {
    perror("shm_at failed");
    exit(EXIT_FAILURE);
  }

  return shm_addr;
}

void detach_shared_port_array(void) {
  void *shm_addr = get_shared_port_array();

  if (shmdt(shm_addr) == -1) {
    perror("detach_shared_port_array failed");
    exit(1);
  }
}

/*SHIP SUPPORT*/
int create_shared_ship_array(int array_size) {
  key_t key = SHIP_KEY;
  int shm_id = shmget(key, sizeof(Ship) * array_size, 0666 | IPC_EXCL | IPC_CREAT);
  if (shm_id == -1) {
    perror("create_shared_ship_array failed");
    exit(EXIT_FAILURE);
  }
  return shm_id;
}


Ship *get_shared_ship_array(void) {
  key_t key = SHIP_KEY;
  Ship *shm_addr;

  int shm_id = shmget(key, 0, 0666);
  if (shm_id == -1) {
    perror("get_shared_ship_array failed");
    exit(EXIT_FAILURE);
  }

  shm_addr = shmat(shm_id, (void *) 0, 0);
  if (shm_addr == (void *) -1) {
    perror("shm_at failed");
    exit(EXIT_FAILURE);
  }

  return shm_addr;
}

void detach_shared_ship_array(void) {
  void *shm_addr = get_shared_ship_array();

  if (shmdt(shm_addr) == -1) {
    perror("detach_shared_ship_array failed");
    exit(1);
  }
}

/*DOCKS SEMAPHORES*/
union sem_un {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
} arg;

int create_docks_semaphores(Port *ports, int SO_PORTI) {
  key_t key = DOCK_SEM_KEY;
  int i, sem_op;
  int sem_id = semget(key, SO_PORTI, 0666 | IPC_EXCL | IPC_CREAT);

  if (sem_id < 0) {
    perror("create_docks_semaphores failed creating");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < SO_PORTI; i++) {
    arg.val = ports[i].n_docks;
    sem_op = semctl(sem_id, i, SETVAL, arg);
    if (sem_op < 0) {
      perror("create_docks_semaphores failed assign value");
      exit(EXIT_FAILURE);
    }
  }

  return sem_id;
}

void dock_at_the_port(int port_id) {
  key_t key = DOCK_SEM_KEY;
  struct sembuf sop;

  int sem_id = semget(key, 1, 0);
  if (sem_id < 0) {
    /*perror("dock_at_the_port failed get");
    exit(EXIT_FAILURE);*/
  }

  sop.sem_num = port_id;
  sop.sem_op = -1;
  sop.sem_flg = 0;

  if (semop(sem_id, &sop, 1) == -1) {
    /*perror("dock_at_the_port failed decrementing");
    exit(EXIT_FAILURE);*/
  }
}

void undock_from_the_port(int port_id) {
  key_t key = DOCK_SEM_KEY;
  struct sembuf sop;

  int sem_id = semget(key, 1, 0);
  if (sem_id < 0) {
    perror("dock_at_the_port failed get");
    exit(EXIT_FAILURE);
  }

  sop.sem_num = port_id;
  sop.sem_op = +1;
  sop.sem_flg = 0;

  if (semop(sem_id, &sop, 1) == -1) {
    perror("dock_at_the_port failed incrementing");
    exit(EXIT_FAILURE);
  }
}

int get_available_docks(int port_id) {
  key_t key = DOCK_SEM_KEY;
  int result;

  int sem_id = semget(key, 1, 0);
  if (sem_id < 0) {
    perror("get_available_docks failed get");
    exit(EXIT_FAILURE);
  }

  arg.array = 0;
  result = (int) semctl(sem_id, port_id, GETVAL, arg);

  if (result == -1) {
    perror("get_available_docks failed get available");
    exit(EXIT_FAILURE);
  }

  return result;
}

void destroy_docks_semaphores(void) {
  key_t key = DOCK_SEM_KEY;
  int sem_id = semget(key, 1, 0);
  if (sem_id < 0) {
    perror("get_available_docks failed get");
    exit(EXIT_FAILURE);
  }


  if (semctl(sem_id, 0, IPC_RMID, NULL) == -1) {
    perror("destroy_docks_semaphores failed");
    exit(EXIT_FAILURE);
  }
}

/*TRANSFER SEMAPHORE*/
int ledger_mutex_create(int SO_PORTI) {
  key_t key = PORT_STOCK_SEM_KEY;
  int n_sems = SO_PORTI + 1;
  int i;
  int sem_id = semget(key, n_sems, IPC_CREAT | 0666);
  if (sem_id < 0) {
    perror("ledger_mutex_create create");
    exit(EXIT_FAILURE);
  }

  arg.val = 1;
  for (i = 0; i < n_sems; i++) {
    if (semctl(sem_id, i, SETVAL, arg) < 0) {
      perror("ledger_mutex_create get");
      exit(EXIT_FAILURE);
    }
  }

  return sem_id;
}

int lock_ledger(int port_id) {
  key_t key = PORT_STOCK_SEM_KEY;
  int sem_num = port_id;
  struct sembuf sops;

  int sem_id = semget(key, 1, 0);
  if (sem_id < 0) {
    /*perror("lock_ledger get");
    exit(EXIT_FAILURE);*/
  }

  sops.sem_num = sem_num;
  sops.sem_op = -1;
  sops.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sops, 1) < 0) {
    /*perror("lock_ledger acquire");
    exit(EXIT_FAILURE);*/
  }

  return 0;
}

int unlock_ledger(int port_id) {
  key_t key = PORT_STOCK_SEM_KEY;
  int sem_num = port_id;
  struct sembuf sops;

  int sem_id = semget(key, 1, 0);
  if (sem_id < 0) {
    perror("unlock_ledger get");
    exit(EXIT_FAILURE);
  }

  sops.sem_num = sem_num;
  sops.sem_op = 1;
  sops.sem_flg = SEM_UNDO;

  if (semop(sem_id, &sops, 1) < 0) {
    perror("unlock_ledger release");
    exit(EXIT_FAILURE);
  }

  return 0;
}

int get_value_mutex(int port_id) {
  int result;
  key_t key = PORT_STOCK_SEM_KEY;

  int sem_id = semget(key, 1, 0);
  if (sem_id < 0) {
    perror("get_value_mutex get");
    exit(EXIT_FAILURE);
  }

  arg.array = 0;
  result = (int) semctl(sem_id, port_id, GETVAL, arg);
  if (result == -1) {
    perror("get_value_mutex result");
    exit(EXIT_FAILURE);
  }

  return result;
}

void destroy_ledger(void) {
  key_t key = PORT_STOCK_SEM_KEY;
  int sem_id = semget(key, 0, 0);
  if (sem_id < 0) {
    perror("destroy_ledger get");
    exit(EXIT_FAILURE);
  }


  if (semctl(sem_id, 0, IPC_RMID, NULL) == -1) {
    perror("destroy_ledger destroy");
    exit(EXIT_FAILURE);
  }
}

void print_lot_array(Lot_array cargo) {
  int i;
  printf("\n-------------\n");
  for (i = 0; i < cargo.size; i++) {
    printf("Good:%d -> qt:%d, exp:%d\n", cargo.lots[i].good_id, cargo.lots[i].quantity, cargo.lots[i].expire_in);
  }
  fflush(NULL);
}


int add_lot_array_element(Lot_array *lots_array, Lot element) {
  if (lots_array->size >= MAX_LOT_ARRAY) {
    fprintf(stderr, "add_lot_array_element exceeding MAX_LOT_ARRAY\n");
    exit(EXIT_FAILURE);
  }

  lots_array->lots[lots_array->size] = element;
  lots_array->size++;

  return 0;
}

Lot extract_lot_array_element(Lot_array *lots_array, int index) {
  Lot element = lots_array->lots[index];
  int i;

  if (index < 0 || index >= lots_array->size) {
    fprintf(stderr, "extract_lot_array_element out of bound\n");
    exit(EXIT_FAILURE);
  }

  for (i = index; i < lots_array->size - 1; i++) {
    lots_array->lots[i] = lots_array->lots[i + 1];
  }

  lots_array->size--;

  return element;
}

Lot get_lot_array_element(Lot_array *lots_array, int index) {
  Lot element = lots_array->lots[index];

  if (index < 0 || index >= lots_array->size) {
    fprintf(stderr, "extract_lot_array_element out of bound\n");
    exit(EXIT_FAILURE);
  }

  return element;
}

int get_random_int(int low, int high, unsigned int *state) {

  int random = rand_r(state);

  return low + random % (abs(high - low) + 1);
}

void generate_ports(Port ports[], int SO_PORTI, int SO_BANCHINE, int SO_LATO, int SO_FILL, int SO_MERCI) {
  int sum, k, i, p;
  int random, max_step, min_step;
  unsigned int state = time(NULL) ^ getpid();

  for (i = 0; i < SO_PORTI; i++) {
    ports[i].port_id = i;
    ports[i].n_docks = get_random_int(1, SO_BANCHINE, &state);
    ports[i].received.size = 0;
    ports[i].demand.size = 0;
    ports[i].offer.size = 0;
    ports[i].initial_offer_tons = SO_FILL / SO_PORTI;
    ports[i].initial_demand_tons = SO_FILL / SO_PORTI;

    random = get_random_int(0, 3, &state);
    switch (random) {
      case 0: {
        ports[i].x = 0;
        ports[i].y = get_random_int(0, SO_LATO - 1, &state);
        break;
      }
      case 1: {
        ports[i].x = get_random_int(0, SO_LATO - 1, &state);
        ports[i].y = 0;
        break;
      }
      case 2: {
        ports[i].x = SO_LATO - 1;
        ports[i].y = get_random_int(0, SO_LATO - 1, &state);
        break;
      }
      case 3: {
        ports[i].x = get_random_int(0, SO_LATO - 1, &state);
        ports[i].y = SO_LATO - 1;
        break;
      }
      default:
        exit(EXIT_FAILURE);
    }
  }


  min_step = MAX_LOT_ARRAY / SO_PORTI;
  max_step = SO_FILL / SO_PORTI;

  sum = 0;
  while (sum < SO_FILL) {
    int random_port_id = get_random_int(0, SO_PORTI - 1, &state);
    ports[random_port_id].initial_offer_tons += get_random_int(min_step, max_step, &state);
    sum += ports[random_port_id].initial_offer_tons;
  }

  sum = 0;
  while (sum < SO_FILL) {
    int random_port_id = get_random_int(0, SO_PORTI - 1, &state);
    ports[random_port_id].initial_demand_tons += get_random_int(1, max_step, &state);
    sum += ports[random_port_id].initial_demand_tons;
  }

  ports[0].x = 0;
  ports[0].y = 0;

  ports[1].x = SO_LATO - 1;
  ports[1].y = 0;

  ports[2].x = SO_LATO - 1;
  ports[2].y = SO_LATO - 1;

  ports[3].x = 0;
  ports[3].y = SO_LATO - 1;

}

int get_tons(Lot_array lot_array) {
  int sum = 0, i;

  for (i = 0; i < lot_array.size; i++) {
    sum += lot_array.lots[i].quantity;
  }
  return sum;
}

int get_tons_good_id(Lot_array lot_array, int good_id) {
  int sum = 0, i;

  for (i = 0; i < lot_array.size; i++) {
    if (lot_array.lots[i].good_id == good_id)
      sum += lot_array.lots[i].quantity;
  }
  return sum;
}

int get_tons_good_id_expired(Lot_array lot_array, int good_id) {
  int sum = 0, i;

  for (i = 0; i < lot_array.size; i++) {
    if (lot_array.lots[i].good_id == good_id && lot_array.lots[i].expire_in <= 0)
      sum += lot_array.lots[i].quantity;
  }
  return sum;
}

void print_stats(int day, int SO_PORTI, int SO_NAVI) {
  Port *ports = get_shared_port_array();
  Ship *ships = get_shared_ship_array();
  int c, i, docked = 0, full_sea = 0, empty_sea = 0;

  for (c = 0; c < SO_NAVI; c++) {
    Ship this_ship = ships[c];
    if (this_ship.state == EMPTY_SEA)
      empty_sea++;
    if (this_ship.state == FULL_SEA)
      full_sea++;
    if (this_ship.state == DOCKED)
      docked++;
  }

  if (day == -1) {
    printf("REPORT FINALE\n\n");
  } else {
    printf("GIORNO: %d\n\n", day);
  }

  printf("Navi in mare con carico: %d\n", full_sea);
  printf("Navi in mare senza carico: %d\n", empty_sea);
  printf("Navi attraccate in porto: %d\n\n", docked);


  for (c = 0; c < SO_PORTI; c++) {
    Port port = ports[c];
    printf(
        "Porto %d -> banchine max: %d/%d | merce presente: %d lotti per %d ton | spedita: %d per %d ton | ricevuta: %d lotti per %d ton | domanda %d ton",
        c,
        get_available_docks(c), port.n_docks,
        port.offer.size, get_tons(port.offer),
        port.shipped.size, get_tons(port.shipped),
        port.received.size, get_tons(port.received),
        get_tons(port.demand)
    );

    printf("\n");
    fflush(NULL);
  }
}

void print_final_stats(int SO_PORTI, int SO_NAVI, int SO_MERCI) {
  Port *ports = get_shared_port_array();
  Ship *ships = get_shared_ship_array();
  int tons;
  int good_idx, port_idx, ship_idx;

  print_stats(-1, SO_PORTI, SO_NAVI);

  printf("\n");

  for (good_idx = 0; good_idx < SO_MERCI; good_idx++) {
    struct {
      int port_id;
      int tons;
    } max_offer, max_demand;

    int in_port = 0, in_port_expired = 0, in_ship_expired = 0, received = 0;
    max_offer.tons = 0;
    max_offer.port_id = -1;
    max_demand.tons = 0;
    max_demand.port_id = -1;

    for (port_idx = 0; port_idx < SO_PORTI; port_idx++) {
      Port port = ports[port_idx];
      in_port += get_tons_good_id(port.offer, good_idx);
      in_port_expired += get_tons_good_id_expired(port.offer, good_idx);
      received += get_tons_good_id(port.received, good_idx);

      tons = get_tons_good_id(port.shipped, good_idx);
      if (max_offer.tons < tons) {
        max_offer.port_id = port_idx;
        max_offer.tons = tons;
      }
      tons = get_tons_good_id(port.received, good_idx);
      if (max_demand.tons < tons) {
        max_demand.port_id = port_idx;
        max_demand.tons = tons;
      }

    }

    for (ship_idx = 0; ship_idx < SO_NAVI; ship_idx++) {
      Ship ship = ships[ship_idx];
      in_ship_expired += get_tons_good_id_expired(ship.cargo, good_idx);
    }

    printf(
        "Id merce: %d -> ferma in porto: %d, scaduta in porto %d, scaduta in nave %d, consegnata %d | Porto con più offerta %d, con più domanda %d\n",
        good_idx,
        in_port, in_port_expired, in_ship_expired, received, max_offer.port_id, max_demand.port_id);
  }

  fflush(NULL);
}

void print_map(int port_size, int map_size) {
  char *map = calloc(map_size * map_size, sizeof(char));
  Port *ports;
  int i;

  for (i = 0; i < map_size * map_size; i++) {
    map[i] = '~';
  }

  ports = get_shared_port_array();

  for (i = 0; i < port_size; i++) {
    map[ports[i].x + ports[i].y * map_size] = 'P';
  }

  printf("\n\n\t\t\t\tMAPPA\n");
  for (i = 0; i < map_size * map_size; i++) {
    if (i % map_size == 0) {
      printf("\n");
    }
    printf(" %c ", map[i]);
  }

  free(map);
}
