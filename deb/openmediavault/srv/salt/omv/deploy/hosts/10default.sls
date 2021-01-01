# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2021 Volker Theile
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
{% set fqdn = dns_config.hostname %}
{% set alias = "" %}
{% if dns_config.domainname %}
{% set fqdn = [dns_config.hostname, dns_config.domainname] | join('.') %}
{% set alias = dns_config.hostname %}
{% endif %}

configure_hosts_default_ipv4:
  file.managed:
    - name: "/etc/hosts"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        127.0.0.1 localhost.localdomain localhost
        127.0.1.1 {{ dns_config.hostname }}
    - user: root
    - group: root
    - mode: 644

{% if salt['omv_utils.is_ipv6_enabled']() %}

configure_hosts_default_ipv6:
  file.append:
    - name: "/etc/hosts"
    - text: |
        # The following lines are desirable for IPv6 capable hosts.
        ::1     ip6-localhost ip6-loopback
        fe00::0 ip6-localnet
        ff00::0 ip6-mcastprefix
        ff02::1 ip6-allnodes
        ff02::2 ip6-allrouters
        ff02::3 ip6-allhosts

{% endif %}

{% for interface in interfaces_config %}

{% if interface.address | is_ipv4 %}
append_hosts_{{ interface.devicename }}_v4:
  host.only:
    - name: "{{ interface.address }}"
    - hostnames:
      - "{{ fqdn }}"
      - "{{ alias }}"
{% endif %}

{% if interface.address6 | is_ipv6 %}
append_hosts_{{ interface.devicename }}_v6:
  host.only:
    - name: "{{ interface.address6 }}"
    - hostnames:
      - "{{ fqdn }}"
      - "{{ alias }}"
{% endif %}

{% endfor %}
