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
# http://www.debian.org/doc/manuals/debian-reference/ch03.en.html#_the_hostname

{% set config = salt['omv_conf.get']('conf.system.network.dns') %}
{% set fqdn = [config.hostname, config.domainname] | reject('equalto', '') | join('.') %}

configure_hostname:
  cmd.run:
    - name: hostnamectl set-hostname "{{ fqdn }}"
    - unless: test "{{ fqdn }}" = "$(hostname)"
