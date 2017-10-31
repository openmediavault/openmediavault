{% set nfs_export_dir = salt['pillar.get']('default:OMV_NFSD_EXPORT_DIR', '/export') %}
{% set nfs_config = salt['omv.get_config']('conf.service.nfs') %}
{% set zeroconf_config = salt['omv.get_config_by_filter'](
  'conf.service.zeroconf.service',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "nfs"}')[0] %}

{% if not (nfs_config.enable | to_bool and zeroconf_config.enable | to_bool) %}

remove_avahi_service_nfs:
  file.absent:
    - name: "/etc/avahi/services/nfs-*.service"

{% else %}

# Announce duplicate shares only once.
{% set nfsshares = salt['omv.get_config_by_filter'](
  'conf.service.nfs.share',
  '{"operator": "distinct", "arg0": "sharedfolderref"}') %}
{% for nfsshare in nfsshares %}

{% set sharedfolder = salt['omv.get_config'](
  'conf.system.sharedfolder', nfsshare.sharedfolderref) %}

configure_avahi_service_nfs:
  file.managed:
    - name: "/etc/avahi/services/nfs-{{ sharedfolder.name }}.service"
    - source:
      - salt://{{ slspath }}/files/template.j2
    - template: jinja
    - context:
        type: "_nfs._tcp"
        port: 2049
        name: "{{ zeroconf_config.name }} - {{ sharedfolder.name }}"
        txtrecord: "path={{ nfs_export_dir }}/{{ sharedfolder.name }}"
    - user: root
    - group: root
    - mode: 644

{% endfor %}

{% endif %}
