configure_postfix_transport:
  file.managed:
    - name: "/etc/postfix/transport"
    - contents: "openmediavault-notification@localhost.localdomain omvnotificationfilter:"
    - user: root
    - group: root
    - mode: 600
    - watch_in:
      - service: start_posfix_service

run_postmap_transport:
  cmd.run:
    - name: "postmap /etc/postfix/transport"
    - onchanges:
      - file: configure_postfix_transport
