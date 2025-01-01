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
# http://linuxwiki.de/OpenSSH
# https://help.ubuntu.com/community/SSH/OpenSSH/Configuring

{% set config = salt['omv_conf.get']('conf.service.ssh') %}

configure_sshd_config:
  file.managed:
    - name: "/etc/ssh/sshd_config"
    - source:
      - salt://{{ tpldir }}/files/sshd_config.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

divert_sshd_config:
  omv_dpkg.divert_add:
    - name: "/etc/ssh/sshd_config"

# Normally, the /run/sshd directory is created by the systemd unitfile
# ssh.service via the "RuntimeDirectory" option, but since the
# configuration file is tested before the service is started, the
# directory does not yet exist. To prevent the error "Missing privilege
# separation directory: /run/sshd", the directory is created if it does
# not exist.
create_sshd_runtime_dir:
  file.directory:
    - name: /run/sshd
    - mode: 0755

test_sshd_config:
  cmd.run:
    - name: "sshd -t"
