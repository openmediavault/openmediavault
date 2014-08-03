/*
 * Embedded Avahi client. This file is part of Shairport.
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

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/error.h>

#include <string.h>
#include "common.h"
#include "mdns.h"

static AvahiClient *client = NULL;
static AvahiEntryGroup *group = NULL;
static AvahiThreadedPoll *tpoll = NULL;

static char *name = NULL;
static int port = 0;

static void egroup_callback(AvahiEntryGroup *g,
                            AvahiEntryGroupState state,
                            AVAHI_GCC_UNUSED void *userdata) {
    if (state==AVAHI_ENTRY_GROUP_COLLISION)
        die("service name already exists on network!");
    if (state==AVAHI_ENTRY_GROUP_FAILURE)
        die("avahi entry group failure!");
}

static void register_service(AvahiClient *c) {
    debug(1, "avahi: register_service\n");
    if (!group)
        group = avahi_entry_group_new(c, egroup_callback, NULL);
    if (!group)
        die("avahi_entry_group_new failed");

    if (!avahi_entry_group_is_empty(group))
        return;

    int ret;
    ret = avahi_entry_group_add_service(group,
                                        AVAHI_IF_UNSPEC,
                                        AVAHI_PROTO_UNSPEC,
                                        0,
                                        name,
                                        "_raop._tcp",
                                        NULL,
                                        NULL,
                                        port,
                                        MDNS_RECORD,
                                        NULL);
    if (ret < 0)
        die("avahi_entry_group_add_service failed");

    ret = avahi_entry_group_commit(group);
    if (ret < 0)
        die("avahi_entry_group_commit failed");
}

static void client_callback(AvahiClient *c,
                            AvahiClientState state,
                            AVAHI_GCC_UNUSED void * userdata) {
    switch (state) {
        case AVAHI_CLIENT_S_REGISTERING:
            if (group)
                avahi_entry_group_reset(group);
            break;

        case AVAHI_CLIENT_S_RUNNING:
            register_service(c);
            break;

        case AVAHI_CLIENT_FAILURE:
        case AVAHI_CLIENT_S_COLLISION:
            die("avahi client failure");

        case AVAHI_CLIENT_CONNECTING:
            break;
    }
}

static int avahi_register(char *srvname, int srvport) {
    debug(1, "avahi: avahi_register\n");
    name = strdup(srvname);
    port = srvport;

    int err;
    if (!(tpoll = avahi_threaded_poll_new())) {
        warn("couldn't create avahi threaded tpoll!");
        return -1;
    }
    if (!(client = avahi_client_new(avahi_threaded_poll_get(tpoll), 
                                    0,
                                    client_callback,
                                    NULL,
                                    &err))) {
        warn("couldn't create avahi client: %s!", avahi_strerror(err));
        return -1;
    }

    if (avahi_threaded_poll_start(tpoll) < 0) {
        warn("couldn't start avahi tpoll thread");
        return -1;
    }

    return 0;
}

static void avahi_unregister(void) {
    debug(1, "avahi: avahi_unregister\n");
    if (tpoll)
        avahi_threaded_poll_stop(tpoll);
    tpoll = NULL;

    if (name)
        free(name);
    name = NULL;
}

mdns_backend mdns_avahi = 
{
    .name = "avahi",
    .mdns_register = avahi_register,
    .mdns_unregister = avahi_unregister
};

