ifeq ($(wildcard config.mk),)
$(warning config.mk does not exist, configuring.)
config.mk:
	sh ./configure
	$(MAKE) shairport
endif

CFLAGS ?= -O2 -Wall
-include config.mk

PREFIX ?= /usr/local

SRCS := shairport.c daemon.c rtsp.c mdns.c mdns_external.c mdns_tinysvcmdns.c common.c rtp.c metadata.c player.c alac.c audio.c audio_dummy.c audio_pipe.c tinysvcmdns.c
DEPS := config.mk alac.h audio.h common.h daemon.h getopt_long.h mdns.h metadata.h player.h rtp.h rtsp.h tinysvcmdns.h

ifdef CONFIG_SNDIO
SRCS += audio_sndio.c
endif

ifdef CONFIG_AO
SRCS += audio_ao.c
endif

ifdef CONFIG_PULSE
SRCS += audio_pulse.c
endif

ifdef CONFIG_ALSA
SRCS += audio_alsa.c
endif

ifdef CONFIG_AVAHI
SRCS += mdns_avahi.c
endif

ifdef CONFIG_HAVE_DNS_SD_H
SRCS += mdns_dns_sd.c
endif

ifndef CONFIG_HAVE_GETOPT_H
SRCS += getopt_long.c
endif

# default target
all: shairport

install: shairport
	install -m 755 -d $(PREFIX)/bin
	install -m 755 shairport $(PREFIX)/bin/shairport

GITREV=$(shell git describe --always)
DIRTY:=$(shell if ! git diff --quiet --exit-code; then echo -dirty; fi)
VERSION=\"$(GITREV)$(DIRTY)\"
__version_file:
	@if [ ! -f version.h -o "`cat .version 2>/dev/null`" != '$(VERSION)' ]; then \
		echo $(VERSION) > version.h; \
	fi

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) $<

shairport.o: __version_file

OBJS := $(SRCS:.c=.o)
shairport: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o shairport

clean:
	rm -f shairport version.h
	rm -f $(OBJS)
