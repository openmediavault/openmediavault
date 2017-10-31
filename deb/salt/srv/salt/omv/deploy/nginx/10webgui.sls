# Documentation/Howto:
# http://wiki.nginx.org/Pitfalls#Taxing_Rewrites
# http://security.stackexchange.com/questions/54639/nginx-recommended-ssl-ciphers-for-security-compatibility-with-pfs
# http://en.wikipedia.org/wiki/List_of_HTTP_header_fields
# http://www.pedaldrivenprogramming.com/2015/04/upgrading-wheezy-to-jessie:-nginx-and-php-fpm/

{% set include_dir = salt['pillar.get']('default:OMV_NGINX_SITE_WEBGUI_INCLUDE_DIR', '/etc/nginx/openmediavault-webgui.d') %}
{% set config = salt['omv.get_config']('conf.webadmin') %}

configure_nginx_site_webgui:
  file.managed:
    - name: "/etc/nginx/sites-available/openmediavault-webgui"
    - source:
      - salt://{{ slspath }}/files/site-webgui.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644
    - watch_in:
      - service: restart_nginx_service

configure_nginx_security:
  file.managed:
    - name: "{{ include_dir }}/security.conf"
    - source:
      - salt://{{ slspath }}/files/security.conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644
    - watch_in:
      - service: restart_nginx_service

execute_nginx_ensite:
  cmd.run:
    - name: "nginx_ensite openmediavault-webgui"
