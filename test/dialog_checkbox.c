/*!
 @file dialog_checkbox.c
 @brief Test pipeline operation dialog checkbox
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    int *index = 0;
    char *values[] = {"v1", "v2", "v3", 0};
    int ok = termux_dialog_checkbox(values, "标题", &index);
    for (int i = 0; i < ok; ++i)
    {
        printf("%i %s\n", index[i], values[index[i]]);
    }
    printf("%i %s\n", ok, ok > -1 ? "ok" : "no");
    free(index);
    return 0;
}
