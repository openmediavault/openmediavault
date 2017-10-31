configure_collectd_conf_memory_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/memory.conf"
    - contents: "LoadPlugin memory"
