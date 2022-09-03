/*!
 @file api.c
 @brief termux api
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "termux/api.h"

#include "pipe.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <jansson.h>
#include <sys/wait.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#endif /* __GNUC__ || __clang__ */

/*!
 @brief instance structure for pipeline
*/
typedef struct
{
    FILE *wr;
    FILE *rd;
    pid_t pid;
} api_s;

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif /* __GNUC__ || __clang__ */

static int api_command(int argc, char *argv[])
{
    extern int run_api_command(int, char **);
    int fd = run_api_command(argc, argv);
    if (fd != -1)
    {
        _Noreturn void exec_callback(int);
        exec_callback(fd);
    }
    return 0;
}

#define R 0
#define W 1

__attribute__((unused)) static int api_open(api_s *ctx, int argc, char *argv[])
{
    ctx->wr = 0;
    ctx->rd = 0;
    ctx->pid = ~0;

    /* create two pipes */
    int pipe_wr[2];
    if (pipe(pipe_wr) < 0)
    {
        goto pipe_wr;
    }
    int pipe_rd[2];
    if (pipe(pipe_rd) < 0)
    {
        goto pipe_rd;
    }

    /* create a child process */
    ctx->pid = fork();
    if (ctx->pid < 0)
    {
        goto pipe_rw;
    }

    if (ctx->pid == 0)
    {
        close(pipe_wr[W]);
        close(pipe_rd[R]);

        if (pipe_wr[R] != STDIN_FILENO)
        {
            int ok = dup2(pipe_wr[R], STDIN_FILENO);
            close(pipe_wr[R]);
            if (ok < 0)
            {
                _exit(EXIT_FAILURE);
            }
        }
        if (pipe_rd[W] != STDOUT_FILENO)
        {
            int ok = dup2(pipe_rd[W], STDOUT_FILENO);
            close(pipe_rd[W]);
            if (ok < 0)
            {
                _exit(EXIT_FAILURE);
            }
        }

        _exit(api_command(argc, argv));
    }

    close(pipe_wr[R]);
    close(pipe_rd[W]);

    ctx->wr = fdopen(pipe_wr[W], "w");
    if (ctx->wr == 0)
    {
        goto open_wr;
    }
    ctx->rd = fdopen(pipe_rd[R], "r");
    if (ctx->rd == 0)
    {
        goto open_rd;
    }

    return 0;

open_rd:
    close(pipe_rd[R]);
open_wr:
    close(pipe_wr[W]);

    return ~0;

pipe_rw:
    close(pipe_rd[R]);
    close(pipe_rd[W]);
pipe_rd:
    close(pipe_wr[R]);
    close(pipe_wr[W]);
pipe_wr:
    return ~0;
}

#undef R
#undef W

__attribute__((unused)) static int api_close(api_s *ctx)
{
    int status = 0;

    if (ctx->pid < 0)
    {
        errno = ECHILD;
        return ~0;
    }

    if (ctx->wr && fclose(ctx->wr) == EOF)
    {
        clearerr(ctx->wr);
    }
    ctx->wr = 0;
    if (ctx->rd && fclose(ctx->rd) == EOF)
    {
        clearerr(ctx->rd);
    }
    ctx->rd = 0;

    /* check if the child process to terminate */
    while (waitpid(ctx->pid, &status, WNOHANG) == 0)
    {
        kill(ctx->pid, SIGTERM);
    }
    ctx->pid = ~0;

    /* check if the child process terminated normally */
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    /* check if the child process was terminated by a signal */
    if (WIFSIGNALED(status))
    {
        return WTERMSIG(status);
    }
    /* reached only if the process did not terminate normally */
    errno = ECHILD;
    return status;
}

__attribute__((unused)) static int api_wait(const api_s *ctx, unsigned long ms)
{
    errno = 0;
    int status = 0;
    pid_t wait = waitpid(ctx->pid, &status, WNOHANG);
    if (wait != 0 || ms == 0)
    {
        /* wait for the child process to terminate */
        while (wait == ~0 && errno == EINTR)
        {
            wait = waitpid(ctx->pid, &status, 0);
        }
        goto exit;
    }

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    sigset_t orig_mask;
    if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0)
    {
        return ~0;
    }

    struct timespec timeout = {.tv_sec = ms / 1000, .tv_nsec = (ms % 1000) * 1000000};
    while (sigtimedwait(&mask, NULL, &timeout) < 0)
    {
        if (errno == EAGAIN)
        {
            errno = ETIMEDOUT;
            return ~0;
        }
    }

    if (sigprocmask(SIG_SETMASK, &orig_mask, 0) < 0)
    {
        return ~0;
    }

