#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "utils.h"

pid_t *ports_pids, *ships_pids;

void end_simulation(int SO_PORTI, int SO_NAVI) {
  int c;

  for (c = 0; c < SO_PORTI; c++) {
    kill(ports_pids[c], SIGINT);
  }
  for (c = 0; c < SO_NAVI; c++) {
    kill(ships_pids[c], SIGINT);
  }

  free(ports_pids);
  free(ships_pids);
  exit(EXIT_SUCCESS);
}

void update_expiration(Lot_array *lots) {
  int i;
  for (i = 0; i < lots->size; i++) {
    lots->lots[i].expire_in--;
  }
}

void
exec_stat_process(int SO_DAYS, int SO_PORTI, int SO_NAVI, int SO_MERCI, int total_processes, int barrier_semaphore_id) {
  pid_t pid;
  int i, j;

  pid = fork();
  if (pid == 0) {
    unlock_ledger(barrier_semaphore_id);
    while (get_value_mutex(barrier_semaphore_id) < total_processes);

    for (i = 0; i < SO_DAYS; ++i) {
      int kill_me_demand = 1, kill_me_offer = 1;
      system("clear");
      print_stats(i, SO_PORTI, SO_NAVI);
      for (j = 0; j < SO_PORTI; j++) {
        update_expiration(&get_shared_port_array()[j].offer);
        if (get_tons(get_shared_port_array()[j].demand) > 0) {
          kill_me_demand = 0;
        }
        if (get_tons(get_shared_port_array()[j].offer) > 0) {
          kill_me_offer = 0;
        }
      }
      for (j = 0; j < SO_NAVI; j++) {
        update_expiration(&get_shared_ship_array()[j].cargo);
      }
      sleep(1);
      if (kill_me_demand == 1 || kill_me_offer == 1)
        break;
    }

    system("clear");
    print_final_stats(SO_PORTI, SO_NAVI, SO_MERCI);

    end_simulation(SO_PORTI, SO_NAVI);
    exit(EXIT_FAILURE);
  } else if (pid > 0) {

  } else {
    /* fork failed */
    perror("fork failed");
    exit(EXIT_FAILURE);
  }
}

void exec_port_processes(int SO_PORTI, int int_SO_MERCI, int int_SO_CAPACITY, int int_SO_MIN_VITA, int int_SO_MAX_VITA,
                         int int_total_processes, int int_barrier_semaphore_id) {
  int c;
  char port_id[12], SO_MERCI[12], SO_CAPACITY[12], SO_MIN_VITA[12], SO_MAX_VITA[12], total_processes[12], barrier_semaphore_id[12];
  ports_pids = malloc(sizeof(pid_t) * SO_PORTI);
  for (c = 0; c < SO_PORTI; c++) {
    ports_pids[c] = fork();
    if (ports_pids[c] == 0) {
      sprintf(port_id, "%d", c);
      sprintf(SO_MERCI, "%d", int_SO_MERCI);
      sprintf(SO_CAPACITY, "%d", int_SO_CAPACITY);
      sprintf(SO_MIN_VITA, "%d", int_SO_MIN_VITA);
      sprintf(SO_MAX_VITA, "%d", int_SO_MAX_VITA);
      sprintf(total_processes, "%d", int_total_processes);
      sprintf(barrier_semaphore_id, "%d", int_barrier_semaphore_id);
      execl("bin/port_process", port_id, SO_MERCI, SO_CAPACITY, SO_MIN_VITA, SO_MAX_VITA, total_processes,
            barrier_semaphore_id, NULL);

      perror("execv failed");
      exit(EXIT_FAILURE);
    } else if (ports_pids[c] < 0) {
      /* fork failed */
      perror("fork failed");
      exit(EXIT_FAILURE);
    }
  }
}

void exec_ship_processes(int SO_NAVI, int int_SO_PORTI, int int_SO_SPEED, int int_SO_LATO,
                         int int_SO_LOADSPEED, int int_total_processes, int int_barrier_semaphore_id) {
  int c;
  char port_id[12], SO_PORTI[12], SO_SPEED[12], SO_LATO[12], SO_LOADSPEED[12], total_processes[12], barrier_semaphore_id[12];
  ships_pids = malloc(sizeof(pid_t) * SO_NAVI);

  sprintf(SO_PORTI, "%d", int_SO_PORTI);
  sprintf(SO_SPEED, "%d", int_SO_SPEED);
  sprintf(SO_LATO, "%d", int_SO_LATO);
  sprintf(SO_LOADSPEED, "%d", int_SO_LOADSPEED);
  sprintf(total_processes, "%d", int_total_processes);
  sprintf(barrier_semaphore_id, "%d", int_barrier_semaphore_id);
  for (c = 0; c < SO_NAVI; c++) {
    sprintf(port_id, "%d", c);

    ships_pids[c] = fork();
    if (ships_pids[c] == 0) {
      execl("bin/ship_process", port_id, SO_PORTI, SO_SPEED, SO_LATO, SO_LOADSPEED, total_processes,
            barrier_semaphore_id, NULL);
    } else if (ships_pids[c] < 0) {
      /* fork failed */
      perror("fork failed");
      exit(EXIT_FAILURE);
    }
  }
}

