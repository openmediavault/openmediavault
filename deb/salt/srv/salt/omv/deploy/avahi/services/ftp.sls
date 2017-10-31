{% set ftp_config = salt['omv.get_config']('conf.service.ftp') %}
{% set zeroconf_config = salt['omv.get_config_by_filter'](
  'conf.service.zeroconf.service',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "ftp"}')[0] %}

{% if not (ftp_config.enable | to_bool and zeroconf_config.enable | to_bool) %}

remove_avahi_service_ftp:
  file.absent:
    - name: "/etc/avahi/services/ftp.service"

{% else %}

configure_avahi_service_ftp:
  file.managed:
    - name: "/etc/avahi/services/ftp.service"
    - source:
      - salt://{{ slspath }}/files/template.j2
    - template: jinja
    - context:
        type: "_ftp._tcp"
        port: {{ ftp_config.port }}
        name: "{{ zeroconf_config.name }}"
    - user: root
    - group: root
    - mode: 644

{% endif %}
