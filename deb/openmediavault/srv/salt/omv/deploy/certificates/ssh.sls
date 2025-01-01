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

{% set certificates = salt['omv_conf.get']('conf.system.certificate.ssh') %}
{% set ssh_key_prefix = salt['pillar.get']('default:OMV_SSH_KEY_PREFIX', 'openmediavault-') %}

remove_ssh_certificates:
  module.run:
    - file.find:
      - path: "/etc/ssh"
      - iname: "{{ ssh_key_prefix }}*"
      - delete: "f"

{% for certificate in certificates %}

create_ssh_{{ certificate.uuid }}_priv:
  file.managed:
    - name: "/etc/ssh/{{ ssh_key_prefix }}{{ certificate.uuid }}"
    - contents: |
        {{ certificate.privatekey | indent(8) }}
    - user: root
    - group: root
    - mode: 600

create_ssh_{{ certificate.uuid }}_pub:
  file.managed:
    - name: "/etc/ssh/{{ ssh_key_prefix }}{{ certificate.uuid }}.pub"
    - contents: |
        {{ certificate.publickey | indent(8) }}
    - user: root
    - group: root
    - mode: 644

{% endfor %}
