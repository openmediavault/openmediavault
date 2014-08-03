#ifndef _AUDIO_H
#define _AUDIO_H

typedef struct {
    void (*help)(void);
    char *name;

    // start of program
    int (*init)(int argc, char **argv);
    // at end of program
    void (*deinit)(void);

    void (*start)(int sample_rate);
    // block of samples
    void (*play)(short buf[], int samples);
    void (*stop)(void);

    // may be NULL, in which case soft volume is applied
    void (*volume)(double vol);
} audio_output;

audio_output *audio_get_output(char *name);
void audio_ls_outputs(void);

#endif //_AUDIO_H
