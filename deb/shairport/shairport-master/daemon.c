/*
 * This file is part of Shairport.
 * Copyright (c) Paul Lietar 2013
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "common.h"

static int lock_fd = -1;
static int daemon_pipe[2] = {-1, -1};

void daemon_init() {
    int ret;
    ret = pipe(daemon_pipe);
    if (ret < 0)
        die("couldn't create a pipe?!");

    pid_t pid = fork();
    if (pid < 0)
        die("failed to fork!");

    if (pid) {
        close(daemon_pipe[1]);

        char buf[64];
        ret = read(daemon_pipe[0], buf, sizeof(buf));
        if (ret < 0) {
            // No response from child, something failed
            fprintf(stderr, "Spawning the daemon failed.\n");
            exit(1);
        } else if (buf[0] != 0) {
            // First byte is non zero, child sent error message
            write(STDERR_FILENO, buf, ret);
            fprintf(stderr, "\n");
            exit(1);
        } else {
            // Success !
            if (!config.pidfile)
                printf("%d\n", pid);
            exit(0);
        }
    } else {
        close(daemon_pipe[0]);

        if (config.pidfile) {
            lock_fd = open(config.pidfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            if (lock_fd < 0) {
                die("Could not open pidfile");
            }

            ret = lockf(lock_fd,F_TLOCK,0);
            if (ret < 0) {
                die("Could not lock pidfile. Is an other instance running ?");
            }

            dprintf(lock_fd, "%d\n", getpid());
        }
    }
}

void daemon_ready() {
    char ok = 0;
    write(daemon_pipe[1], &ok, 1);
    close(daemon_pipe[1]);
    daemon_pipe[1] = -1;
}

void daemon_fail(const char *format, va_list arg) {
    // Are we still initializing ?
    if (daemon_pipe[1] > 0) {
        vdprintf(daemon_pipe[1], format, arg);
    }
}

void daemon_exit() {
    if (lock_fd > 0) {
        lockf(lock_fd, F_ULOCK, 0);
        close(lock_fd);
        unlink(config.pidfile);
        lock_fd = -1;
    }
}

