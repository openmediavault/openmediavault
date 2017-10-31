{% set mountpoints = salt['omv.get_config_by_filter'](
  'conf.system.filesystem.mountpoint',
  '{"operator": "not", "arg0": {"operator": "stringContains", "arg0": "opts", "arg1": "bind"}}') %}

configure_collectd_conf_df_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/df.conf"
    - source:
      - salt://{{ slspath }}/files/collectd-df.j2
    - template: jinja
    - context:
        mountpoints: {{ mountpoints | json }}
