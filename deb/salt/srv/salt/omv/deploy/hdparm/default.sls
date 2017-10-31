# Documentation/Howto:
# http://wiki.debianforum.de/Hdparm

{% set config = salt['omv.get_config']('conf.system.hdparm') %}

configure_hdparm_conf:
  file.managed:
    - name: "/etc/hdparm.conf"
    - source:
      - salt://{{ slspath }}/files/etc-hdparm.conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_pm-utils_hook:
  file.managed:
    - name: "/usr/lib/pm-utils/sleep.d/99openmediavault-hdparm"
    - source:
      - salt://{{ slspath }}/files/usr-lib-pm-utils-sleep.d-99openmediavault-hdparm.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 755
