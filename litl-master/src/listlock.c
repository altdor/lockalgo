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
// Posix functions
listlock* findNodeWithThreadId(listlock_mutex_t *impl){
	listlock* curr = impl->head->next;
	listlock* obj;
	while(curr != impl->head){
		if (curr->threadId == cur_thread_id){
			return curr;
		}
		curr = curr->next;
	}

	obj = (listlock*)malloc(sizeof(listlock));
	if (obj == NULL){
		return NULL;
	}
	obj->flag = false;
	obj->threadId = cur_thread_id;
	obj->next = curr->next;
	while(__sync_val_compare_and_swap(&curr->next, obj, obj->next) != obj){
		obj->next = curr->next;
	}
	return obj;
}

int trylock(listlock* curr, listlock* lock){
	if(lock == curr || (lock == NULL && __sync_val_compare_and_swap(&lock, curr, NULL) == curr)){
		return SUCCESS;
	}
	else{
		return FAILURE;
	}
}

int listlock_mutex_lock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {
	listlock* curr = findNodeWithThreadId(impl);
	if(curr == NULL)
		return FAILURE;
	curr->flag = true;
	while(true){
		listlock* lock = impl->current;
		if(trylock(curr, lock) == SUCCESS)
			return SUCCESS;
	}
}

int listlock_mutex_trylock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {
    listlock* curr = findNodeWithThreadId(impl);
	listlock* lock = impl->current;
	return trylock(curr, lock);
}
void listlock_mutex_unlock(listlock_mutex_t *impl, listlock_context_t *UNUSED(me)) {
	if(impl->current->threadId != cur_thread_id)
		return;
	impl->current->flag = false;
	listlock* curr = impl->current->next;
	while(curr != impl->current){
		if(curr->flag == true){
			impl->current = curr;
			return;
		}
	}
	impl->current = NULL;
	return;
}

int listlock_mutex_destroy(listlock_mutex_t *lock) {
    listlock* curr = lock->head;
	listlock* next;
	do{
		next = curr->next;
		free(curr);
		curr = next;
	} while(lock->head != curr);
	free(lock);
	return 0;
}

int listlock_cond_init(listlock_cond_t *cond, const pthread_condattr_t *attr) {
    return REAL(pthread_cond_init)(cond, attr);
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
    return REAL(pthread_cond_signal)(cond);
}

int listlock_cond_broadcast(listlock_cond_t *cond) {
    return REAL(pthread_cond_broadcast)(cond);
}

int listlock_cond_destroy(listlock_cond_t *cond) {
    return REAL(pthread_cond_destroy)(cond);
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
	mutex->current = NULL;
	mutex->head = (listlock*)malloc(sizeof(listlock));
	if (mutex->head == NULL){
		free(mutex);
		return NULL;
	}
	mutex->head->next = mutex->head;
    return mutex;
}

// Define library function ptr
// lock_mutex_unlock_fct     lock_mutex_unlock     = listlock_mutex_unlock;
