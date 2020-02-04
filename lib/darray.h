#ifndef DARRAY_H_
#define DARRAY_H_

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#define darray_set_capacity(arr, size)			\
do {							\
	if (arr) {					\
		((size_t *)(arr))[-1] = (size);		\
	}						\
} while(0)						

#define darray_set_size(arr, size)			\
do {							\
	if (arr) {					\
		((size_t *)(arr))[-2] = (size);		\
	}						\
} while(0)

#define darray_capacity(arr)				\
	((arr) ? ((size_t *)(arr))[-1] : (size_t)0)

#define darray_size(arr)				\
	((arr) ? ((size_t *)(arr))[-2] : (size_t)0)

#define darray_empty(arr)				\
	(darray_size(arr) == 0)

#define darray_grow(arr, amount)									\
do {													\
	if (!(arr)) {											\
		size_t *__p = malloc((amount) * sizeof(*(arr)) + (sizeof(size_t) * 2));			\
		assert(__p);										\
		(arr) = (void *)(&__p[2]);								\
		darray_set_capacity((arr), (amount));							\
		darray_set_size((arr), 0);								\
	}												\
	else {												\
		size_t *__p1 = &((size_t *)(arr))[-2];							\
		size_t *__p2 = realloc(__p1, ((amount) * sizeof(*(arr)) + (sizeof(size_t) * 2))); 	\
		assert(__p2);										\
		(arr) = (void *)(&__p2[2]);								\
		darray_set_capacity((arr), (amount));							\
	}												\
} while(0);

#define darray_pop_back(arr)				\
do {							\
	darray_set_size((arr), darray_size(arr) - 1);	\
} while(0);

#define darray_erase(arr, i)						\
do {									\
	if (arr) {							\
		const size_t __sz = darray_size(arr);			\
		if ((i) < __sz) {					\
			darray_set_size((arr), __sz - 1);		\
									\
			size_t __x;					\
			for (__x = (i); __x < (__sz - 1); ++__x) {	\
				(arr)[__x] = (arr)[__x + 1];		\
			}						\
		}							\
	}								\
} while(0)

#define darray_free(arr)				\
do {							\
	if (arr) {					\
		size_t *p1 = &((size_t *)(arr))[-2];	\
		free(p1);				\
	}						\
} while(0);

#define darray_begin(arr)				\
	(arr)

#define darray_end(arr)					\
	((arr) ? &((arr)[darray_size(arr)]) : NULL)

#ifdef LOGARITHMIC_GROWTH

#define darray_push_back(arr, val)					\
do {									\
	size_t __cap = darray_capacity(arr);				\
	if (__cap <= darray_size(arr)) {				\
		darray_grow((arr), !__cap ? __cap + 1 : __cap * 2);	\
	}								\
	arr[darray_size(arr)] = (val);					\
	darray_set_size((arr), darray_size(arr) + 1);			\
} while(0)

#else

#define darray_push_back(arr, val)					\
do {									\
	size_t __cap = darray_capacity(arr);				\
	if (__cap <= darray_size(arr)) {				\
		darray_grow((arr), __cap + 1);				\
	}								\
	arr[darray_size(arr)] = (val);					\
	darray_set_size((arr), darray_size(arr) + 1);			\
} while(0)

#endif

#endif
