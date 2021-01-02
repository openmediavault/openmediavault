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
# http://blog.shadypixel.com/monitoring-a-ups-with-nut-on-debian-or-ubuntu-linux/
# http://www.die-welt.net/2011/02/mustek-powemust-1000-usb-on-debian-squeeze/
# http://beeznest.wordpress.com/2008/07/14/howto-setup-nut-network-ups-tools-on-debian/
# http://kiserai.net/blog/2009/03/07/belkin-universal-ups-with-nut-debian
# http://rogerprice.org/NUT.html
# http://linuxman.wikispaces.com/NUT
# http://ifireball.wordpress.com/2011/10/13/configuring-aviem-pro2100-ups-on-debian-6-0-stablesqueeze/
# http://blog.shadypixel.com/monitoring-a-ups-with-nut-on-debian-or-ubuntu-linux
# http://adi.roiban.ro/2011/11/10/monitor-the-ups-in-ubuntu-with-network-ups-tools

{% set nut_config = salt['omv_conf.get']('conf.service.nut') %}

{% if nut_config.enable | to_bool %}

{% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}
{% set admin_user = salt['pillar.get']('default:OMV_NUT_UPSDUSERS_ADMIN_USER', 'admin') %}
{% set admin_passwd = salt['pillar.get']('default:OMV_NUT_UPSDUSERS_ADMIN_PASSWORD', salt['random.get_str'](16)) %}
{% set monitor_user = salt['pillar.get']('default:OMV_NUT_UPSDUSERS_ADMIN_USER', 'monmaster') %}
{% set monitor_passwd = salt['pillar.get']('default:OMV_NUT_UPSDUSERS_MONITOR_PASSWORD', salt['random.get_str'](16)) %}

configure_nut_nut_conf:
  file.managed:
    - name: "/etc/nut/nut.conf"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        MODE={{ "netserver" if nut_config.remotemonitor | to_bool else nut_config.mode }}
    - user: root
    - group: nut
    - mode: 640

configure_nut_ups_conf:
  file.managed:
    - name: "/etc/nut/ups.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-nut-ups_conf.j2
    - template: jinja
    - context:
        config: {{ nut_config | json }}
    - user: root
    - group: nut
    - mode: 640

configure_nut_upsd_conf:
  file.managed:
    - name: "/etc/nut/upsd.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-nut-upsd_conf.j2
    - template: jinja
    - context:
        config: {{ nut_config | json }}
    - user: root
    - group: nut
    - mode: 640

configure_nut_upsd_users:
  file.managed:
    - name: "/etc/nut/upsd.users"
    - source:
      - salt://{{ tpldir }}/files/etc-nut-upsd_users.j2
    - template: jinja
    - context:
        config: {{ nut_config | json }}
        admin_user: {{ admin_user }}
        admin_passwd: {{ admin_passwd }}
        monitor_user: {{ monitor_user }}
        monitor_passwd: {{ monitor_passwd }}
    - user: root
    - group: nut
    - mode: 640

configure_nut_upsmon_conf:
  file.managed:
    - name: "/etc/nut/upsmon.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-nut-upsmon_conf.j2
    - template: jinja
    - context:
        config: {{ nut_config | json }}
        monitor_user: {{ monitor_user }}
        monitor_passwd: {{ monitor_passwd }}
    - user: root
    - group: nut
    - mode: 640

configure_nut_upssched_conf:
  file.managed:
    - name: "/etc/nut/upssched.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-nut-upssched_conf.j2
    - template: jinja
    - context:
        config: {{ nut_config | json }}
    - user: root
    - group: nut
    - mode: 640

configure_nut_default_upssched_cmd:
  file.managed:
    - name: "/etc/default/upssched-cmd"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        OMV_NUT_UPSSCHEDCMD_EMAIL_ENABLE="{{ "YES" if email_config.enable | to_bool else "NO" }}"
        OMV_NUT_UPSSCHEDCMD_SHUTDOWNTIMER={{ nut_config.shutdowntimer }}
    - user: root
    - group: nut
    - mode: 640

# Create a udev rule for the given port if necessary.
{% set port = nut_config.driverconf | regex_search('port\s*=\s*([a-zA-Z0-9\/]*)\s*.*') %}
# Result looks like: ('auto',) or ('/dev/ttyS1',)
{% if port | length == 1 and port[0] not in (None, 'auto') %}

configure_nut_udev_serialups_rule:
  file.managed:
    - name: "/etc/udev/rules.d/99-openmediavault-nut-serialups.rules"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        KERNEL=="{{ port[0] | path_basename }}", GROUP="nut"
    - user: root
    - group: nut
    - mode: 640

configure_nut_udevadm_trigger:
  cmd.run:
    - name: "udevadm trigger"
    - onchanges:
      - configure_nut_udev_serialups_rule

{% else %}

remove_nut_udev_serialups_rule:
  file.absent:
  - name: "/etc/udev/rules.d/99-openmediavault-nut-serialups.rules"

{% endif %}

{% if nut_config.mode != 'netclient' %}

start_nut_driver_service:
  service.running:
    - name: nut-driver
    - enable: True
    - watch:
      - file: configure_nut_ups_conf

start_nut_server_service:
  service.running:
    - name: nut-server
    - enable: True
    - watch:
      - file: configure_nut_upsd_conf
      - file: configure_nut_upsmon_conf

monitor_nut_server_service:
  monit.monitor:
    - name: nut-server

{% endif %}

start_nut_monitor_service:
  service.running:
    - name: nut-monitor
    - enable: True
    - watch:
      - file: configure_nut_upsmon_conf

monitor_nut_monitor_service:
  monit.monitor:
    - name: nut-monitor

{% else %}

remove_nut_udev_serialups_rule:
  file.absent:
  - name: "/etc/udev/rules.d/99-openmediavault-nut-serialups.rules"

unmonitor_nut_monitor_service:
  monit.unmonitor:
    - name: nut-monitor

unmonitor_nut_server_service:
  monit.unmonitor:
    - name: nut-server

stop_nut_monitor_service:
  service.dead:
    - name: nut-monitor
    - enable: False

stop_nut_server_service:
  service.dead:
    - name: nut-server
    - enable: False

stop_nut_driver_service:
  service.dead:
    - name: nut-driver
    - enable: False

{% endif %}
