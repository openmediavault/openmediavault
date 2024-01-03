# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2024 Volker Theile
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

# Testing:
# apt-get install --reinstall python3-lxml=4.3.2-1+deb10u2
# cron-apt -s

{% set notification_config = salt['omv_conf.get_by_filter'](
  'conf.system.notification.notification',
  {'operator': 'stringEquals', 'arg0': 'id', 'arg1': 'cronapt'},
  min_result=1, max_result=1)[0] %}
{% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}
{% set refrain_file = salt['pillar.get']('default:OMV_CRONAPT_REFRAINFILE', '/etc/cron-apt/refrain') %}
{% set security_upgrades_enabled = salt['pillar.get']('default:OMV_CRONAPT_SECURITY_UPGRADES_ENABLED', 1) %}

{% set pkg_repos = [] %}
{% for value in salt['pkg.list_repos']().values() %}
{% set _ = pkg_repos.extend(value) %}
{% endfor %}
{% set security_pkg_repos = pkg_repos | rejectattr('disabled') | selectattr('type', 'equalto', 'deb') | selectattr('uri', 'match', '^https?://security.(debian.org|ubuntu.com)/.*-security$') | list %}

# If this file exist cron-apt will silently exit, so make sure
# it does not exist.
remove_cron-apt_refrain_file:
  file.absent:
    - name: "{{ refrain_file }}"

# Remove the '/etc/cron.d/cron-apt' file installed by the cron-apt package,
# it is replaced by '/etc/cron.daily/openmediavault-cron-apt'.
remove_cron-apt_cron_d_file:
  file.absent:
    - name: "/etc/cron.d/cron-apt"

create_cron-apt_config:
  file.managed:
    - name: "/etc/cron-apt/config"
    - source:
      - salt://{{ tpldir }}/files/etc_cron-apt_config.j2
    - template: jinja
    - context:
        email_config: {{ email_config | json }}
        notification_config: {{ notification_config | json }}
    - user: root
    - group: root
    - mode: 644

divert_cron-apt_config:
  omv_dpkg.divert_add:
    - name: "/etc/cron-apt/config"

create_cron-apt_download_msg:
  file.managed:
    - name: "/etc/cron-apt/mailmsg.d/3-download"
    - source:
      - salt://{{ tpldir }}/files/etc_cron-apt_mailmsgd_3-download.j2
    - user: root
    - group: root
    - mode: 644

# Install security updates automatically if a security repository
# is configured.
remove_cron-apt_config_install_security_upgrades:
  file.absent:
    - name: "/etc/cron-apt/config.d/5-openmediavault-security"

remove_cron-apt_action_install_security_upgrades:
  file.absent:
    - name: "/etc/cron-apt/action.d/5-openmediavault-security"

remove_cron-apt_install_security_upgrades_msg:
  file.absent:
    - name: "/etc/cron-apt/mailmsg.d/5-openmediavault-security"

{% if security_pkg_repos | length > 0 and security_upgrades_enabled | to_bool %}

{% set security_pkg_repo = security_pkg_repos | first %}

create_cron-apt_config_install_security_upgrades:
  file.managed:
    - name: "/etc/cron-apt/config.d/5-openmediavault-security"
    - contents: |
        OPTIONS="--option quiet=1 --option APT::Get::List-Cleanup=false --option Dir::Etc::SourceList={{ security_pkg_repo.file }} --option Dir::Etc::SourceParts=\"/dev/null\" --option DPkg::Options::=--force-confold"
    - user: root
    - group: root
    - mode: 644

create_cron-apt_action_install_security_upgrades:
  file.managed:
    - name: "/etc/cron-apt/action.d/5-openmediavault-security"
    - contents: |
        dist-upgrade --yes --option APT::Get::Show-Upgraded=true
    - user: root
    - group: root
    - mode: 644

create_cron-apt_install_security_upgrades_msg:
  file.managed:
    - name: "/etc/cron-apt/mailmsg.d/5-openmediavault-security"
    - source:
      - salt://{{ tpldir }}/files/etc_cron-apt_mailmsgd_5-openmediavault-security.j2
    - user: root
    - group: root
    - mode: 644

{% endif %}