exit:
    return status;
}

__attribute__((unused)) static int api_flush(const api_s *ctx)
{
    int ok = 0;
    if (ctx->wr)
    {
        ok += fflush(ctx->wr);
    }
    if (ctx->rd)
    {
        ok += fflush(ctx->rd);
    }
    return ok;
}

__attribute__((unused)) static int api_getc(const api_s *ctx)
{
    return fgetc(ctx->rd);
}

__attribute__((unused)) static int api_putc(const api_s *ctx, int c)
{
    return fputc(c, ctx->wr);
}

__attribute__((unused)) static int api_puts(const api_s *ctx, const char *str)
{
    return fputs(str, ctx->wr);
}

__attribute__((unused)) static size_t api_read(const api_s *ctx, void *data, size_t byte)
{
    return fread(data, 1, byte, ctx->rd);
}

__attribute__((unused)) static size_t api_write(const api_s *ctx, const void *data, size_t byte)
{
    return fwrite(data, 1, byte, ctx->wr);
}

__attribute__((unused)) static int __attribute__((format(printf, 2, 3))) api_printf(const api_s *ctx, const char *fmt, ...)
{
    int stats;
    va_list va;
    va_start(va, fmt);
    stats = vfprintf(ctx->wr, fmt, va);
    va_end(va);
    return stats;
}

__attribute__((unused)) static void argv_display(char *const argv[])
{
    while (*argv)
    {
        printf("%s ", *argv++);
    }
    putchar('\n');
}

static int pipe_exec(int argc, char *argv[], unsigned long timeout)
{
    api_s ctx[1];
    api_open(ctx, argc, argv);
    api_wait(ctx, timeout);
    return api_close(ctx);
}

static int write_text(int argc, char *argv[], void *data, size_t byte)
{
    api_s ctx[1];
    api_open(ctx, argc, argv);
    api_write(ctx, data, byte);
    return api_close(ctx);
}

static int read_text(int argc, char *argv[], char **data, size_t *byte)
{
    api_s ctx[1];
    api_open(ctx, argc, argv);
    if (data && byte)
    {
        *data = 0;
        *byte = 0;
        for (size_t size = BUFSIZ; size == BUFSIZ; *byte += size)
        {
            char buff[BUFSIZ];
            size = api_read(ctx, buff, BUFSIZ);
            if (size)
            {
                char *output = (char *)realloc(*data, size + 1);
                if (output == 0)
                {
                    break;
                }
                *data = output;
                memcpy(*data + *byte, buff, size);
            }
        }
        if (*byte)
        {
            (*data)[*byte] = 0;
        }
    }
    return api_close(ctx);
}

static json_t *read_json(int argc, char *argv[])
{
    api_s ctx[1];
    if (api_open(ctx, argc, argv))
    {
        return 0;
    }
    json_error_t error;
    json_t *root = json_loadf(ctx->rd, 0, &error);
    api_close(ctx);
    return root;
}

int termux_init(void)
{
    char buf[64];
    pipe_s ctx[1];
    pipe_open3(ctx,
               "/data/data/com.termux/files/usr/bin/am",
               (char *[]){"am", "startservice", "-n", "com.termux.api/.KeepAliveService"},
               0);
    fread(buf, 1, 100, ctx->er);
    pipe_close(ctx);
    if (*buf != 'E')
    {
        return 0;
    }
    fputs(buf, stderr);
    return ~0;
}

void termux_exit(void)
{
    pipe_s ctx[1];
    pipe_open3(ctx,
               "/data/data/com.termux/files/usr/bin/am",
               (char *[]){"am", "stopservice", "-n", "com.termux.api/.KeepAliveService"},
               0);
    pipe_wait(ctx, 0);
    pipe_close(ctx);
}

