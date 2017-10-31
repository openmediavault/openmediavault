{% set webadmin_config = salt['omv.get_config']('conf.webadmin') %}
{% set email_config = salt['omv.get_config']('conf.system.notification.email') %}
{% set notification_config = salt['omv.get_config_by_filter'](
  'conf.system.notification.notification',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "monitprocevents"}')[0] %}

configure_monit_nginx_service:
  file.managed:
    - name: "/etc/monit/conf.d/openmediavault-nginx.conf"
    - source:
      - salt://{{ slspath }}/files/nginx.j2
    - template: jinja
    - context:
        email_config: {{ email_config | json }}
        notification_config: {{ notification_config | json }}
        webadmin_config: {{ webadmin_config | json }}
    - user: root
    - group: root
    - mode: 644
