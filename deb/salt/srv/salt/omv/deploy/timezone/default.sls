{% set config = salt['omv.get_config']('conf.system.time') %}

configure_timezone:
  file.managed:
    - name: "/etc/timezone"
    - contents: {{ config.timezone }}
    - user: root
    - group: root
    - mode: 644

set_timezone:
  timezone.system:
    - name: {{ config.timezone }}
    - utc: True
