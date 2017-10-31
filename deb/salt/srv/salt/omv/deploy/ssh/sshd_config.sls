# Documentation/Howto:
# http://linuxwiki.de/OpenSSH
# https://help.ubuntu.com/community/SSH/OpenSSH/Configuring

{% set config = salt['omv.get_config']('conf.service.ssh') %}

configure_ssh_sshd_config:
  file.managed:
    - name: "/etc/ssh/sshd_config"
    - source:
      - salt://{{ slspath }}/files/sshd_config.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644
