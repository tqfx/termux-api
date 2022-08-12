/*!
 @file dialog_time.c
 @brief Test pipeline operation dialog time
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    int ok = termux_dialog_time("标题");
    printf("%i\n", ok);
    return 0;
}
