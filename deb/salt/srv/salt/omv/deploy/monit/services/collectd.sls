{% set monitoring_perfstats_config = salt['omv.get_config']('conf.system.monitoring.perfstats') %}
{% set email_config = salt['omv.get_config']('conf.system.notification.email') %}
{% set notification_config = salt['omv.get_config_by_filter'](
  'conf.system.notification.notification',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "monitprocevents"}')[0] %}

{% if monitoring_perfstats_config.enable | to_bool %}

configure_monit_collectd_service:
  file.managed:
    - name: "/etc/monit/conf.d/openmediavault-collectd.conf"
    - source:
      - salt://{{ slspath }}/files/collectd.j2
    - template: jinja
    - context:
        email_config: {{ email_config | json }}
        notification_config: {{ notification_config | json }}
    - user: root
    - group: root
    - mode: 644

{% else %}

remove_monit_collectd_service:
  file.absent:
    - name: "/etc/monit/conf.d/openmediavault-collectd.conf"

{% endif %}