int SO_NAVI = -1;
int SO_PORTI = -1;
int SO_MERCI = -1;
int SO_SIZE = -1;
int SO_MIN_VITA = -1;
int SO_MAX_VITA = -1;
int SO_LATO = -1;
int SO_SPEED = -1;
int SO_CAPACITY = -1;
int SO_BANCHINE = -1;
int SO_FILL = -1;
int SO_LOADSPEED = -1;
int SO_DAYS = -1;

void read_variables_from_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  char line[256];

  if (file == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  while (fgets(line, sizeof(line), file)) {
    char *key = strtok(line, " =");
    char *value = strtok(NULL, " =");
    long int int_value = strtol(value, NULL, 10);

    if(int_value < 0){
      fprintf(stderr, "Valori negativi non ammessi!\n");
      exit(EXIT_FAILURE);
    }

    if (strcmp(key, "SO_NAVI") == 0) {
      SO_NAVI = (int)int_value;
      if(SO_NAVI < 1){
        fprintf(stderr, "SO_NAVI deve essere >= 1 !\n");
        exit(EXIT_FAILURE);
      }
    } else if (strcmp(key, "SO_PORTI") == 0) {
      SO_PORTI = (int)int_value;
      if(SO_PORTI < 4){
        fprintf(stderr, "SO_PORTI deve essere >= 4 !\n");
        exit(EXIT_FAILURE);
      }
    } else if (strcmp(key, "SO_MERCI") == 0) {
      SO_MERCI = (int)int_value;
    } else if (strcmp(key, "SO_SIZE") == 0) {
      SO_SIZE = (int)int_value;
    } else if (strcmp(key, "SO_MIN_VITA") == 0) {
      SO_MIN_VITA = (int)int_value;
    } else if (strcmp(key, "SO_MAX_VITA") == 0) {
      SO_MAX_VITA = (int)int_value;
    } else if (strcmp(key, "SO_LATO") == 0) {
      SO_LATO = (int)int_value;
    } else if (strcmp(key, "SO_SPEED") == 0) {
      SO_SPEED = (int)int_value;
    } else if (strcmp(key, "SO_CAPACITY") == 0) {
      SO_CAPACITY = (int)int_value;
    } else if (strcmp(key, "SO_BANCHINE") == 0) {
      SO_BANCHINE = (int)int_value;
    } else if (strcmp(key, "SO_FILL") == 0) {
      SO_FILL = (int)int_value;
    } else if (strcmp(key, "SO_LOADSPEED") == 0) {
      SO_LOADSPEED = (int)int_value;
    } else if (strcmp(key, "SO_DAYS") == 0) {
      SO_DAYS = (int)int_value;
    }
  }

  fclose(file);
}


int main(int argc, char *argv[]) {

  int i;
  int total_processes;
  int barrier_semaphore_id;
  Port *ports;
  Ship *ships;
  Lot *lots;

  system("ipcrm -a");
  system("clear");

  if(argc == 1){
    fprintf(stderr, "Devi definire un file di testo contentente le variabili!\n");
    exit(EXIT_FAILURE);
  }
  printf("Leggendo i parametri dal file...\n");
  read_variables_from_file(argv[1]);

  /** Processi porti +
   *  Processi navi +
   *  Processo statistiche +
   *  Processo main +
   *  1 perche' parte da 1 e non da 0
   *  */
  total_processes = SO_PORTI + SO_NAVI + 1 + 1 + 1;
  barrier_semaphore_id = SO_PORTI;

  printf("Creazione array porti...\n");
  create_shared_port_array(SO_PORTI);
  printf("Creazione array navi...\n");
  create_shared_ship_array(SO_NAVI);

  ports = get_shared_port_array();

  lots = malloc(sizeof(Lot) * SO_SIZE);

  printf("Generazione porti...\n");
  generate_ports(ports, SO_PORTI, SO_BANCHINE, SO_LATO, SO_FILL, SO_MERCI);
  printf("Generazione semafori porti...\n");
  create_docks_semaphores(ports, SO_PORTI);
  printf("Generazione semafori libri contabili porti...\n");
  ledger_mutex_create(SO_PORTI);
  printf("Generazione code messaggi...\n");
  create_queues();
  printf("Inizializzazione processi porti...\n");
  exec_port_processes(SO_PORTI, SO_MERCI, SO_CAPACITY, SO_MIN_VITA, SO_MAX_VITA, total_processes, barrier_semaphore_id);
  printf("Inizializzazione processi navi...\n");
  exec_ship_processes(SO_NAVI, SO_PORTI, SO_SPEED, SO_LATO, SO_LOADSPEED, total_processes, barrier_semaphore_id);
  printf("Inizializzazione processo statistiche...\n");
  exec_stat_process(SO_DAYS, SO_PORTI, SO_NAVI, SO_MERCI, total_processes, barrier_semaphore_id);

  printf("Quasi fatto! Attendo che tutti i processi siano pronti...\n");
  unlock_ledger(barrier_semaphore_id);
  while (get_value_mutex(barrier_semaphore_id) < total_processes);

  while (wait(NULL) > 0);

  detach_shared_port_array();
  detach_shared_ship_array();
  destroy_docks_semaphores();
  destroy_ledger();
  destroy_queues();
  free(lots);

  printf("SIMULAZIONE TERMINATA\n");

  system("ipcrm -a");

  exit(EXIT_SUCCESS);
}
