{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}
{% set ftp_config = salt['omv_conf.get']('conf.service.ftp') %}

configure_proftpd_mod_core:
  file.managed:
    - name: "/etc/proftpd/proftpd.conf"
    - source:
      - salt://{{ tpldir }}/files/mod_core.j2
    - template: jinja
    - context:
        dns_config: {{ dns_config | json }}
        ftp_config: {{ ftp_config | json }}
    - user: root
    - group: root
    - mode: 644
    - watch_in:
      - service: start_proftpd_service

# Create welcome message file.
{% if ftp_config.displaylogin | length > 0 %}

configure_proftpd_mod_core_custom_login_msg:
  file.managed:
    - name: "/srv/ftp/welcome.msg"
    - contents: "{{ ftp_config.displaylogin }}"

{% else %}

configure_proftpd_mod_core_default_login_msg:
  file.managed:
    - name: "/srv/ftp/welcome.msg"
    - contents: |
        Welcome user %U@%R to %L FTP server.
        The local time is: %T

{% endif %}
