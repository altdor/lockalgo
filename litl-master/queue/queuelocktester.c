#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "queue.h"

#define SUCCESS 0
#define FAILURE -1

volatile int numberOfOperations; 
void *do_thread_work(void *queueptr){

	int i, ran, tmp, j = 0;
	time_t t;
	unsigned int id = pthread_self();
	Queue *queue = (Queue*)queueptr;
	
	srand((unsigned) time(&t));
	
	for (i = 0; i < numberOfOperations; i++){
		ran = rand();
		if((ran % 2) == 0){
			enqueue(queue, j);
			j++;
		} else{
			dequeue(queue, &tmp);
		}
	}
}

int main(int argc, char **argv){

	int i, numberOfThreads;
	clock_t begin, end;
	Queue *queue;
	
	if(argc != 3)
		printf("usage ...........");
	
	numberOfThreads = atoi(argv[1]);
	numberOfOperations = atoi(argv[2]);
	
	pthread_t threadArray[numberOfThreads];

	queue = queue_create(); 
	if(queue == NULL){
		fprintf(stderr, "Error creating lock\n");
		return FAILURE;
	}
	
	begin = clock();

	for(i = 0; i < numberOfThreads; i++){
		if(pthread_create(&(threadArray[i]), NULL, do_thread_work, queue)){
			fprintf(stderr, "Error creating thread\n");
			return FAILURE;
		}
	}

	for(i = 0; i < numberOfThreads; i++){
		if(pthread_join(threadArray[i], NULL)){
			fprintf(stderr, "Error joining thread\n");
			return FAILURE;
		}
	}

	end = clock();
	
	printf("throughput: %ld operations/sec\n", (CLOCKS_PER_SEC * numberOfThreads * numberOfOperations)/(end - begin));

	if(queue_destroy(queue) == FAILURE){
		fprintf(stderr, "Error destroying lock\n");
		return FAILURE;
	}

	return SUCCESS;
}
