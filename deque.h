#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "option.h"

#define str(x) #x

#define __deq_t(t) t ## _deque

#define DEQUE_MINIMUM_CAPACITY 1

#define Deque(t) \
typedef struct { \
	size_t tail; \
	size_t head; \
	size_t capacity; \
	t* data; \
} __deq_t(t)

#define deque_new(t) \
__deq_t(t)* t ## _deque_new() { \
	__deq_t(t)* deq = calloc(1, sizeof(__deq_t(t))); \
	deq->capacity = DEQUE_MINIMUM_CAPACITY; \
	deq->data = calloc(deq->capacity, sizeof(t)); \
	return deq; \
}

#define deque_free(t) \
void t ## _deque_free(__deq_t(t)** deq) { \
	if ((*deq)->data) { \
		free((*deq)->data); \
	} \
	(*deq)->data = NULL; \
	(*deq)->capacity = 0; \
	(*deq)->tail = 0; \
	(*deq)->head = 0; \
	free(*deq); \
	*deq = NULL; \
}

#define deque_len(deq) ((deq->head - deq->tail) & (deq->capacity - 1))
#define deque_is_full(deq) ((deq->capacity - deque_len(deq)) == 1)
#define deque_is_empty(deq) (deq->head == deq->tail)

#define __deq_handle_capacity_increase(t, deq, old_cap) \
do { \
	if (deq->tail <= deq->head) { \
		continue; \
	} else if (deq->head < old_cap - deq->tail) { \
		memcpy(&deq->data[old_cap], &deq->data[0], deq->head * sizeof(t)); \
		deq->head += old_cap; \
	} else { \
		size_t new_tail = deq->capacity - (old_cap - deq->tail); \
		memcpy(&deq->data[new_tail], &deq->data[deq->tail], (old_cap - deq->tail) * sizeof(t)); \
		deq->tail = new_tail; \
	} \
} while (0);

#define __deq_grow(t, deq) \
do { \
	if (deque_is_full(deq)) { \
		size_t old_cap = deq->capacity; \
		deq->capacity <<= 1; \
		deq->data = (t*) reallocarray((void*) deq->data, deq->capacity, sizeof(t)); \
		__deq_handle_capacity_increase(t, deq, old_cap); \
	} \
} while (0);

//#https://doc.rust-lang.org/src/alloc/collections/vec_deque/mod.rs.html
#define deque_push_back(t) \
void t ## _deque_push_back(__deq_t(t)* deq, t item) { \
	if (deque_is_full(deq)) { \
		__deq_grow(t, deq); \
	} \
	size_t head = deq->head; \
	deq->head = (deq->head + 1) & (deq->capacity - 1); \
	deq->data[head] = item; \
}

#define deque_push_front(t) \
void t ## _deque_push_front(__deq_t(t)* deq, t item) { \
	if (deque_is_full(deq)) { \
		__deq_grow(t, deq); \
	} \
	deq->tail = (deq->tail - 1) & (deq->capacity - 1); \
	deq->data[deq->tail] = item; \
}

#define deque_pop_back(t) \
__opt_t(t) t ## _deque_pop_back(__deq_t(t)* deq) { \
	if (deque_is_empty(deq)) { \
		return None(t); \
	} \
	deq->head = (deq->head - 1) & (deq->capacity - 1); \
	return Some(t, deq->data[deq->head]); \
}

#define deque_pop_front(t) \
__opt_t(t) t ## _deque_pop_front(__deq_t(t)* deq) { \
	if (deque_is_empty(deq)) { \
		return None(t); \
	} \
	size_t tail = deq->tail; \
	deq->tail = (deq->tail + 1) & (deq->capacity - 1); \
	return Some(t, deq->data[tail]); \
}
