# Documentation/Howto:
# http://www.cyberciti.biz/faq/disable-the-mail-alert-by-crontab-command
# http://www.cyberciti.biz/faq/how-do-i-add-jobs-to-cron-under-linux-or-unix-oses
# http://wiki.dreamhost.com/index.php/Crontab

{% set cron_jobs = salt['omv.get_config_by_filter'](
  'conf.system.cron.job',
  '{"operator": "and", "arg0": {"operator": "stringEquals", "arg0": "type", "arg1": "userdefined"}, "arg1": {"operator": "equals", "arg0": "enable", "arg1": "1"}}') %}
{% set scripts_dir = salt['pillar.get']('default:OMV_CRONTAB_CRONSCRIPTS_DIR', '/var/lib/openmediavault/cron.d') %}
{% set userdefined_prefix = salt['pillar.get']('default:OMV_CRONTAB_USERDEFINED_PREFIX', 'userdefined-') %}

remove_cron_userdefined_scripts:
  file.absent:
    - name: "{{ scripts_dir | path_join(userdefined_prefix + '*') }}"

{% for cron_job in cron_jobs %}
create_cron_userdefined_{{ cron_job.uuid }}_script:
  file.managed:
    - name: "{{ scripts_dir | path_join(userdefined_prefix + cron_job.uuid) }}"
    - contents: |
        #!/bin/sh
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        {{ cron_job.command }}
    - user: {{ cron_job.username }}
    - group: root
    - mode: 744
{% endfor %}

create_cron_userdefined:
  file.managed:
    - name: "/etc/cron.d/openmediavault-userdefined"
    - source:
      - salt://{{ slspath }}/files/userdefined.j2
    - template: jinja
    - context:
        jobs: {{ cron_jobs | json }}
    - user: root
    - group: root
    - mode: 644
