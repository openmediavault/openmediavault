{% set certificates = salt['omv.get_config']('conf.system.certificate.ssl') %}
{% set certificate_prefix = salt['pillar.get']('default:OMV_SSL_CERTIFICATE_PREFIX', 'openmediavault-') %}

remove_ssl_certificates:
  file.absent:
    - names:
      - "/etc/ssl/certs/{{ certificate_prefix }}*.crt"
      - "/etc/ssl/private/{{ certificate_prefix }}*.key"

{% for certificate in certificates %}

create_ssl_{{ certificate.uuid }}_crt:
  file.managed:
    - name: "/etc/ssl/certs/{{ certificate_prefix }}{{ certificate.uuid }}.crt"
    - contents: "{{ certificate.certificate }}"
    - user: root
    - group: root
    - mode: 644

create_ssl_{{ certificate.uuid }}_key:
  file.managed:
    - name: "/etc/ssl/private/{{ certificate_prefix }}{{ certificate.uuid }}.key"
    - contents: "{{ certificate.privatekey }}"
    - user: root
    - group: root
    - mode: 640

{% endfor %}

# Create symbolic links to files named by the hash values.
update_ssl_certificates:
  cmd.run:
    - name: "update-ca-certificates --fresh"
