{% set config = salt['omv_conf.get']('conf.service.ftp') %}

configure_proftpd_mod_auth:
  file.append:
    - name: "/etc/proftpd/proftpd.conf"
    - sources:
      - salt://{{ tpldir }}/files/mod_auth.j2
    - template: jinja
    - context:
        config: {{ config | json }}

# Modify /etc/ftpusers because root login is handled by the PAM
# pam_listfile.so module.
{%- if config.rootlogin | to_bool %}

# Allow 'root' to login.
proftpd_ftpusers_allow_root:
  file.comment:
    - name: "/etc/ftpusers"
    - regex: "^root"

{% else %}

# Deny 'root' to login.
proftpd_ftpusers_deny_root:
  file.uncomment:
    - name: "/etc/ftpusers"
    - regex: "^root"

{% endif %}
