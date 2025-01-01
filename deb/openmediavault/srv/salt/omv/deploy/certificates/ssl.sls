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

{% set certificates_config = salt['omv_conf.get']('conf.system.certificate.ssl') %}
{% set certificate_prefix = salt['pillar.get']('default:OMV_SSL_CERTIFICATE_PREFIX', 'openmediavault-') %}

remove_ssl_certificates_crt:
  module.run:
    - file.find:
      - path: "/etc/ssl/certs"
      - iname: "{{ certificate_prefix }}*.crt"
      - delete: "f"

remove_ssl_certificates_key:
  module.run:
    - file.find:
      - path: "/etc/ssl/private"
      - iname: "{{ certificate_prefix }}*.key"
      - delete: "f"

{% for certificate in certificates_config %}

create_ssl_{{ certificate.uuid }}_crt:
  file.managed:
    - name: "/etc/ssl/certs/{{ certificate_prefix }}{{ certificate.uuid }}.crt"
    - contents: |
        {{ certificate.certificate | indent(8) }}
    - user: root
    - group: root
    - mode: 644

create_ssl_{{ certificate.uuid }}_key:
  file.managed:
    - name: "/etc/ssl/private/{{ certificate_prefix }}{{ certificate.uuid }}.key"
    - contents: |
        {{ certificate.privatekey | indent(8) }}
    - user: root
    - group: root
    - mode: 640

{% endfor %}

# Create symbolic links to files named by the hash values.
update_ssl_certificates:
  cmd.run:
    - name: "update-ca-certificates --fresh"
