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
#include <arraylistlock.h>
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

void resizeArray(listlock_mutex_t *impl){
	
	listlock_node_t** volatile newArrayList;
	int newSize, i;
	
	for(newSize = 1; cur_thread_id > newSize - 1; newSize*=2);

	newArrayList = (listlock_node_t**)malloc(newSize * sizeof(listlock_node_t*));
	
	for(i = 0; i <= impl->currentListSize; i++)
		newArrayList[i] = impl->arrayList[i];
	for(i = impl->currentListSize; i < newSize; i++)
		newArrayList[i] = NULL;
	
	impl->arrayList = newArrayList;
	impl->currentListSize = newSize;
	impl->resizeInProgress = false;
}

void verifyNodeExist(listlock_mutex_t *impl){
	
	int activeThreadCount;
	
	while(cur_thread_id > impl->currentListSize - 1){
		if(__sync_bool_compare_and_swap(&impl->resizeInProgress, false, true))
			resizeArray(impl);
		else
			while(impl->resizeInProgress);
	}
	
	while(true){
		activeThreadCount = impl->activeThreadCount;
		if(activeThreadCount - 1 < (int)cur_thread_id)
			__sync_bool_compare_and_swap(&impl->activeThreadCount, activeThreadCount, (int)cur_thread_id + 1);
		else
			break;
	}
	
	if(impl->arrayList[cur_thread_id] == NULL){
		impl->arrayList[cur_thread_id] = (listlock_node_t*)malloc(sizeof(listlock_node_t));
		impl->arrayList[cur_thread_id]->flag = false;
	}
}

int trylock(listlock_mutex_t *lock){

	if(lock->owner == cur_thread_id || (lock->owner == NO_OWNER && __sync_bool_compare_and_swap(&lock->owner, NO_OWNER, cur_thread_id)))
		return SUCCESS;
	else
		return FAILURE;
}

int listlock_mutex_lock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {

	verifyNodeExist(impl);
	
	impl->arrayList[cur_thread_id]->flag = true;
	//MEMORY_BARRIER();
	
	while(true){
		if(trylock(impl) == SUCCESS){
			return SUCCESS;
		}
	}
}

int listlock_mutex_trylock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {

	verifyNodeExist(impl);
	//MEMORY_BARRIER();

	return trylock(impl);
}
void listlock_mutex_unlock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {

	int i, curr;
	int arrayLength = impl->activeThreadCount;
	
	//if this thread doesn't own the lock return
	if(impl->owner != cur_thread_id)
		return;

	impl->arrayList[impl->owner]->flag = false;
	//MEMORY_BARRIER();
	
	for(i = 1; i < arrayLength; i++){
		curr = (i + cur_thread_id) % arrayLength;
		if(impl->arrayList[curr] != NULL && impl->arrayList[curr]->flag == true){
			impl->owner = curr;
			//MEMORY_BARRIER();
			return;
		}
	}
	impl->owner = NO_OWNER;
	//MEMORY_BARRIER();

	return;
}

int listlock_mutex_destroy(listlock_mutex_t *lock) {

	int i;

	for(i = 0; i < lock->activeThreadCount - 1;i++){
		free(lock->arrayList[i]);
	}
	free(lock->arrayList);
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

	int i;

	listlock_mutex_t *mutex = (listlock_mutex_t*)alloc_cache_align(sizeof(listlock_mutex_t));
	if (mutex == NULL){
		return NULL;
	}

	mutex->owner = NO_OWNER;
	mutex->currentListSize = INITIAL_ARRAY_LENGTH;
	mutex->activeThreadCount = 0;
	mutex->resizeInProgress = false;
	mutex->arrayList = (listlock_node_t**)alloc_cache_align(INITIAL_ARRAY_LENGTH * sizeof(listlock_node_t*));
	if (mutex->arrayList == NULL){
		free(mutex);
		return NULL;
	}
	
	for(i = 0; i < INITIAL_ARRAY_LENGTH; i++)
		mutex->arrayList[i] = NULL;

	//MEMORY_BARRIER();
    return mutex;
}

// Define library function ptr
// lock_mutex_unlock_fct     lock_mutex_unlock     = listlock_mutex_unlock;
