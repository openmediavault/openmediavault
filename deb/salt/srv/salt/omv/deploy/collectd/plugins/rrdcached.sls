# Documentation/Howto:
# http://www.korbinian-pauli.de/artikel/mit-lm-sensors-und-rrd-cpu-und-mb-temperatur-uberwachen
# http://www.uvm.edu/~bcodding/ticker/archives/33
# http://blog.stefan-betz.net/2009/1/6/collectd-und-rrdtool-im-einsatz
# http://wiki.lugbz.de/HowTo/SelfMadeMonitoring
# https://jeremy.visser.name/2010/02/23/enable-caching-in-collectd
# https://collectd.org/wiki/index.php/Inside_the_RRDtool_plugin

# Testing:
# rrdtool info /var/lib/rrdcached/db/localhost/<xxx>/<xxx>.rrd

{% set rrdcached_socket_file = salt['pillar.get']('default:OMV_RRDCACHED_SOCKETFILE', '/var/run/rrdcached.sock') %}
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
