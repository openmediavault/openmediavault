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
# http://www.korbinian-pauli.de/artikel/mit-lm-sensors-und-rrd-cpu-und-mb-temperatur-uberwachen
# http://www.uvm.edu/~bcodding/ticker/archives/33
# http://blog.stefan-betz.net/2009/1/6/collectd-und-rrdtool-im-einsatz
# http://wiki.lugbz.de/HowTo/SelfMadeMonitoring
# https://jeremy.visser.name/2010/02/23/enable-caching-in-collectd
# https://collectd.org/wiki/index.php/Inside_the_RRDtool_plugin

# Testing:
# rrdtool info /var/lib/rrdcached/db/localhost/<xxx>/<xxx>.rrd

{% set rrdcached_socket_file = salt['pillar.get']('default:OMV_RRDCACHED_SOCKETFILE', '/run/rrdcached.sock') %}
{% set data_dir = salt['pillar.get']('default:OMV_RRDCACHED_BASEDIR', '/var/lib/rrdcached/db/') %}
{% set create_files = salt['pillar.get']('default:OMV_COLLECTD_RRDCACHED_CREATEFILES', 'true') %}
{% set collect_statistics = salt['pillar.get']('default:OMV_COLLECTD_RRDCACHED_COLLECTSTATISTICS', 'true') %}

configure_collectd_conf_rrdcached_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/rrdcached.conf"
    - contents: |
        LoadPlugin rrdcached
        <Plugin rrdcached>
            DaemonAddress "unix:{{ rrdcached_socket_file }}"
            DataDir "{{ data_dir }}"
            CreateFiles {{ create_files }}
            CollectStatistics {{ collect_statistics }}
        </Plugin>