int termux_clipboard_get(char **data, size_t *byte)
{
    int argc = 2;
    char *argv[3] = {0, "Clipboard", 0};
    return read_text(argc, argv, data, byte);
}

int termux_clipboard_set(void *data, size_t byte)
{
    int argc = 8;
    char *argv[9] = {0, "Clipboard", "-e", "api_version", "2", "--ez", "set", "true", 0};
    return write_text(argc, argv, data, byte);
}

static char *dialog_line(char *const values[])
{
    size_t size = 1;
    size_t cur = 0;
    char *line = 0;
    if (values == 0 || *values == 0)
    {
        goto exit;
    }

    for (char *const *strv = values; *strv; ++strv)
    {
        size += strlen(*strv) + 1;
    }

    line = (char *)malloc(size);
    if (line == 0)
    {
        goto exit;
    }

    for (char *const *strv = values; *strv; ++strv)
    {
        strcpy(line + cur, *strv);
        cur += strlen(*strv) + 1;
        line[cur - 1] = ',';
    }
    line[cur - 1] = 0;

exit:
    return line;
}

int termux_dialog_confirm(char *hint, char *title)
{
    int ok = ~0;
    int argc = 5;
    char *argv[12] = {0, "Dialog", "--es", "input_method", "confirm"};
    char *input_hint = hint;
    if (input_hint)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_hint";
        argv[argc++] = input_hint;
    }
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, "text");
        const char *string = json_string_value(object);
        if (strcmp(string, "yes") == 0)
        {
            ok = 0;
        }
        else if (strcmp(string, "no") == 0)
        {
            ok = 1;
        }
        else
        {
            ok = 2;
        }
        json_decref(root);
    }
    return ok;
}

int termux_dialog_checkbox(char *const values[], char *title, int **index)
{
    int ok = ~0;
    int argc = 5;
    char *argv[12] = {0, "Dialog", "--es", "input_method", "checkbox"};
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    char *input_values = dialog_line(values);
    if (input_values)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_values";
        argv[argc++] = input_values;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    free(input_values);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        json_int_t code = json_integer_value(object);
        if (code == -1)
        {
            object = json_object_get(root, "values");
            size_t n = json_array_size(object);
            if (n)
            {
                *index = (int *)malloc(sizeof(int) * n);
            }
            for (size_t i = 0; i != n; ++i)
            {
                json_t *item = json_array_get(object, i);
                (*index)[i] = (int)json_integer_value(json_object_get(item, "index"));
            }
            ok = (int)n;
        }
        json_decref(root);
    }
    return ok;
}

int termux_dialog_counter(char *title, int min, int max, int *out)
{
    int ok = ~0;
    int argc = 5;
    char *argv[12] = {0, "Dialog", "--es", "input_method", "counter"};
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    char input_range[64];
    snprintf(input_range, 64, "%i,%i,%i", min, max, out ? *out : 0);
    argv[argc++] = "--eia";
    argv[argc++] = "input_range";
    argv[argc++] = input_range;
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        json_int_t code = json_integer_value(object);
        if (code == -1)
        {
            object = json_object_get(root, "text");
            *out = atoi(json_string_value(object));
            ok = 0;
        }
        json_decref(root);
    }
    return ok;
}

int termux_dialog_date(char *format, char *title, char **out)
{
    int ok = ~0;
    int argc = 5;
    char *argv[12] = {0, "Dialog", "--es", "input_method", "date"};
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    char *date_format = format;
    if (date_format)
    {
        argv[argc++] = "--es";
        argv[argc++] = "date_format";
        argv[argc++] = date_format;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        json_int_t code = json_integer_value(object);
        if (code == -1)
        {
            object = json_object_get(root, "text");
            *out = strdup(json_string_value(object));
            ok = 0;
        }
        json_decref(root);
    }
    return ok;
}

int termux_dialog_radio(char *const values[], char *title)
{
    int ok = ~0;
    int argc = 5;
    char *argv[12] = {0, "Dialog", "--es", "input_method", "radio"};
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    char *input_values = dialog_line(values);
    if (input_values)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_values";
        argv[argc++] = input_values;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    free(input_values);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        json_int_t code = json_integer_value(object);
        if (code == -1)
        {
            object = json_object_get(root, "index");
            if (object)
            {
                ok = (int)json_integer_value(object);
            }
        }
        json_decref(root);
    }
    return ok;
}

