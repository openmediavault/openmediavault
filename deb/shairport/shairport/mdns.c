/*
 * mDNS registration handler. This file is part of Shairport.
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


#include <memory.h>
#include <string.h>
#include "config.h"
#include "common.h"
#include "mdns.h"

#ifdef CONFIG_AVAHI
extern mdns_backend mdns_avahi;
#endif

extern mdns_backend mdns_external_avahi;
extern mdns_backend mdns_external_dns_sd;
extern mdns_backend mdns_tinysvcmdns;

#ifdef CONFIG_HAVE_DNS_SD_H
extern mdns_backend mdns_dns_sd;
#endif

static mdns_backend *mdns_backends[] = {
#ifdef CONFIG_AVAHI
    &mdns_avahi,
#endif
#ifdef CONFIG_HAVE_DNS_SD_H
    &mdns_dns_sd,
#endif
    &mdns_external_avahi,
    &mdns_external_dns_sd,
    &mdns_tinysvcmdns,
    NULL
};

void mdns_register(void) {
    char *mdns_apname = malloc(strlen(config.apname) + 14);
    char *p = mdns_apname;
    int i;
    for (i=0; i<6; i++) {
        sprintf(p, "%02X", config.hw_addr[i]);
        p += 2;
    }
    *p++ = '@';
    strcpy(p, config.apname);

    mdns_backend **b = NULL;
    
    if (config.mdns_name != NULL)
    {
        for (b = mdns_backends; *b; b++)
        {
            if (strcmp((*b)->name, config.mdns_name) != 0) // Not the one we are looking for
                continue;
            int error = (*b)->mdns_register(mdns_apname, config.port);
            if (error >= 0)
            {
                config.mdns = *b;
            }
            break;
        }

        if (*b == NULL)
            warn("%s mDNS backend not found");
    }
    else
    {
        for (b = mdns_backends; *b; b++)
        {
            int error = (*b)->mdns_register(mdns_apname, config.port);
            if (error >= 0)
            {
                config.mdns = *b;
                break;
            }
        }
    }

    if (config.mdns == NULL)
        die("Could not establish mDNS advertisement!");
}

void mdns_unregister(void) {
    if (config.mdns) {
        config.mdns->mdns_unregister();
    }
}

void mdns_ls_backends(void) {
    mdns_backend **b = NULL;
    printf("Available mDNS backends: \n");
    for (b = mdns_backends; *b; b++)
    {
        printf("    %s\n", (*b)->name);
    }
}

