# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.

# Documentation/Howto:
# http://www.debian.org/doc/manuals/debian-reference/ch05.en.html#_the_hostname_resolution

{% set interfaces_config = salt['omv_conf.get']('conf.system.network.interface') %}
{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}
{% set fqdn = dns_config.hostname | lower %}
{% set alias = "" %}
{% if dns_config.domainname | length > 0 %}
{% set fqdn = [dns_config.hostname, dns_config.domainname] | join('.') | lower %}
{% set alias = dns_config.hostname | lower %}
{% endif %}
{% set static_ipv4_interfaces = interfaces_config | selectattr('method', 'equalto', 'static') | sort(attribute='routemetric') %}
{% set static_ipv6_interfaces = interfaces_config | selectattr('method6', 'equalto', 'static') | sort(attribute='routemetric6') %}

configure_hosts_header:
  file.managed:
    - name: "/etc/hosts"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
    - user: root
    - group: root
    - mode: 644

configure_hosts_localhost_ipv4:
  file.append:
    - name: "/etc/hosts"
    - text: |
        127.0.0.1 localhost.localdomain localhost

# For a system with a permanent IP address, that permanent IP address
# should be used instead of 127.0.1.1.
# https://www.debian.org/doc/manuals/debian-reference/ch05.en.html#_the_hostname_resolution
configure_hosts_localhost_fqdn_ipv4:
  file.append:
    - name: "/etc/hosts"
    - text: |
{%- if static_ipv4_interfaces | length == 0 %}
        127.0.1.1 {{ fqdn }} {{ alias }}
{% else %}
        {{ static_ipv4_interfaces | first | attr('address') }} {{ fqdn }} {{ alias }}
{% endif %}

configure_hosts_comment_ipv6:
  file.append:
    - name: "/etc/hosts"
    - text: |
        # The following lines are desirable for IPv6 capable hosts.

configure_hosts_localhost_ipv6:
  file.append:
    - name: "/etc/hosts"
    - text: |
{%- if static_ipv6_interfaces | length == 0 %}
        # Add hostname to ::1 if there is no other IPv6 interface. This is
        # necessary to properly resolve the hostname, otherwise building the Salt
        # grains (core.fqdns and core.ip_fqdn) will take a very long time.
        ::1 localhost ip6-localhost ip6-loopback {{ fqdn }} {{ alias }}
{% else %}
        ::1 localhost ip6-localhost ip6-loopback
        {{ static_ipv6_interfaces | first | attr('address6') }} {{ fqdn }} {{ alias }}
{% endif %}

configure_hosts_various_ipv6:
  file.append:
    - name: "/etc/hosts"
    - text: |
        fe00::0 ip6-localnet
        ff00::0 ip6-mcastprefix
        ff02::1 ip6-allnodes
        ff02::2 ip6-allrouters
        ff02::3 ip6-allhosts
