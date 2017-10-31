# Documentation/Howto:
# http://www.postfix.org/TLS_README.html
# https://en.wikipedia.org/wiki/SMTPS
# http://blog.mailgun.com/25-465-587-what-port-should-i-use/

{% set config = salt['omv.get_config']('conf.system.notification.email') %}

configure_postfix_main:
  file.managed:
    - name: "/etc/postfix/main.cf"
    - source:
      - salt://{{ slspath }}/files/main.cf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644
    - watch_in:
      - service: start_posfix_service
