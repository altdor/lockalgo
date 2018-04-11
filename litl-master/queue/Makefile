all: queuelocktester

queue.o: queue.c queue.h
	gcc -pthread -c queue.c -o queue.o

queuelocktester.o: queuelocktester.c queue.h
	gcc -pthread -c queuelocktester.c -o queuelocktester.o
	
queuelocktester: queuelocktester.o queue.o
	gcc -pthread queuelocktester.o queue.o -o queuelocktester