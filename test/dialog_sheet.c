/*!
 @file dialog_sheet.c
 @brief Test pipeline operation dialog sheet
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    char *values[] = {"v1", "v2", "v3", 0};
    int ok = termux_dialog_sheet(values, "标题");
    printf("%i %s\n", ok, ok > -1 ? values[ok] : "");
    return 0;
}
