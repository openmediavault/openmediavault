{% set notification_config = salt['omv.get_config_by_filter'](
  'conf.system.notification.notification',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "cronapt"}')[0] %}
{% set email_config = salt['omv.get_config']('conf.system.notification.email') %}

create_cron-apt_config:
  file.managed:
    - name: "/etc/cron-apt/config"
    - source:
      - salt://{{ slspath }}/files/etc_cron-apt_config.j2
    - template: jinja
    - context:
        email_config: {{ email_config | json }}
        notification_config: {{ notification_config | json }}
    - user: root
    - group: root
    - mode: 644
