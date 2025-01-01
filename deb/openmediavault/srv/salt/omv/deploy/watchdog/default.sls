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
# http://0pointer.de/blog/projects/watchdog.html
# https://wiki.archlinux.org/title/Kernel_module
# https://www.kernel.org/doc/html/latest/watchdog/watchdog-parameters.html
# https://www.freedesktop.org/software/systemd/man/systemd-system.conf.html
# https://www.freedesktop.org/software/systemd/man/modules-load.d.html
# https://www.thomas-krenn.com/de/wiki/Watchdog

# Testing
# systemctl show --property=RuntimeWatchdogUSec
# wdctl
# lsmod | grep softdog
# echo c >/proc/sysrq-trigger

{% set watchdog_enabled = salt['pillar.get']('default:OMV_WATCHDOG_ENABLED', 'yes') %}
{% set watchdog_kernel_module_name = salt['pillar.get']('default:OMV_WATCHDOG_MODULE_NAME', 'softdog') %}
{% set watchdog_kernel_module_options = salt['pillar.get']('default:OMV_WATCHDOG_MODULE_OPTIONS', '') %}
{% set watchdog_systemd_runtimewatchdogsec = salt['pillar.get']('default:OMV_WATCHDOG_SYSTEMD_RUNTIMEWATCHDOGSEC', '5min') %}

{% if watchdog_enabled | to_bool %}

# Make sure the watchdog timeout is correct. E.g. there are problems
# on ARM, e.g. RPi, devices.
# https://forums.raspberrypi.com/viewtopic.php?f=29&t=147501
{% if (grains['osarch'] in ['armhf', 'arm64']) and (watchdog_kernel_module_name in ['softdog', 'bcm2835_wdt']) and (watchdog_systemd_runtimewatchdogsec | to_sec > 15) %}
{% set watchdog_systemd_runtimewatchdogsec = 15 %}
{% endif %}

configure_watchdog_systemd:
  file.managed:
    - name: "/etc/systemd/system.conf.d/openmediavault-watchdog.conf"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        [Manager]
        RuntimeWatchdogSec={{ watchdog_systemd_runtimewatchdogsec }}
    - makedirs: True
    - mode: 644

configure_watchdog_module:
  file.managed:
    - name: "/etc/modules-load.d/openmediavault-watchdog.conf"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        {{ watchdog_kernel_module_name }}
    - mode: 644

{% if watchdog_kernel_module_options | length > 0 %}

configure_watchdog_module_options:
  file.managed:
    - name: "/etc/modprobe.d/openmediavault-watchdog.conf"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        options {{ watchdog_kernel_module_name }} {{ watchdog_kernel_module_options }}
    - mode: 644

{% endif %}

restart_systemd_modules_load_service:
  service.running:
    - name: systemd-modules-load
    - enable: True
    - watch:
      - file: configure_watchdog_module

{% else %}

disable_watchdog_systemd:
  file.managed:
    - name: "/etc/systemd/system.conf.d/openmediavault-watchdog.conf"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        [Manager]
        RuntimeWatchdogSec=off
    - makedirs: True
    - mode: 644

remove_watchdog_module:
  file.absent:
    - name: "/etc/modules-load.d/openmediavault-watchdog.conf"

remove_watchdog_module_options:
  file.absent:
    - name: "/etc/modprobe.d/openmediavault-watchdog.conf"

{% endif %}

watchdog_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% if not watchdog_enabled | to_bool %}

unload_watchdog_module:
  kmod.absent:
    - name: {{ watchdog_kernel_module_name }}
    - persist: False
    - comment: False

{% endif %}
