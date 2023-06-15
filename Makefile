CC = gcc
CFLAGS = -std=c89 -Wpedantic -D_GNU_SOURCE

all: utils master ship_process port_process
	$(CC) $(CFLAGS) -o bin/ship_process bin/ship_process.o bin/utils.o -lm
	$(CC) $(CFLAGS) -o bin/port_process bin/port_process.o bin/utils.o
	$(CC) $(CFLAGS) -o simulazione_navi bin/master.o bin/utils.o
	chmod 777 bin/ship_process
	chmod 777 bin/port_process
	chmod 777 simulazione_navi

master:
	$(CC) $(CFLAGS) -c src/master.c -o bin/master.o

ship_process:
	$(CC) $(CFLAGS) -c src/ship_process.c -o bin/ship_process.o

port_process:
	$(CC) $(CFLAGS) -c src/port_process.c -o bin/port_process.o

utils:
	$(CC) $(CFLAGS) -c src/utils.c -o bin/utils.o

clean:
	rm -r -f bin/*

run: clean all
	clear
	./simulazione_navi file.txt
