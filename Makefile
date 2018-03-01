default: vm

CC=g++
CFLAGS= -O3 -fpermissive

garbage_collector.o: garbage_collector.cpp garbage_collector.hpp vm.h
	$(CC) $(CFLAGS) -c garbage_collector.cpp

vm: vm.c vm.h garbage_collector.o
	$(CC) $(CFLAGS) -o vm vm.c garbage_collector.o

clean: vm 
	rm *.o
	rm vm