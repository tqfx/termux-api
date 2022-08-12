/*!
 @file pipe.c
 @brief pipeline implementation
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#include "pipe.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>

int pipe_flush(const pipe_s *ctx)
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
    if (ctx->er)
    {
        ok += fflush(ctx->er);
    }
    return ok;
}

int pipe_getc(const pipe_s *ctx)
{
    return fgetc(ctx->rd);
}

int pipe_gete(const pipe_s *ctx)
{
    return fgetc(ctx->er);
}

int pipe_putc(const pipe_s *ctx, int c)
{
    return fputc(c, ctx->wr);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif /* __GNUC__ || __clang__ */

int pipe_scanf(const pipe_s *ctx, const char *fmt, ...)
{
    int stats;
    va_list va;
    va_start(va, fmt);
    stats = vfscanf(ctx->rd, fmt, va);
    va_end(va);
    return stats;
}

int pipe_scanfe(const pipe_s *ctx, const char *fmt, ...)
{
    int stats;
    va_list va;
    va_start(va, fmt);
    stats = vfscanf(ctx->er, fmt, va);
    va_end(va);
    return stats;
}

int pipe_printf(const pipe_s *ctx, const char *fmt, ...)
{
    int stats;
    va_list va;
    va_start(va, fmt);
    stats = vfprintf(ctx->wr, fmt, va);
    va_end(va);
    return stats;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif /* __GNUC__ || __clang__ */

size_t pipe_read(const pipe_s *ctx, void *data, size_t byte)
{
    return fread(data, 1, byte, ctx->rd);
}

size_t pipe_reade(const pipe_s *ctx, void *data, size_t byte)
{
    return fread(data, 1, byte, ctx->er);
}

size_t pipe_write(const pipe_s *ctx, const void *data, size_t byte)
{
    return fwrite(data, 1, byte, ctx->wr);
}

#include <unistd.h>
#include <sys/wait.h>

#define R 0
#define W 1

int pipe_open(pipe_s *ctx, const char *path, char *const argv[], char *const envp[])
{
    ctx->wr = 0;
    ctx->rd = 0;
    ctx->er = 0;
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

        extern char **environ;
        execve(path, argv, envp ? envp : environ);

        _exit(127); /* command not found */
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

int pipe_open3(pipe_s *ctx, const char *path, char *const argv[], char *const envp[])
{
    ctx->wr = 0;
    ctx->rd = 0;
    ctx->er = 0;
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
    int pipe_er[2];
    if (pipe(pipe_er) < 0)
    {
        goto pipe_er;
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
        close(pipe_er[R]);

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
        if (pipe_er[W] != STDERR_FILENO)
        {
            int ok = dup2(pipe_er[W], STDERR_FILENO);
            close(pipe_er[W]);
            if (ok < 0)
            {
                _exit(EXIT_FAILURE);
            }
        }

        extern char **environ;
        execve(path, argv, envp ? envp : environ);

        _exit(127); /* command not found */
    }

    close(pipe_wr[R]);
    close(pipe_rd[W]);
    close(pipe_er[W]);

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
    ctx->er = fdopen(pipe_er[R], "r");
    if (ctx->rd == 0)
    {
        goto open_er;
    }

    return 0;

open_er:
    close(pipe_er[R]);
open_rd:
    close(pipe_rd[R]);
open_wr:
    close(pipe_wr[W]);

    return ~0;

pipe_rw:
    close(pipe_er[R]);
    close(pipe_er[W]);
pipe_er:
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

int pipe_close(pipe_s *ctx)
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
    if (ctx->er && fclose(ctx->er) == EOF)
    {
        clearerr(ctx->er);
    }
    ctx->er = 0;

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

int pipe_wait(const pipe_s *ctx, unsigned long ms)
{
    errno = 0;
    int status = 0;
    pid_t wait = waitpid(ctx->pid, &status, WNOHANG);
    if (wait != 0 || ms == 0)
    {
        /* wait for the child process to terminate */
        waitpid(ctx->pid, &status, 0);
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
