#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define NUMBER_OF_THREADS 8
#define NUMBER_OF_EXECUTIONS 1000000
#define BUFFER_SIZE 1000

pthread_mutex_t lock;
volatile int counter;

void *do_thread_work(void *nullptr){

	int j, tmp;
	unsigned int id = pthread_self();
	char buf[BUFFER_SIZE]; 
	
	for (j = 0; j < NUMBER_OF_EXECUTIONS; j++){
		
		pthread_mutex_lock(&lock);
		
		counter = counter + 1;
		
		pthread_mutex_unlock(&lock);
	}
}

int main(int argc, char **argv){

	int i;
	counter = 0;

	pthread_t threadArray[NUMBER_OF_THREADS];

	if(pthread_mutex_init(&lock, NULL) != 0){
		fprintf(stderr, "Error creating lock\n");
		return 0;
	}

	for(i = 0; i < NUMBER_OF_THREADS; i++){
		if(pthread_create(&(threadArray[i]), NULL, do_thread_work, NULL)){
			fprintf(stderr, "Error creating thread\n");
			return 0;
		}
	}

	for(i = 0; i < NUMBER_OF_THREADS; i++){
		if(pthread_join(threadArray[i], NULL)){
			fprintf(stderr, "Error joining thread\n");
			return 0;
		}
	}
	
	if(counter == NUMBER_OF_THREADS*NUMBER_OF_EXECUTIONS){
		printf("execution finished successfully\n");
	}

	if(pthread_mutex_destroy(&lock)){
		fprintf(stderr, "Error destroying lock\n");
		return 0;
	}
	
	return 0;
}
