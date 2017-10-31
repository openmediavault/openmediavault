{% set config = salt['omv.get_config']('conf.system.notification.email') %}

{% if config.enable | to_bool %}

remove_postfix_cron:
  file.absent:
    - name: "/etc/cron.hourly/openmediavault-flushmailq"

{% else %}

configure_postfix_cron:
  file.managed:
    - name: "/etc/cron.hourly/openmediavault-flushmailq"
    - contents: |
        #!/bin/sh
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        # Flush the mail queue every hour if email notification is disabled.
        postsuper -d ALL
    - user: root
    - group: root
    - mode: 750
    - watch_in:
      - service: start_posfix_service

{% endif %}
