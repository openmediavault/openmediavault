# Documentation/Howto:
# http://www.cyberciti.biz/faq/disable-the-mail-alert-by-crontab-command
# http://www.cyberciti.biz/faq/how-do-i-add-jobs-to-cron-under-linux-or-unix-oses
# http://wiki.dreamhost.com/index.php/Crontab

{% set cron_jobs = salt['omv.get_config_by_filter'](
  'conf.system.cron.job',
  '{"operator": "and", "arg0": {"operator": "stringEnum", "arg0": "type", "arg1": ["reboot", "shutdown", "standby"]}, "arg1": {"operator": "equals", "arg0": "enable", "arg1": "1"}}') %}

create_cron_powermanagement:
  file.managed:
    - name: "/etc/cron.d/openmediavault-powermngmt"
    - source:
      - salt://{{ slspath }}/files/powermanagement.j2
    - template: jinja
    - context:
        jobs: {{ cron_jobs | json }}
    - user: root
    - group: root
    - mode: 644
