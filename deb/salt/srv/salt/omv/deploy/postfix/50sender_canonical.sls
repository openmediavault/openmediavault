{% set config = salt['omv.get_config']('conf.system.notification.email') %}

# Rewrite all outgoing emails with the configured sender address, otherwise
# the SMTP relay will bounce them, e.g.
# postfix/smtp[xxxxx]: XXXXXXXXXX: to=<abc@xyz.localdomain>,
#   orig_to=<test>, relay=mail.gmx.net[x.x.x.x]:25, delay=1,
#   delays=0.02/0.02/0.93/0.06, dsn=5.7.0, status=bounced (host
#   mail.gmx.net[x.x.x.x] said: 550 5.7.0 Sender address does not belong to
#   logged in user {mp030} (in reply to MAIL FROM command))
configure_postfix_sender_canonical:
  file.managed:
    - name: "/etc/postfix/sender_canonical"
    - source:
      - salt://{{ slspath }}/files/sender_canonical.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 600
    - watch_in:
      - service: start_posfix_service

run_postmap_sender_canonical:
  cmd.run:
    - name: "postmap /etc/postfix/sender_canonical"
    - onchanges:
      - file: configure_postfix_sender_canonical
