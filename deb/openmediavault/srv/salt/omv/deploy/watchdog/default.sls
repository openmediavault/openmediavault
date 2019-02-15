# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2019 Volker Theile
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
# http://www.kernel.org/doc/Documentation/watchdog/
# http://www.gentoo-wiki.info/Watchdog
# http://www.pc-freak.net/blog/how-to-automatically-reboot-restart-debian-gnu-lenny-linux-on-kernel-panic-some-general-cpu-overload-or-system-crash-2

{% set watchdog_device = salt['pillar.get']('default:OMV_WATCHDOG_WATCHDOGDEVICE', '/dev/watchdog') %}
{% set watchdog_realtime = salt['pillar.get']('default:OMV_WATCHDOG_REALTIME', 'yes') %}
{% set watchdog_priority = salt['pillar.get']('default:OMV_WATCHDOG_PRIORITY', '1') %}
{% set watchdog_options = salt['pillar.get']('default:OMV_WATCHDOG_WATCHDOGOPTIONS', '') %}
{% set watchdog_module = salt['pillar.get']('default:OMV_WATCHDOG_WATCHDOGMODULE', 'softdog') %}

configure_default_watchdog:
  file.managed:
    - name: "/etc/default/watchdog"
    - contents: |
        {{ pillar['headers']['multiline'] | indent(8) }}

        # Set run_watchdog to 1 to start watchdog or 0 to disable it.
        # Not used with systemd for the time being.
        run_watchdog=1
        # Specify additional watchdog options here (see manpage).
        watchdog_options="{{ watchdog_options }}"
        # Load module before starting watchdog
        watchdog_module="{{ watchdog_module }}"
        # Set run_wd_keepalive to 1 to start wd_keepalive after stopping watchdog or 0
        # to disable it. Running it is the default.
        run_wd_keepalive=0
    - user: root
    - group: root
    - mode: 644

configure_watchdog_conf:
  file.managed:
    - name: "/etc/watchdog.conf"
    - contents: |
        {{ pillar['headers']['multiline'] | indent(8) }}
        watchdog-device = {{ watchdog_device }}
        # This greatly decreases the chance that watchdog won't be scheduled before
        # your machine is really loaded
        realtime = {{ watchdog_realtime }}
        priority = {{ watchdog_priority }}
    - user: root
    - group: root
    - mode: 644