int termux_dialog_sheet(char *const values[], char *title)
{
    int ok = ~0;
    int argc = 5;
    char *argv[12] = {0, "Dialog", "--es", "input_method", "sheet"};
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    char *input_values = dialog_line(values);
    if (input_values)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_values";
        argv[argc++] = input_values;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    free(input_values);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        json_int_t code = json_integer_value(object);
        if (code == 0)
        {
            object = json_object_get(root, "index");
            if (object)
            {
                ok = (int)json_integer_value(object);
            }
        }
        json_decref(root);
    }
    return ok;
}

int termux_dialog_spinner(char *const values[], char *title)
{
    int ok = ~0;
    int argc = 5;
    char *argv[12] = {0, "Dialog", "--es", "input_method", "spinner"};
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    char *input_values = dialog_line(values);
    if (input_values)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_values";
        argv[argc++] = input_values;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    free(input_values);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        json_int_t code = json_integer_value(object);
        if (code == -1)
        {
            object = json_object_get(root, "index");
            if (object)
            {
                ok = (int)json_integer_value(object);
            }
        }
        json_decref(root);
    }
    return ok;
}

int termux_dialog_speech(char *hint, char *title, char **out)
{
    int ok = ~0;
    int argc = 5;
    char *argv[12] = {0, "Dialog", "--es", "input_method", "speech"};
    char *input_hint = hint;
    if (input_hint)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_hint";
        argv[argc++] = input_hint;
    }
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        if (object == 0)
        {
            object = json_object_get(root, "error");
            if (object)
            {
                fprintf(stderr, "%s\n", json_string_value(object));
            }
            ok = ~1;
            goto done;
        }
        json_int_t code = json_integer_value(object);
        if (code == 0)
        {
            object = json_object_get(root, "text");
            *out = strdup(json_string_value(object));
            ok = 0;
        }
    done:
        json_decref(root);
    }
    return ok;
}

int termux_dialog_text(char *hint, char *title, char **out, int option)
{
    int ok = ~0;
    int argc = 5;
    char *argv[21] = {0, "Dialog", "--es", "input_method", "text"};
    char *input_hint = hint;
    if (input_hint)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_hint";
        argv[argc++] = input_hint;
    }
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    if (option & TERMUX_DIALOG_M)
    {
        argv[argc++] = "--ez";
        argv[argc++] = "multiple_lines";
        argv[argc++] = "true";
    }
    if (option & TERMUX_DIALOG_P)
    {
        argv[argc++] = "--ez";
        argv[argc++] = "password";
        argv[argc++] = "true";
    }
    if (option & TERMUX_DIALOG_N)
    {
        argv[argc++] = "--ez";
        argv[argc++] = "numeric";
        argv[argc++] = "true";
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        json_int_t code = json_integer_value(object);
        if (code == -1)
        {
            object = json_object_get(root, "text");
            *out = strdup(json_string_value(object));
            ok = 0;
        }
        json_decref(root);
    }
    return ok;
}

int termux_dialog_time(char *title)
{
    int ok = ~0;
    int argc = 5;
    char *argv[9] = {0, "Dialog", "--es", "input_method", "time"};
    char *input_title = title;
    if (input_title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "input_title";
        argv[argc++] = input_title;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, "code");
        json_int_t code = json_integer_value(object);
        if (code == -1)
        {
            int hour, minute;
            object = json_object_get(root, "text");
            const char *string = json_string_value(object);
            sscanf(string, " %i:%i", &hour, &minute);
            ok = hour * 100 + minute;
        }
        json_decref(root);
    }
    return ok;
}

int termux_fingerprint(char *title, char *description, char *subtitle, char *cancel)
{
    int ok = ~0;
    int argc = 2;
    char *argv[15] = {0, "Fingerprint"};
    if (title)
    {
        argv[argc++] = "--es";
        argv[argc++] = "title";
        argv[argc++] = title;
    }
    if (description)
    {
        argv[argc++] = "--es";
        argv[argc++] = "description";
        argv[argc++] = description;
    }
    if (subtitle)
    {
        argv[argc++] = "--es";
        argv[argc++] = "subtitle";
        argv[argc++] = subtitle;
    }
    if (cancel)
    {
        argv[argc++] = "--es";
        argv[argc++] = "cancel";
        argv[argc++] = cancel;
    }
    argv[argc] = 0;
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, "auth_result");
        const char *string = json_string_value(object);
        if (strcmp(string, "AUTH_RESULT_SUCCESS") == 0)
        {
            ok = 0;
        }
        else if (strcmp(string, "AUTH_RESULT_FAILURE") == 0)
        {
            ok = 1;
        }
        else
        {
            ok = 2;
        }
        json_decref(root);
    }
    return ok;
}

