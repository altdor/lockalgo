#include "queue.h"

#define SUCCESS 0
#define FAILURE -1
#define QUEUE_IS_EMPTY -2

Queue* queue_create() {
	
	Queue *queue = (Queue*)malloc(sizeof(Queue));

	if(queue == NULL)
		return NULL;
	
	queue->tail = (QueueNode*)malloc(sizeof(QueueNode));
	if(queue->tail == NULL){
		free(queue);
		return NULL;
	}

	queue->tail->next = NULL;
	queue->head = queue->tail;
	
	if(pthread_mutex_init(&queue->lock, NULL)){
		fprintf(stderr, "Error creating lock\n");
		free(queue->tail);
		free(queue);
		return NULL;
	}

	return queue;
}

int queue_destroy(Queue *queue) {

	int ret = SUCCESS;
	QueueNode *curr = queue->tail;
	QueueNode *prev;

	while(curr != NULL){
		prev = curr;
		curr = curr->next;
		free(prev);
	}

	if(pthread_mutex_destroy(&queue->lock)){
		fprintf(stderr, "Error destroying lock\n");
		ret = FAILURE;
	}

	free(queue);

	return ret;
}

int enqueue(Queue *queue, int val) {

	QueueNode *node;
	if(pthread_mutex_lock(&queue->lock)){
		return FAILURE;
	}

	node = (QueueNode*)malloc(sizeof(QueueNode));
	if(node == NULL)
		return FAILURE;
	
	node->next = NULL;
	node->value = val;
	queue->head->next = node;
	queue->head = node;

	if(pthread_mutex_unlock(&queue->lock) != SUCCESS)
		return FAILURE;
	
	return SUCCESS;
}

int dequeue(Queue *queue, int *val) {

	QueueNode *tmp;

	if(pthread_mutex_lock(&queue->lock))
		return FAILURE;

	if(queue->tail->next == NULL) {
		if(pthread_mutex_unlock(&queue->lock) != SUCCESS)
			return FAILURE;
		return QUEUE_IS_EMPTY;
	}

	tmp = queue->tail;
	queue->tail = queue->tail->next;

	free(tmp);
	*val = queue->tail->value;

	if(pthread_mutex_unlock(&queue->lock) != SUCCESS)
		return FAILURE;

	return SUCCESS;
}
