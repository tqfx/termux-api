/*!
 @file dialog_speech.c
 @brief Test termux api dialog speech
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>
#include <limits.h>

int main(void)
{
    char *out = 0;
    int ok = termux_dialog_speech("提示", "标题", &out);
    printf("%i %s\n", ok, out ? out : "");
    free(out);
    return 0;
}
