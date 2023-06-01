{% set config = salt['omv_conf.get']('conf.service.ftp') %}

prereq_proftpd_mod_tls_certificates:
  salt.state:
    - tgt: '*'
    - sls: omv.deploy.certificates

configure_proftpd_mod_tls:
  file.managed:
    - name: "/etc/proftpd/tls.conf"
    - source:
      - salt://{{ tpldir }}/files/mod_tls.j2
    - template: jinja
    - context:
        config: {{ config.modules.mod_tls | json }}
    - user: root
    - group: root
    - mode: 644
