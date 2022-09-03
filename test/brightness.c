/*!
 @file brightness.c
 @brief Test termux api brightness
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>
#include <unistd.h>

int main(void)
{
    termux_brightness(0);
    sleep(1);
    termux_brightness(255);
    sleep(1);
    termux_brightness(~0);
    return 0;
}
