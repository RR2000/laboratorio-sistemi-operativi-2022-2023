#ifndef PROGETTO_SISTEMI_OPERATIVI_2022_2023_UTILS_H
#define PROGETTO_SISTEMI_OPERATIVI_2022_2023_UTILS_H

#define MAX_LOT_ARRAY              30000
#define DOCKED_MESSAGE        0xABCDFFDD
#define LOADED_LOTS_MESSAGE   0xABCDDDFF
#define PORT_KEY              0xABCDDADA
#define SHIP_KEY              0xABCDDEDE
#define DOCK_SEM_KEY          0xABCD5F04
#define PORT_STOCK_SEM_KEY    0xABCD55F9

typedef struct {
  int good_id;
  int quantity;
  int expire_in;
} Lot;

typedef struct {
  int size;
  Lot lots[MAX_LOT_ARRAY];
} Lot_array;

typedef struct {
  int port_id;
  int x;
  int y;
  int n_docks;
  int initial_offer_tons;
  int initial_demand_tons;
  Lot_array offer;
  Lot_array demand;
  Lot_array shipped;
  Lot_array received;
} Port;

typedef enum {
  DOCKED,
  FULL_SEA,
  EMPTY_SEA
} Ship_State;

typedef struct {
  int x;
  int y;
  Lot_array cargo;
  int loaded_tons;
  Ship_State state;
} Ship;


void create_queues(void);

void send_docked_ship_message(long port_id, int ship_id);

int receive_docked_ship_message(long port_id);

void send_tons_loaded_message(long port_id, int tons);

int receive_tons_loaded_message(long port_id);

int destroy_queues(void);


int create_shared_port_array(int size);

Port *get_shared_port_array(void);

void detach_shared_port_array(void);


int create_shared_ship_array(int size);

Ship *get_shared_ship_array(void);

void detach_shared_ship_array(void);


void print_lot_array(Lot_array cargo);

Lot get_lot_array_element(Lot_array *lots_array, int index);


Lot extract_lot_array_element(Lot_array *lots_array, int index);

int add_lot_array_element(Lot_array *lots_array, Lot element);

int create_docks_semaphores(Port *ports, int SO_PORTI);


void dock_at_the_port(int port_id);

void undock_from_the_port(int port_id);

int get_available_docks(int port_id);

void destroy_docks_semaphores(void);


int ledger_mutex_create(int SO_PORTI);

int lock_ledger(int port_id);

int unlock_ledger(int port_id);

int get_value_mutex(int port_id);

void destroy_ledger(void);


int get_random_int(int low, int high, unsigned int *state);

void generate_ports(Port ports[], int SO_PORTI, int SO_BANCHINE, int SO_LATO, int SO_FILL, int SO_MERCI);

int get_tons(Lot_array lots);

void print_stats(int day, int SO_PORTI, int SO_NAVI);

void print_final_stats(int SO_PORTI, int SO_NAVI, int SO_MERCI);

void print_map(int port_size, int map_size);

#endif /*PROGETTO_SISTEMI_OPERATIVI_2022_2023_UTILS_H*/
