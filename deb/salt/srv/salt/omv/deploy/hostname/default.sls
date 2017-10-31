# Documentation/Howto:
# http://www.debian.org/doc/manuals/debian-reference/ch03.en.html#_the_hostname

{% set config = salt['omv.get_config']('conf.system.network.dns') %}

configure_hostname:
  cmd.run:
    - name: hostnamectl set-hostname {{ config.hostname }}
    - unless: test "{{ config.hostname }}" = "$(hostname)"
