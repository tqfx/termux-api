/*!
 @file dialog_text.c
 @brief Test termux api dialog text
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    {
        char *out = 0;
        int ok = termux_dialog_text("提示", "标题", &out, 0);
        printf("%i %s\n", ok, out ? out : "");
        free(out);
    }
    {
        char *out = 0;
        int ok = termux_dialog_text("提示", "标题+m", &out, TERMUX_DIALOG_M);
        printf("%i %s\n", ok, out ? out : "");
        free(out);
    }
    {
        char *out = 0;
        int ok = termux_dialog_text("提示", "标题+n", &out, TERMUX_DIALOG_N);
        printf("%i %s\n", ok, out ? out : "");
        free(out);
    }
    {
        char *out = 0;
        int ok = termux_dialog_text("提示", "标题+p", &out, TERMUX_DIALOG_P);
        printf("%i %s\n", ok, out ? out : "");
        free(out);
    }
    {
        char *out = 0;
        int ok = termux_dialog_text("提示", "标题+mp", &out, TERMUX_DIALOG_M | TERMUX_DIALOG_P);
        printf("%i %s\n", ok, out ? out : "");
        free(out);
    }
    {
        char *out = 0;
        int ok = termux_dialog_text("提示", "标题+np", &out, TERMUX_DIALOG_N | TERMUX_DIALOG_P);
        printf("%i %s\n", ok, out ? out : "");
        free(out);
    }
    {
        char *out = 0;
        int ok = termux_dialog_text("提示", "标题+mn", &out, TERMUX_DIALOG_M | TERMUX_DIALOG_N);
        printf("%i %s\n", ok, out ? out : "");
        free(out);
    }
    {
        char *out = 0;
        int ok = termux_dialog_text("提示", "标题+mnp", &out, TERMUX_DIALOG_M | TERMUX_DIALOG_N | TERMUX_DIALOG_P);
        printf("%i %s\n", ok, out ? out : "");
        free(out);
    }
    return 0;
}
