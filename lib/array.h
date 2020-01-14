#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>


#define ARRAY_INIT_CAPACITY 8

/**
 *    // BEGIN // POLYMORPHISM-ISH MACRO EXTENDATION
 */

#define arr_init(arr) array arr; 	array_init(&arr)
#define arr_append(arr, data) 		array_append(&arr, (char*) data)
#define arr_set(arr, index, data)	array_set(&arr, index, (char*) data)
#define arr_get(arr, _type, index) 	*((_type *) array_get(&arr, index))
#define arr_remove(arr, index) 		array_remove(&arr, index)

#define arr_size(arr) array_size(&arr)
#define arr_free(arr) array_free(&arr)

/**
 *    // END // POLYMORPHISM-ISH MACRO EXTENDATION
 */

typedef struct _array array;

/**
 * @brief      Array struct.
 */
struct _array
{
	char *data;
	size_t member_size;

	int capacity;
	int size;
};

/**
 * @brief      Realloc array with new size.
 *
 * @param      ref       Reference to the associated array struct.
 * @param[in]  capacity  New capacity.
 */
void array_resize(array *ref, int capacity, bool set_size);

/**
 * @brief      Initialize array.
 *
 * @param      ref       Reference to the associated array struct.
 */
void array_init(array *ref, size_t member_size);

/**
 * @brief      Get current size of allocated indicies.
 *
 * @param      ref       Reference to the associated array struct.
 *
 * @return     size
 */
int array_size(array *ref);

/**
 * @brief      Append data to the end of array.
 *
 * @param      ref       Reference to the associated array struct.
 * @param      data  	 The data to append.
 */
void array_append(array *ref, void *data);

/**
 * @brief      Set data to given index.
 *
 * @param      ref       Reference to the associated array struct.
 * @param[in]  index     Index of data to set.
 * @param      data      The data to set.
 */
void array_set(array *ref, int index, void *data);

/**
 * @brief      Get data from array by an index.
 *
 * @param      ref       Reference to the associated array struct.
 * @param[in]  index     Index of data to get.
 *
 * @return     Return value from an index if present, NULL otherwise.
 */
void * array_get(array *ref, int index);

char *array_data(array *ref);

/**
 * @brief      Free the memory allocated to array.
 *
 * @param      ref       Reference to the associated array struct.
 */
void array_free(array *ref);

#endif