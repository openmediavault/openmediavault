{% set ssh_config = salt['omv.get_config']('conf.service.ssh') %}
{% set zeroconf_config = salt['omv.get_config_by_filter'](
  'conf.service.zeroconf.service',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "ssh"}')[0] %}

{% if not (ssh_config.enable | to_bool and zeroconf_config.enable | to_bool) %}

remove_avahi_service_ssh:
  file.absent:
    - name: "/etc/avahi/services/ssh.service"

{% else %}

configure_avahi_service_ssh:
  file.managed:
    - name: "/etc/avahi/services/ssh.service"
    - source:
      - salt://{{ slspath }}/files/ssh.j2
    - template: jinja
    - context:
        port: {{ ssh_config.port }}
        name: "{{ zeroconf_config.name }}"
    - user: root
    - group: root
    - mode: 644

{% endif %}
