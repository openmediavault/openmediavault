# Documentation/Howto:
# http://de.linwiki.org/wiki/Linuxfibel_-_Netzwerk_Server_-_NFS_Server
# http://wiki.ubuntuusers.de/NFS
# http://www.centos.org/docs/5/html/Deployment_Guide-en-US/s1-nfs-server-config-exports.html
# https://help.ubuntu.com/community/NFSv4Howto
# http://jkossen.nl/2009/05/12/simple-nfsv4-configuration-for-debian-and-ubuntu.html
# http://doc.opensuse.org/products/opensuse/openSUSE/opensuse-reference/cha.nfs.html

# Testing:
# showmount -e <nfs-server>

{% set config = salt['omv.get_config']('conf.service.nfs') %}

{% if config.enable %}

configure_default_nfs-kernel-server:
  file.managed:
    - name: "/etc/default/nfs-kernel-server"
    - source:
      - salt://{{ slspath }}/files/etc-default-nfs-kernel-server.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_nfsd_exports:
  file.managed:
    - name: "/etc/exports"
    - source:
      - salt://{{ slspath }}/files/etc-exports.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

start_nfsd_service:
  service.running:
    - name: nfs-kernel-server
    - enable: True
    - watch:
      - file: "/etc/default/nfs-kernel-server"
      - file: "/etc/exports"

{% else %}

stop_nfsd_service:
  service.dead:
    - name: nfs-kernel-server
    - enable: False

{% endif %}
