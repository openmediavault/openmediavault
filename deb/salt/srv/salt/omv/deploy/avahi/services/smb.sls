{% set smb_config = salt['omv.get_config']('conf.service.smb') %}
{% set zeroconf_config = salt['omv.get_config_by_filter'](
  'conf.service.zeroconf.service',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "smb"}')[0] %}

{% if not (smb_config.enable | to_bool and zeroconf_config.enable | to_bool) %}

remove_avahi_service_smb:
  file.absent:
    - name: "/etc/avahi/services/smb.service"

{% else %}

configure_avahi_service_smb:
  file.managed:
    - name: "/etc/avahi/services/smb.service"
    - source:
      - salt://{{ slspath }}/files/template.j2
    - template: jinja
    - context:
        type: "_smb._tcp"
        port: 445
        name: "{{ zeroconf_config.name }}"
    - user: root
    - group: root
    - mode: 644

{% endif %}
