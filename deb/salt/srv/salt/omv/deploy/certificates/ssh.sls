{% set certificates = salt['omv.get_config']('conf.system.certificate.ssh') %}
{% set ssh_key_prefix = salt['pillar.get']('default:OMV_SSH_KEY_PREFIX', 'openmediavault-') %}

remove_ssh_certificates:
  file.absent:
    - name: "/etc/ssh/{{ ssh_key_prefix }}*"

{% for certificate in certificates %}

create_ssh_{{ certificate.uuid }}_priv:
  file.managed:
    - name: "/etc/ssh/{{ ssh_key_prefix }}{{ certificate.uuid }}"
    - contents: "{{ certificate.privatekey }}"
    - user: root
    - group: root
    - mode: 600

create_ssh_{{ certificate.uuid }}_pub:
  file.managed:
    - name: "/etc/ssh/{{ ssh_key_prefix }}{{ certificate.uuid }}.pub"
    - contents: "{{ certificate.publickey }}"
    - user: root
    - group: root
    - mode: 644

{% endfor %}
