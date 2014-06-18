/*
 * mDNS registration handler. This file is part of Shairport.
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

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h> 
#include <ifaddrs.h>
#include <unistd.h>
#include <netinet/in.h>
#include "common.h"
#include "mdns.h"

#include "tinysvcmdns.h"

static struct mdnsd *svr = NULL;

static int mdns_tinysvcmdns_register(char *apname, int port) {
    struct ifaddrs *ifalist;
    struct ifaddrs *ifa;

    svr = mdnsd_start();
    if (svr == NULL) {
        warn("tinysvcmdns: mdnsd_start() failed");
        return -1;
    }

    char hostname[100];
    gethostname(hostname, 100);

    if (getifaddrs(&ifalist) < 0)
    {
        warn("tinysvcmdns: getifaddrs() failed");
        return -1;
    }

    ifa = ifalist;

    // Look for an ipv4/ipv6 non-loopback interface to use as the main one.
    for (ifa = ifalist; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!(ifa->ifa_flags & IFF_LOOPBACK) && ifa->ifa_addr &&
            ifa->ifa_addr->sa_family == AF_INET)
        {
            uint32_t main_ip = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;

            mdnsd_set_hostname(svr, hostname, main_ip);
            break;
        }
        else if (!(ifa->ifa_flags & IFF_LOOPBACK) && ifa->ifa_addr &&
                 ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct in6_addr *addr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;

            mdnsd_set_hostname_v6(svr, hostname, addr);
            break;
        }
    }

    if (ifa == NULL)
    {
        warn("tinysvcmdns: no non-loopback ipv4 or ipv6 interface found");
        return -1;
    }


    // Skip the first one, it was already added by set_hostname
    for (ifa = ifa->ifa_next; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_flags & IFF_LOOPBACK) // Skip loop-back interfaces
            continue;

        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET: { // ipv4
                    uint32_t ip = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
                    struct rr_entry *a_e = rr_create_a(create_nlabel(hostname), ip);
                    mdnsd_add_rr(svr, a_e);
                }
                break;
            case AF_INET6: { // ipv6
                    struct in6_addr *addr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
                    struct rr_entry *aaaa_e = rr_create_aaaa(create_nlabel(hostname), addr);
                    mdnsd_add_rr(svr, aaaa_e);
                }
                break;
        }
    }

    freeifaddrs(ifa);

    const char *txt[] = { MDNS_RECORD, NULL };
    struct mdns_service *svc = mdnsd_register_svc(svr,
                                apname,
                                "_raop._tcp.local",
                                port,
                                NULL,
                                txt);

    mdns_service_destroy(svc);

    return 0;
}

static void mdns_tinysvcmdns_unregister(void) {
    if (svr)
    {
        mdnsd_stop(svr);
        svr = NULL;
    }
}

mdns_backend mdns_tinysvcmdns = {
    .name = "tinysvcmdns",
    .mdns_register = mdns_tinysvcmdns_register,
    .mdns_unregister = mdns_tinysvcmdns_unregister
};

