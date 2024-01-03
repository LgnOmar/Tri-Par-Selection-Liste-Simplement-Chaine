#pragma once

#include "raylib.h"
#include <stdio.h>
#include <string.h>

/** @brief Allocate memory using the same memory allocator as the `Array` functions. */
#define Array_mem_alloc MemAlloc
/** @brief Reallocate memory using the same memory allocator as the `Array` functions. */
#define Array_mem_realloc MemRealloc
/** @brief Deallocate memory using the same memory allocator as the `Array` functions. */
#define Array_mem_free MemFree

/**
 * @brief Represents an array used in the sorting algorithm visualizer
 */
typedef struct Array
{
    unsigned int *_arr;
    size_t len;
} * Array;

/**
 * @brief Designates whether an index operation of an `Array` was successful.
 * `ARRAY_OK`: The operation was successful.
 * `ARRAY_ERR`: The operation was unsuccessful.
 */
typedef enum Array_ResultCondition
{
    ARRAY_OK,
    ARRAY_ERR
} Array_ResultCondition;

/**
 * @brief Respresents the result of an `Array` index operation.
 * The `condition` property designates whether the operation was successful.
 * If `condition` is `ARRAY_OK`, `value` will contain the result of the operation.
 * If `condition` is `ARRAY_ERR`, `value` is redundant.
 */
typedef struct Array_Result
{
    Array_ResultCondition condition;
    unsigned int value;
} Array_Result;

/**
 * @brief Similar to `Array_Result` but with a `bool` as the `value` property
 * @see Array_Result
 */
typedef struct Array_Result_Bool
{
    Array_ResultCondition condition;
    bool value;
} Array_Result_Bool;

/**
 * @brief Macro that propagates the error state of an `Array_Result` or `Array_Result_Bool`
 * by returning `false` out of the current function if `maybe_err`'s `condition` property is `ARRAY_ERR`
 */
#define Array_propagate_err(maybe_err)    \
    if (maybe_err.condition == ARRAY_ERR) \
        return false;

/**
 * @brief Struct type representing a sorting algorithm.
 * `fun` is a function pointer which takes an `Array` and sorts it in place,
 * returning `false` if there were any abnormalities and `true` otherwise.
 * `name` is a null-terminated `const char *` holding the name of the sorting algorithm.
 */
typedef const struct Algorithm
{
    bool (*fun)(Array);
    const char *name;
} Algorithm;

/**
 * @brief A type for an array access callback function pointer.
 * It takes an `Array` (the array accessed) and a `size_t` (the index of the `Array` that was accessed).
 */
typedef void (*Array_CallbackType)(Array, size_t);
/** @brief Internal value */
static void _Array_default_callback(Array array, size_t index) {}
/** @brief Internal value */
static Array_CallbackType _Array_at_callback = _Array_default_callback;
/** @brief Internal value */
static Array_CallbackType _Array_set_callback = _Array_default_callback;

/**
 * @brief Sets `_Array_at_callback`, the callback invoked every time Array_set is.
 *
 * @param callback What to set `_Array_at_callback` to
 */
void Array_set_at_callback(Array_CallbackType callback)
{
    _Array_at_callback = callback;
}

/**
 * @brief Sets `_Array_at_callback`, the callback invoked every time Array_set is.
 *
 * @param callback What to set `_Array_at_callback` to
 */
void Array_set_set_callback(Array_CallbackType callback)
{
    _Array_set_callback = callback;
}

/**
 * @brief Creates a new `Array` of length `len` whose items are all initalized to zero
 *
 * @param len The length of the `Array` to create
 */
Array Array_new(size_t len)
{
    Array returned = Array_mem_alloc(sizeof(struct Array));
    returned->len = len;
    returned->_arr = Array_mem_alloc(len * sizeof(unsigned int));
    memset(returned->_arr, 0, sizeof(unsigned int) * len);
    return returned;
}

/**
 * @brief Frees the memory allocated to the inputted `Array`
 *
 * @param array The `Array` whose memory is to be freed
 * WARNING: After an `Array` is freed, it will become unusable; using it may cause a segmentation fault.
 */
void Array_free(Array array)
{
    Array_mem_free(array->_arr);
    Array_mem_free(array);
}

/**
 * @brief Indexes into an `Array`.
 *
 * @param array The `Array` to be indexed into
 * @param index The index to index into the `Array`
 * @return An `Array_Result` struct containing whether the index was successful, and if successful, the result of the index
 * @note If it is defined, invokes `_Array_at_callback` with the `Array` indexed into and the index it was indexed to.
 * `_Array_at_callback` is of type `void(Array, size_t)`.
 * @see Array_Result
 * @see Array_set_at_callback
 */
Array_Result Array_at(Array array, size_t index)
{
    if (index >= array->len)
        return (Array_Result){ARRAY_ERR};
    unsigned int returned = array->_arr[index];
    _Array_at_callback(array, index);
    return (Array_Result){ARRAY_OK, returned};
}

/**
 * @brief Sets an index at an `Array`.
 *
 * @param array The `Array` to modify
 * @param index The index of the `Array` to modify
 * @param value The value to replace the current value at the specified index
 * @return `ARRAY_ERR` if `index` is past the end of `array`; `ARRAY_OK` otherwise
 * @note If it is defined, invokes `_Array_set_callback` with the `Array` indexed into and the index it was indexed to.
 * `_Array_set_callback` is of type `void(Array, size_t)`.
 * @see Array_set_set_callback
 */
