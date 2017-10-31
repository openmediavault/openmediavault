{% set email_config = salt['omv.get_config']('conf.system.notification.email') %}
{% set users = salt['omv.get_config_by_filter'](
  'conf.system.usermngmnt.user',
  '{"operator": "stringNotEquals", "arg0": "email", "arg1": ""}') %}

# Add a catch-all recipient, thus all emails send to an user/address not
# existing will be redirected to the configured primary recipient address.
configure_postfix_recipient_canonical:
  file.managed:
    - name: "/etc/postfix/recipient_canonical"
    - source:
      - salt://{{ slspath }}/files/recipient_canonical.j2
    - template: jinja
    - context:
        fqdn: {{ salt['network.get_fqdn']() }}
        email_config: {{ email_config | json }}
        users: {{ users | json }}
    - user: root
    - group: root
    - mode: 600
    - require:
      - salt: deploy_postfix_hostname
    - watch_in:
      - service: start_posfix_service

run_postmap_recipient_canonical:
  cmd.run:
    - name: "postmap /etc/postfix/recipient_canonical"
    - onchanges:
      - file: configure_postfix_recipient_canonical
