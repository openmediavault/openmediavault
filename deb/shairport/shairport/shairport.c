/*
 * Shairport, an Apple Airplay receiver
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

#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <openssl/md5.h>
#include <sys/wait.h>
#include <getopt.h>
#include "common.h"
#include "daemon.h"
#include "rtsp.h"
#include "mdns.h"
#include "getopt_long.h"
#include "metadata.h"

static const char *version =
    #include "version.h"
    ;

static void log_setup();

static int shutting_down = 0;

void shairport_shutdown(int retval) {
    if (shutting_down)
        return;
    shutting_down = 1;
    printf("Shutting down...\n");
    mdns_unregister();
    rtsp_shutdown_stream();
    if (config.output)
        config.output->deinit();
    daemon_exit(); // This does nothing if not in daemon mode

    exit(retval);
}

static void sig_ignore(int foo, siginfo_t *bar, void *baz) {
}
static void sig_shutdown(int foo, siginfo_t *bar, void *baz) {
    shairport_shutdown(0);
}

static void sig_child(int foo, siginfo_t *bar, void *baz) {
    pid_t pid;
    while ((pid = waitpid((pid_t)-1, 0, WNOHANG)) > 0) {
        if (pid == mdns_pid && !shutting_down) {
            die("MDNS child process died unexpectedly!");
        }
    }
}

static void sig_logrotate(int foo, siginfo_t *bar, void *baz) {
    log_setup();
}

void usage(char *progname) {
    printf("Usage: %s [options...]\n", progname);
    printf("  or:  %s [options...] -- [audio output-specific options]\n", progname);

    printf("\n");
    printf("Mandatory arguments to long options are mandatory for short options too.\n");

    printf("\n");
    printf("Options:\n");
    printf("    -h, --help          show this help\n");
    printf("    -p, --port=PORT     set RTSP listening port\n");
    printf("    -a, --name=NAME     set advertised name\n");
    printf("    -k, --password=PW   require password to stream audio\n");
    printf("    -b FILL             set how full the buffer must be before audio output\n");
    printf("                        starts. This value is in frames; default %d\n", config.buffer_start_fill);
    printf("    -d, --daemon        fork (daemonise). The PID of the child process is\n");
    printf("                        written to stdout, unless a pidfile is used.\n");
    printf("    -P, --pidfile=FILE  write daemon's pid to FILE on startup.\n");
    printf("                        Has no effect if -d is not specified\n");
    printf("    -l, --log=FILE      redirect shairport's standard output to FILE\n");
    printf("                        If --error is not specified, it also redirects\n");
    printf("                        error output to FILE\n");
    printf("    -e, --error=FILE    redirect shairport's standard error output to FILE\n");
    printf("    -B, --on-start=COMMAND  run a shell command when playback begins\n");
    printf("    -E, --on-stop=COMMAND   run a shell command when playback ends\n");
    printf("    -w, --wait-cmd          block while the shell command(s) run\n");
    printf("    -M, --meta-dir=DIR      set a directory to write metadata and album cover art to\n");

    printf("    -o, --output=BACKEND    select audio output method\n");
    printf("    -m, --mdns=BACKEND      force the use of BACKEND to advertise the service\n");
    printf("                            if no mdns provider is specified,\n");
    printf("                            shairport tries them all until one works.\n");

    printf("\n");
    mdns_ls_backends();
    printf("\n");
    audio_ls_outputs();
}

int parse_options(int argc, char **argv) {
    // prevent unrecognised arguments from being shunted to the audio driver
    setenv("POSIXLY_CORRECT", "", 1);

    static struct option long_options[] = {
        {"help",      no_argument,        NULL, 'h'},
        {"daemon",    no_argument,        NULL, 'd'},
        {"pidfile",   required_argument,  NULL, 'P'},
        {"log",       required_argument,  NULL, 'l'},
        {"error",     required_argument,  NULL, 'e'},
        {"port",      required_argument,  NULL, 'p'},
        {"name",      required_argument,  NULL, 'a'},
        {"password",  required_argument,  NULL, 'k'},
        {"output",    required_argument,  NULL, 'o'},
        {"on-start",  required_argument,  NULL, 'B'},
        {"on-stop",   required_argument,  NULL, 'E'},
        {"wait-cmd",  no_argument,        NULL, 'w'},
        {"meta-dir",  required_argument,  NULL, 'M'},
        {"mdns",      required_argument,  NULL, 'm'},
        {NULL,        0,                  NULL,   0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv,
                              "+hdvP:l:e:p:a:k:o:b:B:E:M:wm:",
                              long_options, NULL)) > 0) {
        switch (opt) {
            default:
                usage(argv[0]);
                exit(1);
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'd':
                config.daemonise = 1;
                break;
            case 'v':
                debuglev++;
                break;
            case 'p':
                config.port = atoi(optarg);
                break;
            case 'a':
                config.apname = optarg;
                break;
            case 'o':
                config.output_name = optarg;
                break;
            case 'k':
                config.password = optarg;
                break;
            case 'b':
                config.buffer_start_fill = atoi(optarg);
                break;
            case 'B':
                config.cmd_start = optarg;
                break;
            case 'E':
                config.cmd_stop = optarg;
                break;
            case 'w':
                config.cmd_blocking = 1;
                break;
            case 'M':
                config.meta_dir = optarg;
                break;
            case 'P':
                config.pidfile = optarg;
                break;
            case 'l':
                config.logfile = optarg;
                break;
            case 'e':
                config.errfile = optarg;
                break;
            case 'm':
                config.mdns_name = optarg;
                break;
        }
    }
    return optind;
}

void signal_setup(void) {
    // mask off all signals before creating threads.
    // this way we control which thread gets which signals.
    // for now, we don't care which thread gets the following.
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGTERM);
    sigdelset(&set, SIGHUP);
    sigdelset(&set, SIGSTOP);
    sigdelset(&set, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // setting this to SIG_IGN would prevent signalling any threads.
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &sig_ignore;
    sigaction(SIGUSR1, &sa, NULL);

    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = &sig_shutdown;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_sigaction = &sig_logrotate;
    sigaction(SIGHUP, &sa, NULL);

    sa.sa_sigaction = &sig_child;
    sigaction(SIGCHLD, &sa, NULL);
}

// forked daemon lets the spawner know it's up and running OK
// should be called only once!
void shairport_startup_complete(void) {
    if (config.daemonise) {
        daemon_ready();
    }
}

void log_setup() {
    if (config.logfile) {
        int log_fd = open(config.logfile,
                O_WRONLY | O_CREAT | O_APPEND,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (log_fd < 0)
            die("Could not open logfile");

        dup2(log_fd, STDOUT_FILENO);
        setvbuf (stdout, NULL, _IOLBF, BUFSIZ);

        if (!config.errfile) {
            dup2(log_fd, STDERR_FILENO);
            setvbuf (stderr, NULL, _IOLBF, BUFSIZ);
        }
    }

    if (config.errfile) {
        int err_fd = open(config.errfile,
                O_WRONLY | O_CREAT | O_APPEND,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (err_fd < 0)
            die("Could not open logfile");

        dup2(err_fd, STDERR_FILENO);
        setvbuf (stderr, NULL, _IOLBF, BUFSIZ);
    }
}

int main(int argc, char **argv) {
    printf("Starting Shairport %s\n", version);

    signal_setup();
    memset(&config, 0, sizeof(config));

    // set defaults
    config.buffer_start_fill = 220;
    config.port = 5002;
    char hostname[100];
    gethostname(hostname, 100);
    config.apname = malloc(20 + 100);
    snprintf(config.apname, 20 + 100, "Shairport on %s", hostname);

    // parse arguments into config
    int audio_arg = parse_options(argc, argv);

    // mDNS supports maximum of 63-character names (we append 13).
    if (strlen(config.apname) > 50)
        die("Supplied name too long (max 50 characters)");

    if (config.daemonise) {
        daemon_init();
    }

    log_setup();

    config.output = audio_get_output(config.output_name);
    if (!config.output) {
        audio_ls_outputs();
        die("Invalid audio output specified!");
    }
    config.output->init(argc-audio_arg, argv+audio_arg);

    uint8_t ap_md5[16];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, config.apname, strlen(config.apname));
    MD5_Final(ap_md5, &ctx);
    memcpy(config.hw_addr, ap_md5, sizeof(config.hw_addr));

    if (config.meta_dir)
        metadata_open();

    rtsp_listen_loop();

    // should not.
    shairport_shutdown(1);
    return 1;
}
