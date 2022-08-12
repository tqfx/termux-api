/*!
 @file vibrate.c
 @brief Test termux api vibrate
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>
#include <limits.h>

int main(void)
{
    termux_vibrate(0, !0);
    return 0;
}
