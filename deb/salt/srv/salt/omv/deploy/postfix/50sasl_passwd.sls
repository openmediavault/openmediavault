{% set config = salt['omv.get_config']('conf.system.notification.email') %}

configure_postfix_sasl_passwd:
  file.managed:
    - name: "/etc/postfix/sasl_passwd"
    - source:
      - salt://{{ slspath }}/files/sasl_passwd.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 600
    - watch_in:
      - service: start_posfix_service

run_postmap_sasl_passwd:
  cmd.run:
    - name: "postmap /etc/postfix/sasl_passwd"
    - onchanges:
      - file: configure_postfix_sasl_passwd
