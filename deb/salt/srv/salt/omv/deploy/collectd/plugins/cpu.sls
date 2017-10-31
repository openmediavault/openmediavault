configure_collectd_conf_cpu_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/cpu.conf"
    - contents: "LoadPlugin cpu"
