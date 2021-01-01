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
# http://www.kernel.org/doc/Documentation/watchdog/
# http://www.gentoo-wiki.info/Watchdog
# http://www.pc-freak.net/blog/how-to-automatically-reboot-restart-debian-gnu-lenny-linux-on-kernel-panic-some-general-cpu-overload-or-system-crash-2

{% set watchdog_default_options = salt['pillar.get']('default:OMV_WATCHDOG_DEFAULT_OPTIONS', '') %}
{% set watchdog_default_module = salt['pillar.get']('default:OMV_WATCHDOG_DEFAULT_MODULE', 'softdog') %}
{% set watchdog_conf_device = salt['pillar.get']('default:OMV_WATCHDOG_CONF_DEVICE', '/dev/watchdog') %}
{% set watchdog_conf_realtime = salt['pillar.get']('default:OMV_WATCHDOG_CONF_REALTIME', 'yes') %}
{% set watchdog_conf_priority = salt['pillar.get']('default:OMV_WATCHDOG_CONF_PRIORITY', '1') %}
{% set watchdog_conf_watchdog_timeout = salt['pillar.get']('default:OMV_WATCHDOG_CONF_WATCHDOG_TIMEOUT', '') %}

configure_default_watchdog:
  file.managed:
    - name: "/etc/default/watchdog"
    - contents: |
        {{ pillar['headers']['multiline'] | indent(8) }}

        # Set run_watchdog to 1 to start watchdog or 0 to disable it.
        # Not used with systemd for the time being.
        run_watchdog=1
        # Specify additional watchdog options here (see manpage).
        watchdog_options="{{ watchdog_default_options }}"
        # Load module before starting watchdog
        watchdog_module="{{ watchdog_default_module }}"
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
        watchdog-device = {{ watchdog_conf_device }}
        # This greatly decreases the chance that watchdog won't be scheduled before
        # your machine is really loaded
        realtime = {{ watchdog_conf_realtime }}
        priority = {{ watchdog_conf_priority }}
        {%- if watchdog_conf_watchdog_timeout %}
        watchdog-timeout = {{ watchdog_conf_watchdog_timeout }}
        {% endif %}
    - user: root
    - group: root
    - mode: 644
