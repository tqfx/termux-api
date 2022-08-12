/*!
 @file volume.c
 @brief Test pipeline operation volume
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

static void display(termux_volume_s *ctx)
{
    printf("--------------------\n");
    printf("|%12s|%2i|%2i|\n", "call", ctx->call->max_volume, ctx->call->volume);
    printf("|%12s|%2i|%2i|\n", "system", ctx->system->max_volume, ctx->system->volume);
    printf("|%12s|%2i|%2i|\n", "ring", ctx->ring->max_volume, ctx->ring->volume);
    printf("|%12s|%2i|%2i|\n", "music", ctx->music->max_volume, ctx->music->volume);
    printf("|%12s|%2i|%2i|\n", "alarm", ctx->alarm->max_volume, ctx->alarm->volume);
    printf("|%12s|%2i|%2i|\n", "notification", ctx->notice->max_volume, ctx->notice->volume);
    printf("--------------------\n");
}

int main(void)
{
    termux_volume_s ctx[1];
    int ok = termux_volume_get(ctx);
    if (ok == 0)
    {
        display(ctx);
        ctx->call->volume = 15;
        ctx->music->volume = 15;
        ctx->alarm->volume = 15;
    }
    ok = termux_volume_set(ctx);
    if (ok == 0)
    {
        display(ctx);
        ctx->call->volume = 0;
        ctx->music->volume = 0;
        ctx->alarm->volume = 0;
    }
    ok = termux_volume_set(ctx);
    if (ok == 0)
    {
        display(ctx);
    }
    return 0;
}
