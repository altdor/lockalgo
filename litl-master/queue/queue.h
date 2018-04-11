#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct queueNode {
	struct queueNode *next;
	int value;
} QueueNode;

typedef struct queue {
	QueueNode *head;
	QueueNode *tail;
	pthread_mutex_t lock;
} Queue;

Queue* queue_create();
int queue_destroy(Queue *queue);
int enqueue(Queue *queue, int val);
int dequeue(Queue *queue, int *val);