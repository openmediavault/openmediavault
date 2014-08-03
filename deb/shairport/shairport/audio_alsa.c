/*
 * libalsa output driver. This file is part of Shairport.
 * Copyright (c) Muffinman, Skaman 2013
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

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <alsa/asoundlib.h>
#include "common.h"
#include "audio.h"

static void help(void);
static int init(int argc, char **argv);
static void deinit(void);
static void start(int sample_rate);
static void play(short buf[], int samples);
static void stop(void);
static void volume(double vol);

audio_output audio_alsa = {
    .name = "alsa",
    .help = &help,
    .init = &init,
    .deinit = &deinit,
    .start = &start,
    .stop = &stop,
    .play = &play,
    .volume = NULL
};

static snd_pcm_t *alsa_handle = NULL;
static snd_pcm_hw_params_t *alsa_params = NULL;

static snd_mixer_t *alsa_mix_handle = NULL;
static snd_mixer_elem_t *alsa_mix_elem = NULL;
static snd_mixer_selem_id_t *alsa_mix_sid = NULL;
static long alsa_mix_minv, alsa_mix_range;

static char *alsa_out_dev = "default";
static char *alsa_mix_dev = NULL;
static char *alsa_mix_ctrl = "Master";
static int alsa_mix_index = 0;

static void help(void) {
    printf("    -d output-device    set the output device [default*|...]\n"
           "    -t mixer-type       set the mixer type [software*|hardware]\n"
           "    -m mixer-device     set the mixer device ['output-device'*|...]\n"
           "    -c mixer-control    set the mixer control [Master*|...]\n"
           "    -i mixer-index      set the mixer index [0*|...]\n"
           "    *) default option\n"
          );
}

static int init(int argc, char **argv) {
    int hardware_mixer = 0;

    optind = 1; // optind=0 is equivalent to optind=1 plus special behaviour
    argv--;     // so we shift the arguments to satisfy getopt()
    argc++;
    // some platforms apparently require optreset = 1; - which?
    int opt;
    while ((opt = getopt(argc, argv, "d:t:m:c:i:")) > 0) {
        switch (opt) {
            case 'd':
                alsa_out_dev = optarg;
                break;
            case 't':
                if (strcmp(optarg, "hardware") == 0)
                    hardware_mixer = 1;
                break;
            case 'm':
                alsa_mix_dev = optarg;
                break;
            case 'c':
                alsa_mix_ctrl = optarg;
                break;
            case 'i':
                alsa_mix_index = strtol(optarg, NULL, 10);
                break;
            default:
                help();
                die("Invalid audio option -%c specified", opt);
        }
    }

    if (optind < argc)
        die("Invalid audio argument: %s", argv[optind]);

    if (!hardware_mixer)
        return 0;

    if (alsa_mix_dev == NULL)
        alsa_mix_dev = alsa_out_dev;
    audio_alsa.volume = &volume;

    int ret = 0;
    long alsa_mix_maxv;

    snd_mixer_selem_id_alloca(&alsa_mix_sid);
    snd_mixer_selem_id_set_index(alsa_mix_sid, alsa_mix_index);
    snd_mixer_selem_id_set_name(alsa_mix_sid, alsa_mix_ctrl);

    if ((snd_mixer_open(&alsa_mix_handle, 0)) < 0)
        die ("Failed to open mixer");
    if ((snd_mixer_attach(alsa_mix_handle, alsa_mix_dev)) < 0)
        die ("Failed to attach mixer");
    if ((snd_mixer_selem_register(alsa_mix_handle, NULL, NULL)) < 0)
        die ("Failed to register mixer element");

    ret = snd_mixer_load(alsa_mix_handle);
    if (ret < 0)
        die ("Failed to load mixer element");
    alsa_mix_elem = snd_mixer_find_selem(alsa_mix_handle, alsa_mix_sid);
    if (!alsa_mix_elem)
        die ("Failed to find mixer element");
    snd_mixer_selem_get_playback_volume_range (alsa_mix_elem, &alsa_mix_minv, &alsa_mix_maxv);
    alsa_mix_range = alsa_mix_maxv - alsa_mix_minv;

    return 0;
}

static void deinit(void) {
    stop();
    if (alsa_mix_handle) {
        snd_mixer_close(alsa_mix_handle);
    }
}

static void start(int sample_rate) {
    if (sample_rate != 44100)
        die("Unexpected sample rate!");

    int ret, dir = 0;
    snd_pcm_uframes_t frames = 64;
    ret = snd_pcm_open(&alsa_handle, alsa_out_dev, SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0)
        die("Alsa initialization failed: unable to open pcm device: %s\n", snd_strerror(ret));

    snd_pcm_hw_params_alloca(&alsa_params);
    snd_pcm_hw_params_any(alsa_handle, alsa_params);
    snd_pcm_hw_params_set_access(alsa_handle, alsa_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(alsa_handle, alsa_params, SND_PCM_FORMAT_S16);
    snd_pcm_hw_params_set_channels(alsa_handle, alsa_params, 2);
    snd_pcm_hw_params_set_rate_near(alsa_handle, alsa_params, (unsigned int *)&sample_rate, &dir);
    snd_pcm_hw_params_set_period_size_near(alsa_handle, alsa_params, &frames, &dir);
    ret = snd_pcm_hw_params(alsa_handle, alsa_params);
    if (ret < 0)
        die("unable to set hw parameters: %s\n", snd_strerror(ret));
}

static void play(short buf[], int samples) {
    int err = snd_pcm_writei(alsa_handle, (char*)buf, samples);
    if (err < 0)
        err = snd_pcm_recover(alsa_handle, err, 0);
    if (err < 0)
        die("Failed to write to PCM device: %s\n", snd_strerror(err));
}

static void stop(void) {
    if (alsa_handle) {
        snd_pcm_drain(alsa_handle);
        snd_pcm_close(alsa_handle);
        alsa_handle = NULL;
    }
}

static void volume(double vol) {
    long alsa_volume = (vol*alsa_mix_range)+alsa_mix_minv;
    if(snd_mixer_selem_set_playback_volume_all(alsa_mix_elem, alsa_volume) != 0)
        die ("Failed to set playback volume");
}
