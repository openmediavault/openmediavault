# Documentation/Howto:
# https://docs.saltstack.com/en/latest/ref/states/all/salt.states.network.html
# https://github.com/saltstack/salt/blob/develop/salt/states/network.py
# https://github.com/saltstack/salt/blob/develop/salt/modules/debian_ip.py
# https://github.com/saltstack/salt/blob/develop/salt/templates/debian_ip/debian_eth.jinja
# http://www.kernel.org/doc/Documentation/networking/bonding.txt
# http://www.kernel.org/doc/Documentation/networking/ip-sysctl.txt
# http://www.cyberciti.biz/tips/configuring-static-routes-in-debian-or-red-hat-linux-systems.html
# http://www.itsyourip.com/networking/add-persistent-static-routes-in-debian-linux
# http://wiki.debian.org/Bonding
# http://www.thomas-krenn.com/de/wiki/NIC_Bonding_unter_Debian
# http://www.linuxfoundation.org/collaborate/workgroups/networking/bonding
# http://www.howtoforge.com/nic-bonding-on-debian-lenny
# http://wiki.linuxmce.org/index.php/MTU
# http://www.simpledns.com/private-ipv6.aspx

{% set dns = salt['omv.get_config']('conf.system.network.dns') %}
{% set interfaces = salt['omv.get_config_by_filter'](
  'conf.system.network.interface',
  '{"operator": "stringEquals", "arg0": "type", "arg1": "bond"}') %}

configure_interfaces_bond:
  file.append:
    - name: "/etc/network/interfaces"
    - sources:
      - salt://{{ slspath }}/files/bond.j2
    - template: jinja
    - context:
        dns: {{ dns | json }}
        interfaces: {{ interfaces | json }}
