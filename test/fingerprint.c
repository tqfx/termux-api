/*!
 @file fingerprint.c
 @brief Test termux api fingerprint
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    int ok = termux_fingerprint("标题", "描述", "子标题", "取消");

    printf("%i ", ok);
    switch (ok)
    {
    case 0:
        printf("success");
        break;
    case 1:
        printf("failure");
        break;
    case 2:
        printf("unknown");
        break;
    default:
        break;
    }
    putchar('\n');
    return 0;
}
