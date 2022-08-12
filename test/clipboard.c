/*!
 @file clipboard.c
 @brief Test termux api clipboard
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    char *data = 0;
    size_t byte = 0;
    termux_clipboard_set("ok", 2);
    if (termux_clipboard_get(&data, &byte) == 0)
    {
        printf("%s\n", data);
    }
    return 0;
}
