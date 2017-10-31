configure_collectd_conf_load_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/load.conf"
    - contents: "LoadPlugin load"
