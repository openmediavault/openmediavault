{% set webadmin_config = salt['omv.get_config']('conf.webadmin') %}
{% set zeroconf_config = salt['omv.get_config_by_filter'](
  'conf.service.zeroconf.service',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "webadmin"}')[0] %}

{% if not (zeroconf_config.enable | to_bool) %}

remove_avahi_service_webadmin:
  file.absent:
    - name: "/etc/avahi/services/website.service"

{% else %}

configure_avahi_service_webadmin:
  file.managed:
    - name: "/etc/avahi/services/website.service"
    - source:
      - salt://{{ slspath }}/files/webadmin.j2
    - template: jinja
    - context:
        name: "{{ zeroconf_config.name }}"
        enablessl: {{ webadmin_config.enablessl }}
        forcesslonly: {{ webadmin_config.forcesslonly }}
        port: {{ webadmin_config.port }}
        sslport: {{ webadmin_config.sslport }}
    - user: root
    - group: root
    - mode: 644

{% endif %}
