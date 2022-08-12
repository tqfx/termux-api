/*!
 @file dialog_date.c
 @brief Test termux api dialog date
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>
#include <limits.h>

int main(void)
{
    char *out = 0;
    int ok = termux_dialog_date("yyyy-MM-dd k:m:s", "标题", &out);
    printf("%i %s\n", ok, out);
    free(out);
    return 0;
}
