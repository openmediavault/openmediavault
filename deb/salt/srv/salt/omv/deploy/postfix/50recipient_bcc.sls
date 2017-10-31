{% set config = salt['omv.get_config']('conf.system.notification.email') %}

# BCC (blind carbon-copy) all emails send to the primary email address to
# the given secondary email address. Do not use 'sender_bcc_maps' for that,
# otherwise all emails, including local users, will be sent, too.
configure_postfix_recipient_bcc:
  file.managed:
    - name: "/etc/postfix/recipient_bcc"
    - source:
      - salt://{{ slspath }}/files/recipient_bcc.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 600
    - watch_in:
      - service: start_posfix_service

run_postmap_recipient_bcc:
  cmd.run:
    - name: "postmap /etc/postfix/recipient_bcc"
    - onchanges:
      - file: configure_postfix_recipient_bcc