int termux_sensor_cleanup(void)
{
    int argc = 4;
    char *argv[5] = {0, "Sensor", "-a", "cleanup", 0};
    return pipe_exec(argc, argv, 1000);
}

int termux_sensor_list(char ***sensor)
{
    int ok = ~0;
    int argc = 4;
    char *argv[5] = {0, "Sensor", "-a", "list", 0};
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, "sensors");
        if (object)
        {
            size_t n = json_array_size(object);
            if (n)
            {
                *sensor = (char **)malloc(sizeof(char *) * n);
            }
            for (size_t i = 0; i != n; ++i)
            {
                json_t *item = json_array_get(object, i);
                (*sensor)[i] = strdup(json_string_value(item));
            }
            ok = (int)n;
        }
        json_decref(root);
    }
    return ok;
}

int termux_sensor(char *sensor, double **values)
{
    int ok = ~0;
    int argc = 10;
    char *argv[11] = {0, "Sensor", "-a", "sensors", "--es", "sensors", sensor, "--ei", "limit", "1", 0};
    json_t *root = read_json(argc, argv);
    if (root)
    {
        json_t *object = json_object_get(root, sensor);
        if (object == 0)
        {
            goto done;
        }
        object = json_object_get(object, "values");
        if (object == 0)
        {
            goto done;
        }
        size_t n = json_array_size(object);
        if (n)
        {
            *values = (double *)malloc(sizeof(double) * n);
        }
        for (size_t i = 0; i != n; ++i)
        {
            json_t *item = json_array_get(object, i);
            if (json_is_number(item))
            {
                (*values)[i] = json_number_value(item);
            }
        }
        ok = (int)n;
    done:
        json_decref(root);
    }
    return ok;
}

int termux_toast(char *text, char *text_color, char *background, int gravity)
{
    int argc = 2;
    char *argv[15] = {0, "Toast"};
    if (gravity & TERMUX_TOAST_SHORT)
    {
        argv[argc++] = "--ez";
        argv[argc++] = "short";
        argv[argc++] = "true";
    }
    if (text_color)
    {
        argv[argc++] = "--es";
        argv[argc++] = "text_color";
        argv[argc++] = text_color;
    }
    if (background)
    {
        argv[argc++] = "--es";
        argv[argc++] = "background";
        argv[argc++] = background;
    }
    gravity &= 0x3;
    if (gravity)
    {
        char *map[] = {"middle", "top", "middle", "bottom"};
        argv[argc++] = "--es";
        argv[argc++] = "gravity";
        argv[argc++] = map[gravity];
    }
    argv[argc] = 0;
    api_s ctx[1];
    int ok = api_open(ctx, argc, argv);
    api_printf(ctx, "%s\n", text);
    api_wait(ctx, 300);
    api_close(ctx);
    return ok;
}

int termux_torch(int enabled)
{
    int argc = 5;
    char *argv[6] = {0, "Torch", "--ez", "enabled", enabled ? "1" : "0", 0};
    return pipe_exec(argc, argv, 1000);
}

int termux_vibrate(int ms, int force)
{
    int argc = 5;
    char buff[16];
    char *argv[9] = {0, "Vibrate", "--ei", "duration_ms", buff};
    snprintf(buff, 16, "%i", ms > 0 ? ms : 1000);
    if (force)
    {
        argv[argc++] = "--ez";
        argv[argc++] = "force";
        argv[argc++] = "true";
    }
    argv[argc] = 0;
    return pipe_exec(argc, argv, 1000);
}

