# Documentation/Howto:
# http://linuxwiki.de/OpenSSH
# https://help.ubuntu.com/community/SSH/OpenSSH/Configuring
# https://tools.ietf.org/html/rfc4716

# Converting SSH2 public key (OpenSSH -> RFC4716)
# ssh-keygen -e -f ~/.ssh/id_rsa.pub

{% set authorizedKeysFileDir = salt['pillar.get']('default:OMV_SSHD_AUTHORIZEDKEYSFILE_DIR', '/var/lib/openmediavault/ssh/authorized_keys') %}
{% set users = salt['omv.get_config']('conf.system.usermngmnt.user') %}
{% set all_users = salt['user.list_users']() %}

# Remove all existing authorized_keys files. Use file.directory because
# Salt does not support wirdcards for file.absent.
remove_ssh_authorized_keys:
   file.directory:
      - name: {{ authorizedKeysFileDir }}
      - clean: True

{% for user in users %}
{% if user.name in all_users %}
{% set filename = authorizedKeysFileDir | path_join(user.name) %}

create_ssh_authorized_key_file_{{ user.name }}:
  file.managed:
    - name: {{ filename }}
    - mode: 600
    - user: {{ user.name }}
    - contents: ""

{% for sshpubkey in user.sshpubkeys.sshpubkey %}
{% set tmpfile = salt['temp.file']() %}

create_tmp_pubkey_file_{{ user.name }}_{{ loop.index }}:
  file.managed:
    - name: {{ tmpfile }}
    - contents: |
        {{ sshpubkey | indent(8) }}

append_ssh_authorized_key_{{ user.name }}_{{ loop.index }}:
  cmd.run:
    - name: "ssh-keygen -i -f '{{ tmpfile }}' >> '{{ filename }}'"

remove_tmp_pubkey_file_{{ user.name }}_{{ loop.index }}:
  file.absent:
    - name: {{ tmpfile }}

{% endfor %}

{% endif %}
{% endfor %}