Array_ResultCondition Array_set(Array array, size_t index, unsigned int value)
{
    if (index >= array->len)
        return ARRAY_ERR;
    array->_arr[index] = value;
    _Array_set_callback(array, index);
    return ARRAY_OK;
}

/**
 * @brief Creates a new `Array` of length `len` with items beginning at 0 and increasing by 1 for each item
 *
 * @param len The length of the `Array` to create
 */
Array Array_new_init(size_t len)
{
    Array returned = Array_new(len);
    for (size_t i = 0; i < len; i++)
        if (Array_set(returned, i, i) == ARRAY_ERR)
        {
            Array_mem_free(returned);
            return NULL;
        }
    return returned;
}

/**
 * @brief Pushes a value to the end of an `Array`
 *
 * @param array The `Array` to push an item to
 * @param value The item to push to `array`
 * @return `Array_ResultCondition` specifying whether the underlying `Array_set` call was successful.
 * Regardless of the return value, `array`'s length will **always** be incremented and its internal memory reallocated.
 */
Array_ResultCondition Array_push(Array array, unsigned int value)
{
    size_t array_prev_len = array->len;
    array->len++;
    array->_arr = Array_mem_realloc(array->_arr, array->len * sizeof(unsigned int));
    return Array_set(array, array_prev_len, value);
}

/**
 * @brief Pops a value off the end of an `Array`
 *
 * @param array The `Array` to pop a value off of
 * @return An `Array_Result`. If the return value's `condition` parameter is `ARRAY_ERR`,
 * `array`'s length was not decremented and its internal memory was not reallocated.
 */
Array_Result Array_pop(Array array)
{
    if (!array->len)
        return (Array_Result){ARRAY_ERR};
    size_t new_array_len = array->len - 1;
    Array_Result returned = Array_at(array, new_array_len);
    if (returned.condition == ARRAY_ERR)
        return returned;
    array->_arr = Array_mem_realloc(array->_arr, new_array_len);
    array->len = new_array_len;
    return returned;
}

/**
 * @brief Swaps the values of `array` at the two indices specified
 *
 * @param array The `Array` to modify
 * @param index1 The first index to swap
 * @param index2 The second index to swap
 * @return `ARRAY_ERR` if any of the internal calls failed; `ARRAY_OK` otherwise.
 * If `ARRAY_ERR` was returned, this also means that the function call returned prematurely.
 */
Array_ResultCondition Array_swap(Array array, size_t index1, size_t index2)
{
    Array_Result value1 = Array_at(array, index1);
    if (value1.condition == ARRAY_ERR)
        return ARRAY_ERR;
    Array_Result value2 = Array_at(array, index2);
    if (value2.condition == ARRAY_ERR)
        return ARRAY_ERR;
    if (Array_set(array, index1, value2.value) == ARRAY_ERR)
        return ARRAY_ERR;
    if (Array_set(array, index2, value1.value) == ARRAY_ERR)
        return ARRAY_ERR;
    return ARRAY_OK;
}

/**
 * @brief Reorders two elements in an `Array` such that the larger of the two elements is later in the array.
 *
 * @param array The `Array` to modify
 * @param index1 The index of the first item to reorder
 * @param index2 The index of the second item to reorder
 * @return an `Array_Result_Bool` whose `condition` property specifies whether the operation was successful
 * and whose `value` property specifies whether the values at the two indices were swapped
 */
Array_Result_Bool Array_reorder(Array array, size_t index1, size_t index2)
{
    Array_Result value1 = Array_at(array, index1);
    if (value1.condition == ARRAY_ERR)
        return (Array_Result_Bool){ARRAY_ERR};
    /** @brief `value1.value` */
    unsigned int v1v = value1.value;
    Array_Result value2 = Array_at(array, index2);
    if (value2.condition == ARRAY_ERR)
        return (Array_Result_Bool){ARRAY_ERR};
    /** @brief `value2.value` */
    unsigned int v2v = value2.value;
    if (v1v == v2v || (!(index1 > index2 || v1v > v2v) || (index1 > index2 && v1v > v2v)))
        return (Array_Result_Bool){ARRAY_OK, false};
    if (Array_set(array, index1, v2v) == ARRAY_ERR)
        return (Array_Result_Bool){ARRAY_ERR};
    if (Array_set(array, index2, v1v) == ARRAY_ERR)
        return (Array_Result_Bool){ARRAY_ERR};
    return (Array_Result_Bool){ARRAY_OK, true};
}

/**
 * @brief Returns a copy of an `Array`
 *
 * @param array the `Array` to copy
 * @return `NULL` if an underlying call to `Array_at` or `Array_set` is unsuccessful; the resulting `Array` otherwise
 */
Array Array_copy(Array array)
{
    Array returned = Array_new(array->len);
    for (size_t i = 0; i < returned->len; i++)
    {
        Array_Result value = Array_at(array, i);
        if (value.condition == ARRAY_ERR || Array_set(returned, i, value.value) == ARRAY_ERR)
        {
            Array_free(returned);
            return NULL;
        }
    }
    return returned;
}

/**
 * @brief Reverses the order of the elements in an `Array`
 *
 * @param array The `Array` to reverse
 * @return an `Array_ResultCondition`
 */
Array_ResultCondition Array_reverse(Array array)
{
    for (size_t i = 0; i < array->len >> 1; i++)
        if (Array_swap(array, i, array->len - 1 - i) == ARRAY_ERR)
            return ARRAY_ERR;
    return ARRAY_OK;
}