# Documentation/Howto:
# https://docs.saltstack.com/en/latest/ref/states/all/salt.states.network.html
# https://github.com/saltstack/salt/blob/develop/salt/states/network.py
# https://github.com/saltstack/salt/blob/develop/salt/modules/debian_ip.py
# https://github.com/saltstack/salt/blob/develop/salt/templates/debian_ip/debian_eth.jinja

configure_interfaces_lo:
  file.append:
    - name: "/etc/network/interfaces"
    - sources:
      - salt://{{ slspath }}/files/lo.j2
    - template: jinja
