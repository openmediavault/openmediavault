{% set sharedfolders_path = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR', '/sharedfolders') %}
{% set sharedfolders_path_escaped = salt['cmd.run']('systemd-escape --path ' + sharedfolders_path)  %}
{% set sharedfolders = salt['omv.get_config']('conf.system.sharedfolder') %}

remove_sharedfolder_unit_files:
  file.absent:
    - name: "/etc/systemd/system/{{ sharedfolders_path_escaped }}-*.mount"

{% for sharedfolder in sharedfolders %}
{% set what = salt['omv.get_sharedfolder_path'](sharedfolder.uuid) %}
{% set where = sharedfolders_path | path_join(sharedfolder.name) %}
{% set unit_name = salt['cmd.run']('systemd-escape --path --suffix=mount ' ~ where)  %}

configure_sharedfolder_{{ sharedfolder.name }}_unit_file:
  file.managed:
    - name: {{ '/etc/systemd/system' | path_join(unit_name) }}
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        [Unit]
        DefaultDependencies=no
        Conflicts=umount.target
        RequiresMountsFor={{ what }} {{ sharedfolders_path }}

        [Mount]
        What={{ what }}
        Where={{ where }}
        Type=none
        Options=bind,nofail

        [Install]
        WantedBy=local-fs.target
    - user: root
    - group: root
    - mode: 644

{% endfor %}

systemctl_daemon_reload:
  module.run:
    - name: service.systemctl_reload
