{% set socket_file = salt['pillar.get']('default:OMV_COLLECTD_UNIXSOCK_SOCKETFILE', '/var/run/collectd.socket') %}
{% set socket_group = salt['pillar.get']('default:OMV_COLLECTD_UNIXSOCK_SOCKETGROUP', 'root') %}
{% set socket_perms = salt['pillar.get']('default:OMV_COLLECTD_UNIXSOCK_SOCKETPERMS', '0660') %}

configure_collectd_conf_unixsock_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/unixsock.conf"
    - contents: |
        LoadPlugin unixsock
        <Plugin unixsock>
            SocketFile "{{ socket_file }}"
            SocketGroup "{{ socket_group }}"
            SocketPerms "{{ socket_perms }}"
        </Plugin>
