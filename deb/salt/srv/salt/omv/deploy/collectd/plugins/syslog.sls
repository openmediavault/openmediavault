{% set log_level = salt['pillar.get']('default:OMV_COLLECTD_SYSLOG_LOGLEVEL', 'info') %}

configure_collectd_conf_syslog_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/syslog.conf"
    - contents: |
        LoadPlugin syslog
        <Plugin syslog>
            LogLevel {{ log_level }}
        </Plugin>
