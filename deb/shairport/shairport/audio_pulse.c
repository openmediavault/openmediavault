/*
 * PulseAudio output driver. This file is part of Shairport.
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


#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "common.h"
#include "audio.h"

static pa_simple *pa_dev = NULL;
static int pa_error;

static void help(void) {
    printf("    -a server           set the server name\n"
           "    -s sink             set the output sink\n"
           "    -n name             set the application name, as seen by PulseAudio\n"
           "                            defaults to the access point name\n"
          );
}

static int init(int argc, char **argv) {
    char *pa_server = NULL;
    char *pa_sink = NULL;
    char *pa_appname = config.apname;

    optind = 1; // optind=0 is equivalent to optind=1 plus special behaviour
    argv--;     // so we shift the arguments to satisfy getopt()
    argc++;

    // some platforms apparently require optreset = 1; - which?
    int opt;
    while ((opt = getopt(argc, argv, "a:s:n:")) > 0) {
        switch (opt) {
            case 'a':
                pa_server = optarg;
                break;
            case 's':
                pa_sink = optarg;
                break;
            case 'n':
                pa_appname = optarg;
                break;
            default:
                help();
                die("Invalid audio option -%c specified", opt);
        }
    }

    if (optind < argc)
        die("Invalid audio argument: %s", argv[optind]);

    static const pa_sample_spec ss = {
            .format = PA_SAMPLE_S16LE,
            .rate = 44100,
            .channels = 2
    };

    pa_dev = pa_simple_new(pa_server,
            pa_appname,
            PA_STREAM_PLAYBACK,
            pa_sink,
            "Shairport Stream",
            &ss, NULL, NULL,
            &pa_error);

    if (!pa_dev)
        die("Could not connect to pulseaudio server: %s", pa_strerror(pa_error));

    return 0;
}

static void deinit(void) {
    if (pa_dev)
        pa_simple_free(pa_dev);
    pa_dev = NULL;
}

static void start(int sample_rate) {
    if (sample_rate != 44100)
        die("unexpected sample rate!");
}

static void play(short buf[], int samples) {
    if( pa_simple_write(pa_dev, (char *)buf, (size_t)samples * 4, &pa_error) < 0 )
        fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(pa_error));
}

static void stop(void) {
    if (pa_simple_drain(pa_dev, &pa_error) < 0)
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(pa_error));
}

audio_output audio_pulse = {
    .name = "pulse",
    .help = &help,
    .init = &init,
    .deinit = &deinit,
    .start = &start,
    .stop = &stop,
    .play = &play,
    .volume = NULL
};
