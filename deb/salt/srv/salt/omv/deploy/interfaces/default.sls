{% set dirpath = '/srv/salt' | path_join(slspath) %}

include:
{% for file in salt['file.readdir'](dirpath) | sort %}
{% if file | regex_match('^(\d+.+).sls$', ignorecase=True) %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endfor %}

restart_networking_service:
  service.running:
    - name: networking
    - enable: True
    - watch:
      - file: configure_interfaces
