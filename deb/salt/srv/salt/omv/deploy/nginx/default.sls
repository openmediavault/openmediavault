{% set dirpath = '/srv/salt' | path_join(slspath) %}

include:
{% for file in salt['file.readdir'](dirpath) | sort %}
{% if file | regex_match('^(\d+.+).sls$', ignorecase=True) %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endfor %}

test_nginx_service_config:
  cmd.run:
    - name: "nginx -t"

restart_nginx_service:
  service.running:
    - name: nginx
    - enable: True
