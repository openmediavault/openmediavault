{% set interfaces = [] %}
{% for interface in salt['omv.get_config_by_filter'](
  'conf.system.network.interface',
  '{"operator": "or", "arg0": {"operator": "stringEquals", "arg0": "type", "arg1": "ethernet"}, "arg1": {"operator": "stringEquals", "arg0": "type", "arg1": "wireless"}}') %}
{% set used = salt['omv.get_config_by_filter'](
  'conf.system.network.interface',
  '{"operator": "stringContains", "arg0": "slaves", "arg1": "' + interface.devicename + '"}') %}
{% if used | length == 0 %}
{% set interfaces = interfaces.append(interface) %}
{% endif %}
{% endfor %}
{% set interfaces = interfaces + salt['omv.get_config_by_filter'](
  'conf.system.network.interface',
  '{"operator": "or", "arg0": {"operator": "stringEquals", "arg0": "type", "arg1": "bond"}, "arg1": {"operator": "stringEquals", "arg0": "type", "arg1": "vlan"}}') %}

configure_collectd_conf_interface_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/interface.conf"
    - source:
      - salt://{{ slspath }}/files/collectd-interface.j2
    - template: jinja
    - context:
        interfaces: {{ interfaces | json }}
