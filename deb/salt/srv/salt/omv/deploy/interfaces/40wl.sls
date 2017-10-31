# Documentation/Howto:
# https://docs.saltstack.com/en/latest/ref/states/all/salt.states.network.html
# https://github.com/saltstack/salt/blob/develop/salt/states/network.py
# https://github.com/saltstack/salt/blob/develop/salt/modules/debian_ip.py
# https://github.com/saltstack/salt/blob/develop/salt/templates/debian_ip/debian_eth.jinja

{% set dns = salt['omv.get_config']('conf.system.network.dns') %}
{% set interfaces = salt['omv.get_config_by_filter'](
  'conf.system.network.interface',
  '{"operator": "stringEquals", "arg0": "type", "arg1": "wireless"}') %}

configure_interfaces_wl:
  file.append:
    - name: "/etc/network/interfaces"
    - sources:
      - salt://{{ slspath }}/files/wl.j2
    - template: jinja
    - context:
        dns: {{ dns | json }}
        interfaces: {{ interfaces | json }}
