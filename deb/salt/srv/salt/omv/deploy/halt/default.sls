# Documentation/Howto:
# http://www.vdr-wiki.de/wiki/index.php/Debian_-_WAKE_ON_LAN
# http://www.brueck-computer.de/index2.php?modul=1404&link=1

configure_default_halt:
  file.managed:
    - name: "/etc/default/halt"
    - source:
      - salt://{{ slspath }}/files/etc-default-halt.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644
