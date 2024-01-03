#include "raylib.h"
#include "../../Array.c"
#pragma once

static bool _kjfeefnje(Array array)
{
    for (size_t i = 0; i < array->len - 1; i++)
        if (Array_swap(array, i, GetRandomValue(i, array->len - 1)) == ARRAY_ERR)
            return false;
    return true;
}

/**
 * @brief Shuffles an `Array` using Raylib's random number function.
 *
 * @param array The `Array` to shuffle
 * @return Whether the operation was successful
 */
Algorithm StandardShuffle = {_kjfeefnje, "Standard Shuffle"};