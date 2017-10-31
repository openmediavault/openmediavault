{% set config = salt['omv.get_config']('conf.service.ssh') %}
{% set dirpath = '/srv/salt' | path_join(slspath) %}

include:
{% for file in salt['file.readdir'](dirpath) %}
{% if file not in ('.', '..', 'init.sls', 'default.sls') %}
{% if file.endswith('.sls') %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endif %}
{% endfor %}

{% if config.enable | to_bool %}

start_ssh_service:
  service.running:
    - name: ssh
    - enable: True
    - watch:
      - file: configure_ssh_sshd_config

{% else %}

stop_ssh_service:
  service.dead:
    - name: ssh
    - enable: False

{% endif %}
