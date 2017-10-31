configure_interfaces:
  file.managed:
    - name: "/etc/network/interfaces"
    - contents:
      - "{{ pillar['headers']['auto_generated'] }}"
      - "{{ pillar['headers']['warning'] }}"
      - ""
      - ""
    - user: root
    - group: root
    - mode: 644
