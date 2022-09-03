/*!
 @file sensor.c
 @brief Test termux api sensor
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include <stdio.h>

int main(void)
{
    char **sensor = 0;
    int n = termux_sensor_list(&sensor);
    for (int i = 0; i < n; ++i)
    {
        double *values = 0;
        printf("\"%s\":", sensor[i]);
        int m = termux_sensor(sensor[i], &values);
        free(sensor[i]);
        for (int j = 0; j < m; ++j)
        {
            if (j == 0)
            {
                putchar('[');
            }
            printf("%g", values[j]);
            putchar(j < (m - 1) ? ',' : ']');
        }
        putchar('\n');
        free(values);
    }
    termux_sensor_cleanup();
    free(sensor);
    return 0;
}
