/*!
 @file pipe.h
 @brief pipeline implementation
 @copyright Copyright (C) 2020-present tqfx, All rights reserved.
*/

#ifndef __UNIX_PIPE_H__
#define __UNIX_PIPE_H__

#include <stdio.h>
#include <sys/types.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#endif /* __GNUC__ || __clang__ */

/*!
 @brief instance structure for pipeline
*/
typedef struct pipe_s
{
    FILE *wr;
    FILE *rd;
    FILE *er;
    pid_t pid;
} pipe_s;

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif /* __GNUC__ || __clang__ */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 @brief initialize an instance of pipeline structure
 @param[in] ctx points to an instance of pipeline structure
 @param[in] path the filename associated with the file being executed
 @code{.c}
 const char *path = "/bin/ls";
 @endcode
 @param[in] argv a pointer to the argument block terminated by a NULL pointer
 @code{.c}
 char *argv[] = {"ls", "-hlA", NULL};
 @endcode
 @param[in] envp a pointer to the environment block terminated by a NULL pointer
 @code{.c}
 char *envp[] = {"PATH=/bin", NULL};
 @endcode
 @return the execution state of the function
  @retval ~0 failure
  @retval 0 success
*/
int pipe_open(pipe_s *ctx, const char *path, char *const argv[], char *const envp[]) __attribute__((visibility("default")));
int pipe_open3(pipe_s *ctx, const char *path, char *const argv[], char *const envp[]) __attribute__((visibility("default")));

/*!
 @brief terminate an instance of pipeline structure
 @param[in] ctx points to an instance of pipeline structure
 @return the execution state of the function
  @retval ~0 failure
  @retval 0 success
*/
int pipe_close(pipe_s *ctx) __attribute__((visibility("default")));

/*!
 @brief pending within the timeout period specified synchronously
 @param[in] ctx points to an instance of pipeline structure
 @param[in] ms timeout period millisecond specified
 @return the execution state of the function
  @retval ~0 failure
  @retval 0 success
*/
int pipe_wait(const pipe_s *ctx, unsigned long ms) __attribute__((visibility("default")));

int pipe_flush(const pipe_s *ctx) __attribute__((visibility("default")));

int pipe_getc(const pipe_s *ctx) __attribute__((visibility("default")));

int pipe_gete(const pipe_s *ctx) __attribute__((visibility("default")));

int pipe_putc(const pipe_s *ctx, int c) __attribute__((visibility("default")));

int pipe_scanf(const pipe_s *ctx, const char *fmt, ...) __attribute__((visibility("default")));

int pipe_scanfe(const pipe_s *ctx, const char *fmt, ...) __attribute__((visibility("default")));

int pipe_printf(const pipe_s *ctx, const char *fmt, ...) __attribute__((visibility("default")));

size_t pipe_read(const pipe_s *ctx, void *data, size_t byte) __attribute__((visibility("default")));

size_t pipe_reade(const pipe_s *ctx, void *data, size_t byte) __attribute__((visibility("default")));

size_t pipe_write(const pipe_s *ctx, const void *data, size_t byte) __attribute__((visibility("default")));

#if defined(__cplusplus)
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __UNIX_PIPE_H__ */
