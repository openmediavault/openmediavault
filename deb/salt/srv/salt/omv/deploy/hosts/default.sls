# Documentation/Howto:
# http://www.debian.org/doc/manuals/debian-reference/ch05.en.html#_the_hostname_resolution

{% set interfaces = salt['omv.get_config']('conf.system.network.interface') %}
{% set dns = salt['omv.get_config']('conf.system.network.dns') %}
{% set fqdn = dns.hostname %}
{% set alias = "" %}
{% if dns.domainname %}
{% set fqdn = dns.hostname + "." + dns.domainname %}
{% set alias = dns.hostname %}
{% endif %}

configure_hosts:
  file.managed:
    - name: "/etc/hosts"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        127.0.0.1 localhost
        127.0.1.1 {{ fqdn }} {{ alias }}

        # The following lines are desirable for IPv6 capable hosts.
        ::1     ip6-localhost ip6-loopback
        fe00::0 ip6-localnet
        ff00::0 ip6-mcastprefix
        ff02::1 ip6-allnodes
        ff02::2 ip6-allrouters
        ff02::3 ip6-allhosts
    - user: root
    - group: root
    - mode: 644

{% for interface in interfaces %}

{% if interface.address | is_ipv4 %}
append_hosts_{{ interface.devicename }}_v4:
  host.only:
    - name: "{{ interface.address }}"
    - hostnames:
      - "{{ fqdn }}"
      - "{{ alias }}"
{% endif %}

{% if interface.address6 | is_ipv6 %}
append_hosts_{{ interface.devicename }}_v6:
  host.only:
    - name: "{{ interface.address6 }}"
    - hostnames:
      - "{{ fqdn }}"
      - "{{ alias }}"
{% endif %}

{% endfor %}
