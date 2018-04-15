#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define NUMBER_OF_THREADS 8
#define NUMBER_OF_EXECUTIONS 1000000
#define BUFFER_SIZE 1000

int pipefd[2];
pthread_mutex_t lock;

void *do_thread_work(void *nullptr){

	int j, tmp;
	unsigned int id = pthread_self();
	char buf[BUFFER_SIZE]; 
	
	for (j = 0; j < NUMBER_OF_EXECUTIONS; j++){
		
		pthread_mutex_lock(&lock);

		snprintf(buf, BUFFER_SIZE, "%dx%dx", id, 1);
		if((tmp = write(pipefd[1], buf, BUFFER_SIZE)) < 0){
			printf("fprintf failed with error : %d\n", tmp);
		}
		
		usleep(10);
		
		snprintf(buf, BUFFER_SIZE, "%dx%dx", id, 2);
		if((tmp = write(pipefd[1], buf, BUFFER_SIZE)) < 0){
			printf("fprintf failed with error : %d\n", tmp);
		}
		
		pthread_mutex_unlock(&lock);
	}
}

void *verifyOutput(void *nullptr){
	char str[BUFFER_SIZE];
	char str2[BUFFER_SIZE];
	char *str3;
	int a,b,c,d;
	int i;

	for(i = 0; i < NUMBER_OF_THREADS * NUMBER_OF_EXECUTIONS; i++){
		
		if(read(pipefd[0], str, BUFFER_SIZE) < 0){
			fprintf(stderr, "there are too few lines (%d) in output file\n", i);
			return;
		}

		if(strcpy(str2, str) == NULL){
			printf("bingo\n");
			return;
		}
		
		if((str3 = strtok(str, "x")) == NULL){
			printf("line = %s\n", str2);
			return;
		}
		a = atoi(str3);
		
		if((str3 = strtok(NULL, "x")) == NULL){
			printf("line = %s\n", str2);
			return;
		}
		b = atoi(str3);
		
		if(strtok(NULL, "x") != '\0'){
			fprintf(stderr, "line structure is wrong\n");
			return;
		}
		
		if(read(pipefd[0], str, BUFFER_SIZE) < 0){
			fprintf(stderr, "there are too few lines (%d) in output file\n", i);
			return;
		}

		if(strcpy(str2, str) == NULL){
			printf("bingo\n");
			return;
		}
		
		if((str3 = strtok(str, "x")) == NULL){
			printf("line = %s\n", str2);
			return;
		}
		c = atoi(str3);
		
		if((str3 = strtok(NULL, "x")) == NULL){
			printf("line = %s\n", str2);
			return;
		}
		d = atoi(str3);
		
		if(strtok(NULL, "x") != '\0'){
			fprintf(stderr, "line structure is wrong\n");
			return;
		}
		
		if((a != c) || (b != 1) || (d != 2)){
			fprintf(stderr, "line value is wrong\n");
			return;
		}
	}
	
	printf("execution finished successfully\n");

	return;
}

int main(){

	int i;
	
	if(pipe(pipefd) != 0){
		fprintf(stderr, "Error creating pipe\n");
		return 0;
	}

	pthread_t verificationThread;
	pthread_t threadArray[NUMBER_OF_THREADS];

	if(pthread_mutex_init(&lock, NULL) != 0){
		fprintf(stderr, "Error creating lock\n");
		return 0;
	}

	if(pthread_create(&verificationThread, NULL, verifyOutput, NULL)){
		fprintf(stderr, "Error creating verification thread\n");
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
	
	if(pthread_join(verificationThread, NULL)){
		fprintf(stderr, "Error joining verification thread\n");
		return 0;
	}

	if(pthread_mutex_destroy(&lock)){
		fprintf(stderr, "Error destroying lock\n");
		return 0;
	}
	
	close(pipefd[0]);
	close(pipefd[1]);
	
	return 0;
}
