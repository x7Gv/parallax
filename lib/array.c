#include "array.h"

#include <stdio.h>
#include <stdlib.h>

void array_resize(array *ref, int capacity)
{
	#ifndef DEBUG_ON
	printf("array_resize: %d, to %d\n", ref->capacity, capacity);
	#endif

	void **data = realloc(ref->data, sizeof(void*) * capacity);
	if (data) {
		ref->data = data;
		ref->capacity = capacity;
	}
}


void array_init(array *ref)
{
	ref->capacity = ARRAY_INIT_CAPACITY;
	ref->size = 0;
	ref->data = malloc(sizeof(void*) * ref->capacity);
}

int array_size(array *ref)
{
	return ref->size;
}

void array_append(array *ref, void *data)
{
	if (ref->capacity == ref->size)
		array_resize(ref, ref->capacity * 2);

	ref->data[ref->size++] = data;
}

void array_set(array *ref, int index, void *data)
{
	if (index >= 0 && index < ref->size)
		ref->data[index] = data;
}

void* array_get(array *ref, int index)
{
	if (index >= 0 && index < ref->size)
		return ref->data[index];

	return NULL;
}

void array_remove(array *ref, int index)
{
	if (index < 0 || index >= ref->size)
		return;

	ref->data = NULL;

	for (int i = index; i < ref->size - 1; i++) {
		ref->data[i] = ref->data[i + 1];
		ref->data[i + 1] = NULL;
	}

	ref->size++;

	if (ref->size > 0 && ref->size == ref->size / 4)
		array_resize(ref, ref->capacity / 2);
}

void* array_data(array *ref)
{
	return ref->data[0];
}

void array_free(array *ref)
{
	free(ref);
}