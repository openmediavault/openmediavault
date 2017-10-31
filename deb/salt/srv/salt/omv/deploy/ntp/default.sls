# Testing:
# ntpq -p
# ntpd -gq

{% set config = salt['omv.get_config']('conf.system.time') %}

configure_ntp:
  file.managed:
    - name: "/etc/ntp.conf"
    - source:
      - salt://{{ slspath }}/files/etc-ntp_conf.j2
    - template: jinja
    - context:
        timeservers: "{{ config.ntp.timeservers }}"
    - user: root
    - group: root
    - mode: 644

{% if config.ntp.enable %}

start_ntp_service:
  service.running:
    - name: ntp
    - enable: True
    - watch:
      - file: configure_ntp

{% else %}

stop_ntp_service:
  service.dead:
    - name: ntp
    - enable: False

{% endif %}
