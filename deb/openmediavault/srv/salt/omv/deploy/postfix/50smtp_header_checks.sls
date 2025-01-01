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

{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}
{% set fqdn = dns_config.hostname %}
{% if dns_config.domainname | length > 0 %}
{% set fqdn = [dns_config.hostname, dns_config.domainname] | join('.') %}
{% endif %}

configure_postfix_smtp_header_checks:
  file.managed:
    - name: "/etc/postfix/smtp_header_checks"
    - contents: |
        # Append the hostname to the email subject.
        /^Subject: (.*)/ REPLACE Subject: [{{ fqdn }}] ${1}
    - user: root
    - group: root
    - mode: 600
