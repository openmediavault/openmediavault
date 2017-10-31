{% set dirpath = '/srv/salt/' ~ slspath %}

include:
{% for file in salt['file.readdir'](dirpath) %}
{% if file not in ('.', '..', 'init.sls', 'default.sls') %}
{% if file.endswith('.sls') %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endif %}
{% endfor %}
