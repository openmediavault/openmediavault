{% set config = salt['omv.get_config']('conf.system.notification.email') %}

# BCC (blind carbon-copy) all emails to the notification framework.
configure_postfix_sender_bcc:
  file.managed:
    - name: "/etc/postfix/sender_bcc"
    - source:
      - salt://{{ slspath }}/files/sender_bcc.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 600
    - watch_in:
      - service: start_posfix_service

run_postmap_sender_bcc:
  cmd.run:
    - name: "postmap /etc/postfix/sender_bcc"
    - onchanges:
      - file: configure_postfix_sender_bcc
