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
# http://www.postfix.org/TLS_README.html
# https://en.wikipedia.org/wiki/SMTPS
# http://blog.mailgun.com/25-465-587-what-port-should-i-use/

{% set config = salt['omv_conf.get']('conf.system.notification.email') %}
{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}

configure_postfix_main:
  file.managed:
    - name: "/etc/postfix/main.cf"
    - source:
      - salt://{{ tpldir }}/files/main.cf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
        dns_config: {{ dns_config | json }}
    - user: root
    - group: root
    - mode: 644
