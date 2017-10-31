{% set email_config = salt['omv.get_config']('conf.system.notification.email') %}
{% set loadavg_notification_config = salt['omv.get_config_by_filter'](
  'conf.system.notification.notification',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "monitloadavg"}')[0] %}
{% set memory_usage_notification_config = salt['omv.get_config_by_filter'](
  'conf.system.notification.notification',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "monitmemoryusage"}')[0] %}
{% set cpu_usage_notification_config = salt['omv.get_config_by_filter'](
  'conf.system.notification.notification',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "monitcpuusage"}')[0] %}

configure_monit_system_service:
  file.managed:
    - name: "/etc/monit/conf.d/openmediavault-system.conf"
    - source:
      - salt://{{ slspath }}/files/system.j2
    - template: jinja
    - context:
        email_config: {{ email_config | json }}
        loadavg_notification_config: {{ loadavg_notification_config | json }}
        memory_usage_notification_config: {{ loadavg_notification_config | json }}
        cpu_usage_notification_config: {{ loadavg_notification_config | json }}
    - user: root
    - group: root
    - mode: 644
