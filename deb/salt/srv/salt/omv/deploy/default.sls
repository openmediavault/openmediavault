{% set dirpath = '/srv/salt/' ~ slspath %}

include:
{% for file in salt['file.readdir'](dirpath) %}
{% if not file.endswith('.sls') %}
{% if file not in ('.', '..') %}
  - .{{ file }}
{% endif %}
{% endif %}
{% endfor %}
