# Documentation/Howto:
# https://help.ubuntu.com/community/Smartmontools
# http://en.gentoo-wiki.com/wiki/Smartmontools
# http://www.linux-user.de/ausgabe/2004/10/056-smartmontools

{% set config = salt['omv.get_config']('conf.service.smartmontools') %}

configure_default_smartmontools:
  file.managed:
    - name: "/etc/default/smartmontools"
    - source:
      - salt://{{ slspath }}/files/etc-default-smartmontools.j2
    - template: jinja
    - context:
        enable: {{ config.enable }}
        interval: {{ config.interval }}
    - user: root
    - group: root
    - mode: 644

configure_smartd_conf:
  file.managed:
    - name: "/etc/smartd.conf"
    - source:
      - salt://{{ slspath }}/files/etc-smartd_conf.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% if config.enable %}

start_smartd_service:
  service.running:
    - name: smartd
    - enable: True
    - watch:
      - file: configure_smartd_conf

{% else %}

stop_smartd_service:
  service.dead:
    - name: smartd
    - enable: False

{% endif %}
