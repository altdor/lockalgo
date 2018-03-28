/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Hugo Guiroux <hugo.guiroux at gmail dot com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of his software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 * Just a proxy to pthread_mutex, to evaluate overhead of library
 * interposition.
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <assert.h>
#include <listlock.h>
#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <fcntl.h>
#include "interpose.h"
#include "utils.h"
#define SUCCESS 0
#define FAILURE -1

extern __thread unsigned int cur_thread_id;

listlock_node_t* findNodeWithThreadId(listlock_mutex_t *impl){

	listlock_node_t* curr = impl->head->next;
	listlock_node_t* obj;
	
	//search the list for a node referencing to this thread id
	while(curr != impl->head){	
		if (curr->threadId == cur_thread_id){
			printf("debug: thread found");
			fflush(NULL);
			return curr;
		}
		curr = curr->next;
	}

	/* if this thread is trying to lock the lock for the first time
	 * create a new node referencing to it and add it to the list
	 */
	obj = (listlock_node_t*)malloc(sizeof(listlock_node_t));
	if (obj == NULL){
		return NULL;
	}

	obj->flag = false;
	obj->threadId = cur_thread_id;
	MEMORY_BARRIER();
	do {
		obj->next = curr->next;
	} while(!__sync_bool_compare_and_swap(&curr->next, obj->next, obj));
	
	return obj;
}

int trylock(listlock_node_t *curr, listlock_mutex_t *lock){

	if(lock->owner == NULL){
		printf("lock owner is null\n");
		fflush(NULL);
	}
	else{
		if(lock->owner == curr){
			printf("owner == curr");
		}
		fflush(NULL);
	}
	if(lock->owner == curr || (lock->owner == NULL && __sync_bool_compare_and_swap(&lock->owner, NULL, curr)))
		return SUCCESS;
	else
		return FAILURE;
}

int listlock_mutex_lock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {

	listlock_node_t* curr = findNodeWithThreadId(impl);
	if(curr == NULL)
		return FAILURE;
	
	curr->flag = true;
	MEMORY_BARRIER();
	while(true){
		//printf("debug: trying to lock \n");
		//fflush(NULL);
		if(trylock(curr, impl) == SUCCESS){
			printf("debug: locked\n");
			fflush(NULL);
			return SUCCESS;
		}
	}
}

int listlock_mutex_trylock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {

	listlock_node_t* curr = findNodeWithThreadId(impl);
	return trylock(curr, impl);
}
void listlock_mutex_unlock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {

	//if this thread doesn't own the lock return
	if(impl->owner->threadId != cur_thread_id)
		return;

	impl->owner->flag = false;
	MEMORY_BARRIER();
	listlock_node_t* curr = impl->owner->next;
	while(curr != impl->owner){
		if(curr->flag == true){
			impl->owner = curr;
			MEMORY_BARRIER();
			printf("debug: unlock and pass to someone in queue (%d)\n", (int)curr->threadId);
			fflush(NULL);
			return;
		}
		curr = curr->next;
	}
	impl->owner = NULL;
	MEMORY_BARRIER();
	printf("debug: unlock turned to null\n");
			fflush(NULL);
	return;
}

int listlock_mutex_destroy(listlock_mutex_t *lock) {

	listlock_node_t* curr = lock->head;
	listlock_node_t* next;
	do{
		next = curr->next;
		free(curr);
		curr = next;
	} while(lock->head != curr);
	free(lock);
	return 0;
}

int listlock_cond_init(listlock_cond_t *cond, const pthread_condattr_t *attr) {
    return -1;
}

int listlock_cond_timedwait(listlock_cond_t *cond, listlock_mutex_t *lock,
                            listlock_context_t *me, const struct timespec *ts) {
	return -1;
}

int listlock_cond_wait(listlock_cond_t *cond, listlock_mutex_t *lock,
                       listlock_context_t *me){
	return -1;
}

int listlock_cond_signal(listlock_cond_t *cond) {
    return -1;
}

int listlock_cond_broadcast(listlock_cond_t *cond) {
    return -1;
}

int listlock_cond_destroy(listlock_cond_t *cond) {
    return -1;
}

void listlock_thread_start(void) {
}

void listlock_thread_exit(void) {
}

void listlock_application_init(void) {
}

void listlock_application_exit(void) {
}

void listlock_init_context(lock_mutex_t *UNUSED(impl),
                        lock_context_t *UNUSED(context), int UNUSED(number)) {
}

listlock_mutex_t *listlock_mutex_create(const pthread_mutexattr_t *attr) {

	listlock_mutex_t * mutex = (listlock_mutex_t*)alloc_cache_align(sizeof(listlock_mutex_t));
	if (mutex == NULL){
		return NULL;
	}
	mutex->owner = NULL;
	mutex->head = (listlock_node_t*)malloc(sizeof(listlock_node_t));
	if (mutex->head == NULL){
		free(mutex);
		return NULL;
	}
	mutex->head->next = mutex->head;
	mutex->head->threadId = -1;
	mutex->head->flag = false;
	MEMORY_BARRIER();
    return mutex;
}

// Define library function ptr
// lock_mutex_unlock_fct     lock_mutex_unlock     = listlock_mutex_unlock;
