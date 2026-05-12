{% set auth_pam = salt['pillar.get']('default:OMV_PROFTPD_MODAUTHPAM_AUTHPAM', 'on') %}
{% set auth_pam_config = salt['pillar.get']('default:OMV_PROFTPD_MODAUTHPAM_AUTHPAMCONFIG', 'proftpd') %}

configure_proftpd_mod_auth_pam:
  file.append:
    - name: "/etc/proftpd/proftpd.conf"
    - text: |
        <IfModule mod_auth_pam.c>
          AuthPAM {{ auth_pam }}
          AuthPAMConfig {{ auth_pam_config }}
        </IfModule>
