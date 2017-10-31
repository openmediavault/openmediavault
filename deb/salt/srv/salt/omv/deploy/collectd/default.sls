{% set mkrrdgraph = salt['pillar.get']('default:OMV_MKRRDGRAPH', '/usr/sbin/omv-mkrrdgraph') %}
{% set mkrrdgraph_interval = salt['pillar.get']('default:OMV_MKRRDGRAPH_INTERVAL', '15') %}
{% set config = salt['omv.get_config']('conf.system.monitoring.perfstats') %}

configure_collectd_conf:
  file.managed:
    - name: "/etc/collectd/collectd.conf"
    - source:
      - salt://{{ slspath }}/files/etc-collectd_collectd.conf.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

include:
  - .plugins

{% if config.enable %}

start_collectd_service:
  service.running:
    - name: collectd
    - enable: True
    - watch:
      - file: "/etc/collectd/collectd.conf"
      - file: "/etc/collectd/collectd.conf.d/*.conf"

monitor_collectd_service:
  module.run:
    - name: monit.monitor
    - m_name: "collectd"

install_mkrrdgraph_cron_job:
  file.managed:
    - name: "/etc/cron.d/openmediavault-mkrrdgraph"
    - contents:
      - "# Create graphs every {{ mkrrdgraph_interval }} minutes"
      - "# m h dom mon dow user    command"
      - "*/{{ mkrrdgraph_interval }} * * * * root {{ mkrrdgraph }} >/dev/null 2>&1"
    - user: root
    - group: root
    - mode: 644

generate_rrd_graphs:
  cmd.run:
    - name: "{{ mkrrdgraph }}"

{% else %}

remove_mkrrdgraph_cron_job:
  file.absent:
    - name: "/etc/cron.d/openmediavault-mkrrdgraph"

unmonitor_collectd_service:
  module.run:
    - name: monit.unmonitor
    - m_name: "collectd"

stop_collectd_service:
  service.dead:
    - name: collectd
    - enable: False

{% endif %}
