/*!
 @file termux.c
 @brief Test termux api
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    if (termux_init() == 0)
    {
        printf("init ok\n");

        termux_exit();
    }
    return 0;
}
