{% set config = salt['omv.get_config']('conf.system.notification.email') %}

configure_postfix_master:
  file.managed:
    - name: "/etc/postfix/master.cf"
    - source:
      - salt://{{ slspath }}/files/master.cf.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644
    - watch_in:
      - service: start_posfix_service
