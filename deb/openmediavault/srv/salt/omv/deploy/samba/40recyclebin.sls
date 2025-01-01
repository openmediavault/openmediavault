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
# http://www.samba.org/samba/docs/man/Samba-HOWTO-Collection/VFS.html#id2651247
# http://www.redhat.com/advice/tips/sambatrash.html

{% set scripts_dir = salt['pillar.get']('default:OMV_CRONSCRIPTS_DIR', '/var/lib/openmediavault/cron.d') %}
{% set cron_scripts_prefix = salt['pillar.get']('default:OMV_SAMBA_SHARE_RECYCLE_CRONSCRIPT_PREFIX', 'samba-recycle-') %}
{% set config = salt['omv_conf.get']('conf.service.smb') %}

configure_samba_recyclebin_cron:
  file.managed:
    - name: "/etc/cron.daily/openmediavault-samba-recycle"
    - contents: |
        #!/usr/bin/env dash
        {{ pillar['headers']['multiline'] | indent(8) }}
        set -e
        run-parts --new-session --regex='{{ cron_scripts_prefix }}*' {{ scripts_dir }} >/dev/null 2>&1
    - user: root
    - group: root
    - mode: 750

remove_samba_recyclebin_cron_scripts:
  module.run:
    - file.find:
      - path: "{{ scripts_dir }}"
      - iname: "{{ cron_scripts_prefix }}*"
      - delete: "f"

{% if config.enable | to_bool %}

{% for share in config.shares.share | selectattr('recyclebin') %}

configure_samba_recyclebin_cron_script_{{ share.uuid }}:
  file.managed:
    - name: "{{ scripts_dir | path_join(cron_scripts_prefix ~ share.uuid) }}"
    - source:
      - salt://{{ tpldir }}/files/cron-recyclebin-script.j2
    - context:
        share: {{ share | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 750

{% endfor %}

{% endif %}
