/*!
 @file toast.c
 @brief Test termux api toast
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    int ok = termux_toast("文本", 0, 0, TERMUX_TOAST_SHORT | TERMUX_TOAST_TOP);
    if (ok)
    {
        printf("%i\n", ok);
    }
    return 0;
}
