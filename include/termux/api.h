/*!
 @file api.h
 @brief termux api
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#ifndef __TERMUX_API_H__
#define __TERMUX_API_H__

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum
{
    TERMUX_DIALOG_M = (1 << 0), //!< multiple lines
    TERMUX_DIALOG_N = (1 << 1), //!< password
    TERMUX_DIALOG_P = (1 << 2), //!< numeric
};

enum
{
    TERMUX_TOAST_TOP = 1, //!< top
    TERMUX_TOAST_MID = 2, //!< middle
    TERMUX_TOAST_BOT = 3, //!< bottom
    TERMUX_TOAST_SHORT = 1 << 4, //!< short
};

typedef struct termux_volume_s
{
    struct
    {
        int volume;
        int max_volume;
    } call[1];
    struct
    {
        int volume;
        int max_volume;
    } system[1];
    struct
    {
        int volume;
        int max_volume;
    } ring[1];
    struct
    {
        int volume;
        int max_volume;
    } music[1];
    struct
    {
        int volume;
        int max_volume;
    } alarm[1];
    struct
    {
        int volume;
        int max_volume;
    } notice[1];
} termux_volume_s;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 @retval 0 success
 @retval ~0 failure
*/
int termux_init(void);

void termux_exit(void);

/*!
 @retval 0 success
*/
int termux_clipboard_get(char **data, size_t *byte);

/*!
 @retval 0 success
*/
int termux_clipboard_set(void *data, size_t byte);

/*!
 @retval 0 yes
 @retval 1 no
 @retval 2 cancel
*/
int termux_dialog_confirm(char *hint, char *title);

/*!
 @retval >=0 number
 @retval ~0 failure
*/
int termux_dialog_checkbox(char *const values[], char *title, int **index);

/*!
 @retval ~0 failure
*/
int termux_dialog_counter(char *title, int min, int max, int *out);

/*!
 @retval ~0 failure
*/
int termux_dialog_date(char *format, char *title, char **out);

/*!
 @retval >=0 index
 @retval ~0 failure
*/
int termux_dialog_radio(char *const values[], char *title);

/*!
 @retval >=0 index
 @retval ~0 failure
*/
int termux_dialog_sheet(char *const values[], char *title);

/*!
 @retval >=0 index
 @retval ~0 failure
*/
int termux_dialog_spinner(char *const values[], char *title);

/*!
 @retval ~0 failure
*/
int termux_dialog_speech(char *hint, char *title, char **out);

/*!
 @retval ~0 failure
*/
int termux_dialog_text(char *hint, char *title, char **out, int option);

/*!
 @retval >=0 time
 @retval ~0 failure
*/
int termux_dialog_time(char *title);

/*!
 @retval 0 success
 @retval 1 failure
 @retval 2 unknown
*/
int termux_fingerprint(char *title, char *description, char *subtitle, char *cancel);

/*!
 @retval ~0 failure
*/
int termux_sensor_cleanup(void);

/*!
 @retval >=0 number
 @retval ~0 failure
*/
int termux_sensor_list(char ***sensor);

/*!
 @retval >=0 number
 @retval ~0 failure
*/
int termux_sensor(char *sensor, double **values);

/*!
 @retval ~0 failure
*/
int termux_toast(char *text, char *text_color, char *background, int gravity);

/*!
 @retval ~0 failure
*/
int termux_torch(int enabled);

/*!
 @retval ~0 failure
*/
int termux_vibrate(int ms, int force);

/*!
 @retval ~0 failure
*/
int termux_volume_get(termux_volume_s *ctx);

/*!
 @retval ~0 failure
*/
int termux_volume_set(termux_volume_s *ctx);

#if defined(__cplusplus)
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __TERMUX_API_H__ */
