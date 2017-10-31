{% set dirpath = '/srv/salt' | path_join(slspath) %}

include:
{% for file in salt['file.readdir'](dirpath) %}
{% if file not in ('.', '..', 'init.sls', 'default.sls') %}
{% if file.endswith('.sls') %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endif %}
{% endfor %}

test_php-fpm_service_config:
  cmd.run:
    - name: "php-fpm7.0 -t"

restart_php-fpm_service:
  service.running:
    - name: php7.0-fpm
    - enable: True
