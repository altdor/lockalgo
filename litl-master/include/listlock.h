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
 */
#ifndef __LISTLOCK_H__
#define __LISTLOCK_H__

#include "padding.h"
#include <stdbool.h>

#define LOCK_ALGORITHM "listlock"
#define NEED_CONTEXT 0
#define SUPPORT_WAITING 1
#define NO_INDIRECTION 1

typedef struct listlock_node{
	bool flag;
	unsigned int threadId;
	struct listlock_node* next;
} listlock;

typedef struct{
	listlock* current;
	listlock* head;
} listlock_mutex_t __attribute__((aligned(L_CACHE_LINE_SIZE)));

typedef pthread_cond_t listlock_cond_t;
typedef void *listlock_context_t;

listlock_mutex_t *listlock_mutex_create(const pthread_mutexattr_t *attr);
int listlock_mutex_lock(listlock_mutex_t *impl, listlock_context_t *me);
int listlock_mutex_trylock(listlock_mutex_t *impl, listlock_context_t *me);
void listlock_mutex_unlock(listlock_mutex_t *impl, listlock_context_t *me);
int listlock_mutex_destroy(listlock_mutex_t *lock);
int listlock_cond_init(listlock_cond_t *cond, const pthread_condattr_t *attr);
int listlock_cond_timedwait(listlock_cond_t *cond, listlock_mutex_t *lock,
                         listlock_context_t *me, const struct timespec *ts);
int listlock_cond_wait(listlock_cond_t *cond, listlock_mutex_t *lock,
                    listlock_context_t *me);
int listlock_cond_signal(listlock_cond_t *cond);
int listlock_cond_broadcast(listlock_cond_t *cond);
int listlock_cond_destroy(listlock_cond_t *cond);
void listlock_thread_start(void);
void listlock_thread_exit(void);
void listlock_application_init(void);
void listlock_application_exit(void);

typedef listlock_mutex_t lock_mutex_t;
typedef listlock_context_t lock_context_t;
typedef listlock_cond_t lock_cond_t;

#define lock_mutex_create listlock_mutex_create
#define lock_mutex_lock listlock_mutex_lock
#define lock_mutex_trylock listlock_mutex_trylock
#define lock_mutex_unlock listlock_mutex_unlock
#define lock_mutex_destroy listlock_mutex_destroy
#define lock_cond_init listlock_cond_init
#define lock_cond_timedwait listlock_cond_timedwait
#define lock_cond_wait listlock_cond_wait
#define lock_cond_signal listlock_cond_signal
#define lock_cond_broadcast listlock_cond_broadcast
#define lock_cond_destroy listlock_cond_destroy
#define lock_thread_start listlock_thread_start
#define lock_thread_exit listlock_thread_exit
#define lock_application_init listlock_application_init
#define lock_application_exit listlock_application_exit
#define lock_init_context listlock_init_context

#endif // __listlock_H__
