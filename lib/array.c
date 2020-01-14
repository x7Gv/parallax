#include "array.h"

void array_resize(array *ref, int capacity, bool set_size)
{
	#ifndef DEBUG_OFF
	printf("array_resize: %d, to %d\n", ref->capacity, capacity);
	#endif

	if (set_size)
		ref->size = capacity;

	char *data = realloc(ref->data, ref->member_size * capacity);
	if (data) {
		ref->data = data;
		ref->capacity = capacity;
	}
}

void array_init(array *ref, size_t member_size)
{
	ref->member_size = member_size;

	ref->capacity = ARRAY_INIT_CAPACITY;
	ref->size = 0;
	ref->data = malloc(ref->capacity * member_size);
}

int array_size(array *ref)
{
	return ref->size;
}

void array_append(array *ref, void *data)
{
	if (ref->capacity <= ref->size)
		array_resize(ref, ref->capacity * 2, false);

	memcpy(&ref->data[ref->member_size * ref->size++], data, ref->member_size);
}

void array_set(array *ref, int index, void *data)
{
	if (index >= 0 && index < ref->size)
		memcpy(&ref->data[ref->member_size * index], data, ref->member_size);
}

void *array_get(array *ref, int index)
{

	if (index >= 0 && index < ref->size) {

		void *p = malloc(ref->member_size);
		memcpy(p, &ref->data[ref->member_size * index], ref->member_size);

		return p;
	}

	return 0;
}

char* array_data(array *ref)
{
	return ref->data;
}

void array_free(array *ref)
{
	free(ref->data);
}