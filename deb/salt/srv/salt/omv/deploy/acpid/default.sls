# Documentation/Howto:
# https://wiki.archlinux.org/index.php/Shutting_system_down_by_pressing_the_power_button

{% set config = salt['omv.get_config']('conf.system.powermngmnt') %}

configure_default_acpid:
  file.managed:
    - name: "/etc/default/acpid"
    - source:
      - salt://{{ slspath }}/files/etc-default-acpid.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

configure_acpid_events:
  file.managed:
    - name: "/etc/acpi/events/powerbtn"
    - source:
      - salt://{{ slspath }}/files/etc-acpi-events-powerbtn.j2
    - template: jinja
    - context:
        powerbtn: {{ config.powerbtn }}
    - user: root
    - group: root
    - mode: 644

restart_acpid_service:
  service.running:
    - name: acpid
    - watch:
      - file: configure_acpid_events
