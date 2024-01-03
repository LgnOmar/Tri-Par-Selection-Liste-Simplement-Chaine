#include "../../Array.c"

static bool _ewweew(Array array)
{
    for (size_t i = 0; i < array->len - 1; i++)
    {
        size_t min_index = i;
        Array_Result min_value = Array_at(array, min_index);
        Array_propagate_err(min_value);
        for (size_t j = i + 1; j < array->len; j++)
        {
            Array_Result j_val = Array_at(array, j);
            Array_propagate_err(j_val);
            if (j_val.value < min_value.value)
            {
                min_index = j;
                min_value.value = j_val.value;
            }
        }
        if (Array_swap(array, min_index, i) == ARRAY_ERR)
            return false;
    }
    return true;
}

Algorithm SelectionSort = {_ewweew, "Selection Sort"};