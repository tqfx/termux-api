/*!
 @file torch.c
 @brief Test termux api torch
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <unistd.h>

int main(void)
{
    termux_torch(1);
    sleep(1);
    termux_torch(0);
    return 0;
}
