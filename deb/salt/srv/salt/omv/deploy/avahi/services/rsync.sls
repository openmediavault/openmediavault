{% set rsync_config = salt['omv.get_config']('conf.service.rsyncd') %}
{% set zeroconf_config = salt['omv.get_config_by_filter'](
  'conf.service.zeroconf.service',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "rsync"}')[0] %}

{% if not (rsync_config.enable | to_bool and zeroconf_config.enable | to_bool) %}

remove_avahi_service_rsync:
  file.absent:
    - name: "/etc/avahi/services/rsync.service"

{% else %}

configure_avahi_service_rsync:
  file.managed:
    - name: "/etc/avahi/services/rsync.service"
    - source:
      - salt://{{ slspath }}/files/template.j2
    - template: jinja
    - context:
        type: "_rsync._tcp"
        port: {{ rsync_config.port }}
        name: "{{ zeroconf_config.name }}"
    - user: root
    - group: root
    - mode: 644

{% endif %}
