/*
 * pipe output driver. This file is part of Shairport.
 * Copyright (c) James Laird 2013
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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "common.h"
#include "audio.h"

static int fd = -1;
static char *pipename = NULL;
static int Fs;
static long long starttime, samples_played;

static void stop(void) {
    close(fd);
    fd = -1;
}

static void start(int sample_rate) {
    if (fd >= 0)
        stop();

    fd = open(pipename, O_WRONLY | O_NONBLOCK);
    if ((fd < 0) && (errno != ENXIO)) {
        perror("open");
        die("could not open specified pipe for writing");
    }

    // The other end is ready, reopen with blocking
    if (fd >= 0) {
        close(fd);
        fd = open(pipename, O_WRONLY);
    }

    Fs = sample_rate;
    starttime = 0;
    samples_played = 0;
}

// Wait procedure taken from audio_dummy.c
static void wait_samples(int samples) {
    struct timeval tv;

    // this is all a bit expensive but it's long-term stable.
    gettimeofday(&tv, NULL);

    long long nowtime = tv.tv_usec + 1e6*tv.tv_sec;

    if (!starttime)
        starttime = nowtime;

    samples_played += samples;

    long long finishtime = starttime + samples_played * 1e6 / Fs;

    usleep(finishtime - nowtime);
}

static void play(short buf[], int samples) {
    if (fd < 0) {
        wait_samples(samples);

        // check if the other end is ready every 5 seconds
        if (samples_played > 5 * Fs)
            start(Fs);

        return;
    }

    if (write(fd, buf, samples*4) < 0)
        stop();
}

static int init(int argc, char **argv) {
    struct stat sb;

    if (argc != 1)
        die("bad argument(s) to pipe");

    pipename = strdup(argv[0]);

    if (stat(pipename, &sb) < 0)
        die("could not stat() pipe");

    if (!S_ISFIFO(sb.st_mode))
        die("not a pipe");

    return 0;
}

static void deinit(void) {
    if (fd >= 0)
        stop();
    if (pipename)
        free(pipename);
}

static void help(void) {
    printf("    pipe takes 1 argument: the name of the FIFO to write to.\n");
}

audio_output audio_pipe = {
    .name = "pipe",
    .help = &help,
    .init = &init,
    .deinit = &deinit,
    .start = &start,
    .stop = &stop,
    .play = &play,
    .volume = NULL
};
