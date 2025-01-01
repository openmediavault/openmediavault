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
# https://wiki.debian.org/AptConfiguration#Be_careful_with_APT::Default-Release

{% set config = salt['omv_conf.get']('conf.system.apt.distribution') %}
{% set use_kernel_backports = salt['pillar.get']('default:OMV_APT_USE_KERNEL_BACKPORTS', True) -%}
{% set proxy_config = salt['omv_conf.get']('conf.system.network.proxy') %}
{% set use_os_security = salt['pillar.get']('default:OMV_APT_USE_OS_SECURITY', True) %}

{% set pkg_repos = [] %}
{% for value in salt['pkg.list_repos']().values() %}
{% set _ = pkg_repos.extend(value) %}
{% endfor %}
{% set security_pkg_repos = pkg_repos | rejectattr('disabled') | selectattr('uri', 'match', '^https?://((deb|security).debian.org|security.ubuntu.com)/.*-security$') | list %}

# Workaround for https://github.com/mvo5/unattended-upgrades/issues/366
remove_apt_default_unattended_upgrades_conf:
  file.absent:
    - name: "/etc/apt/apt.conf.d/50unattended-upgrades"

# Workaround for https://github.com/mvo5/unattended-upgrades/issues/366
divert_apt_default_unattended_upgrades_conf:
  omv_dpkg.divert_add:
    - name: "/etc/apt/apt.conf.d/50unattended-upgrades"

configure_apt_sources_list_openmediavault:
  file.managed:
    - name: "/etc/apt/sources.list.d/openmediavault.list"
    - source:
      - salt://{{ tpldir }}/files/etc-apt-sources_list_d-openmediavault_list.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

{% set apt_conf_files = salt['cmd.run_stdout']('grep --recursive --files-with-matches --regexp="^APT::Default-Release.*" --ignore-case "/etc/apt/apt.conf" "/etc/apt/apt.conf.d/"', ignore_retcode=True) %}
{% for file in (apt_conf_files.split('\n') if apt_conf_files else []) %}
unset_apt_default_release_{{ file | path_basename }}:
  file.replace:
    - name: "{{ file }}"
    - pattern: "^APT::Default-Release.+\n?"
    - flags:
      - "IGNORECASE"
      - "MULTILINE"
    - repl: ""
    - ignore_if_missing: True
    - backup: False
{% endfor %}

{% if proxy_config.http.enable | to_bool or proxy_config.https.enable |
   to_bool or proxy_config.ftp.enable | to_bool %}

configure_apt_proxy:
  file.managed:
    - name: "/etc/apt/apt.conf.d/99openmediavault-proxy"
    - source:
      - salt://{{ tpldir }}/files/etc-apt-apt_conf_d-99openmediavault-proxy.j2
    - context:
        config: {{ proxy_config | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% else %}

remove_apt_proxy:
  file.absent:
    - name: "/etc/apt/apt.conf.d/99openmediavault-proxy"

{% endif %}

{% if not use_kernel_backports | to_bool %}

remove_apt_pref_kernel_backports:
  file.absent:
    - name: "/etc/apt/preferences.d/openmediavault-kernel-backports.pref"

remove_apt_sources_list_kernel_backports:
  file.absent:
    - name: "/etc/apt/sources.list.d/openmediavault-kernel-backports.list"

{% else %}

configure_apt_pref_kernel_backports:
  file.managed:
    - name: "/etc/apt/preferences.d/openmediavault-kernel-backports.pref"
    - source:
      - salt://{{ tpldir }}/files/etc-apt-preferences_d-openmediavault-kernel-backports_pref.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

configure_apt_sources_list_kernel_backports:
  file.managed:
    - name: "/etc/apt/sources.list.d/openmediavault-kernel-backports.list"
    - source:
      - salt://{{ tpldir }}/files/etc-apt-sources_list_d-openmediavault-kernel-backports_list.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% endif %}

# Remove file used on older openmediavault versions.
remove_apt_sources_list_omv_security_obsolete:
  file.absent:
    - name: "/etc/apt/sources.list.d/openmediavault-debian-security.list"

# Delete all security repositories from source lists to prevent APT
# errors related to duplicate definitions.
{% for security_pkg_repo in security_pkg_repos %}
remove_apt_sources_list_security_{{ loop.index0 }}:
  file.line:
    - name: {{ security_pkg_repo.file }}
    - match: {{ security_pkg_repo.line | regex_escape }}
    - mode: delete
    - quiet: True
{% endfor %}

{% if use_os_security | to_bool %}

configure_apt_sources_list_os_security:
  file.managed:
    - name: "/etc/apt/sources.list.d/openmediavault-os-security.list"
    - source:
      - salt://{{ tpldir }}/files/etc-apt-sources_list_d-openmediavault-os-security_list.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% endif %}

remove_cdrom_apt_sources_list:
  file.line:
    - name: "/etc/apt/sources.list"
    - match: "deb cdrom:"
    - mode: delete
    - quiet: True
