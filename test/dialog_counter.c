/*!
 @file dialog_counter.c
 @brief Test termux api dialog counter
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>
#include <limits.h>

int main(void)
{
    int out = INT_MIN;
    int ok = termux_dialog_counter("标题", INT_MIN, INT_MAX, &out);
    printf("%i %i\n", ok, out);
    return 0;
}
