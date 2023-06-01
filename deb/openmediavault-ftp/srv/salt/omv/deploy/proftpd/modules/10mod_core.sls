{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}
{% set ftp_config = salt['omv_conf.get']('conf.service.ftp') %}
{% set homedir_config = salt['omv_conf.get']('conf.system.usermngmnt.homedir') %}
{% set display_login = salt['pillar.get']('default:OMV_PROFTPD_DISPLAYLOGIN', '/etc/proftpd/welcome.msg') %}

configure_proftpd_mod_core:
  file.managed:
    - name: "/etc/proftpd/proftpd.conf"
    - source:
      - salt://{{ tpldir }}/files/mod_core.j2
    - template: jinja
    - context:
        dns_config: {{ dns_config | json }}
        ftp_config: {{ ftp_config | json }}
        homedir_config: {{ homedir_config | json }}
    - user: root
    - group: root
    - mode: 644

# Create welcome message file.
{% if ftp_config.displaylogin | length > 0 %}

configure_proftpd_mod_core_welcome_msg:
  file.managed:
    - name: "{{ display_login }}"
    - contents: "{{ ftp_config.displaylogin }}"

{% else %}

remove_proftpd_mod_core_welcome_msg:
  file.absent:
    - name: "/etc/proftpd/welcome.msg"

{% endif %}
