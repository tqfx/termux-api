/*!
 @file dialog_confirm.c
 @brief Test termux api dialog confirm
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    int ok = termux_dialog_confirm("提示", "标题");
    printf("%i %s\n", ok, ok == 0 ? "yes" : "no");
    return 0;
}