int termux_volume_get(termux_volume_s *ctx)
{
    int ok = ~0;
    int argc = 2;
    char *argv[3] = {0, "Volume", 0};
    json_t *root = read_json(argc, argv);
    if (root)
    {
        size_t n = json_array_size(root);
        for (size_t i = 0; i != n; ++i)
        {
            json_t *item = json_array_get(root, i);
            json_int_t volume = json_integer_value(json_object_get(item, "volume"));
            json_int_t max_volume = json_integer_value(json_object_get(item, "max_volume"));
            const char *stream = json_string_value(json_object_get(item, "stream"));
            switch (*stream)
            {
            case 'c':
            {
                ctx->call->volume = (int)volume;
                ctx->call->max_volume = (int)max_volume;
            }
            break;
            case 's':
            {
                ctx->system->volume = (int)volume;
                ctx->system->max_volume = (int)max_volume;
            }
            break;
            case 'r':
            {
                ctx->ring->volume = (int)volume;
                ctx->ring->max_volume = (int)max_volume;
            }
            break;
            case 'm':
            {
                ctx->music->volume = (int)volume;
                ctx->music->max_volume = (int)max_volume;
            }
            break;
            case 'a':
            {
                ctx->alarm->volume = (int)volume;
                ctx->alarm->max_volume = (int)max_volume;
            }
            break;
            case 'n':
            {
                ctx->notice->volume = (int)volume;
                ctx->notice->max_volume = (int)max_volume;
            }
            break;
            default:
                break;
            }
        }
        ok = 0;
    }
    return ok;
}

int termux_volume_set(termux_volume_s *ctx)
{
    int ok = ~0;
    int argc = 10;
    char buff[16];
    termux_volume_s stats[1];
    char *argv[11] = {0, "Volume", "-a", "set-volume", "--es", "stream", 0, "--ei", "volume", buff, 0};
    ok = termux_volume_get(stats);
    if (ctx->call->volume != stats->call->volume)
    {
        argv[6] = "call";
        snprintf(buff, 16, "%i", ctx->call->volume);
        if (pipe_exec(argc, argv, 1000) == 0 && ctx->call->volume > stats->call->max_volume)
        {
            ctx->call->max_volume = stats->call->max_volume;
            ctx->call->volume = stats->call->max_volume;
        }
    }
    if (ctx->system->volume != stats->system->volume)
    {
        argv[6] = "system";
        snprintf(buff, 16, "%i", ctx->system->volume);
        if (pipe_exec(argc, argv, 1000) == 0 && ctx->system->volume > stats->system->max_volume)
        {
            ctx->system->max_volume = stats->system->max_volume;
            ctx->system->volume = stats->system->max_volume;
        }
    }
    if (ctx->ring->volume != stats->ring->volume)
    {
        argv[6] = "ring";
        snprintf(buff, 16, "%i", ctx->ring->volume);
        if (pipe_exec(argc, argv, 1000) == 0 && ctx->ring->volume > stats->ring->max_volume)
        {
            ctx->ring->max_volume = stats->ring->max_volume;
            ctx->ring->volume = stats->ring->max_volume;
        }
    }
    if (ctx->music->volume != stats->music->volume)
    {
        argv[6] = "music";
        snprintf(buff, 16, "%i", ctx->music->volume);
        if (pipe_exec(argc, argv, 1000) == 0 && ctx->music->volume > stats->music->max_volume)
        {
            ctx->music->max_volume = stats->music->max_volume;
            ctx->music->volume = stats->music->max_volume;
        }
    }
    if (ctx->alarm->volume != stats->alarm->volume)
    {
        argv[6] = "alarm";
        snprintf(buff, 16, "%i", ctx->alarm->volume);
        if (pipe_exec(argc, argv, 1000) == 0 && ctx->alarm->volume > stats->alarm->max_volume)
        {
            ctx->alarm->max_volume = stats->alarm->max_volume;
            ctx->alarm->volume = stats->alarm->max_volume;
        }
    }
    if (ctx->notice->volume != stats->notice->volume)
    {
        argv[6] = "notification";
        snprintf(buff, 16, "%i", ctx->notice->volume);
        if (pipe_exec(argc, argv, 1000) == 0 && ctx->notice->volume > stats->notice->max_volume)
        {
            ctx->notice->max_volume = stats->notice->max_volume;
            ctx->notice->volume = stats->notice->max_volume;
        }
    }
    return ok;
}
