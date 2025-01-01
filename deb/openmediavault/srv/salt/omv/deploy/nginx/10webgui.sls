# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.

# Documentation/Howto:
# http://wiki.nginx.org/Pitfalls#Taxing_Rewrites
# http://security.stackexchange.com/questions/54639/nginx-recommended-ssl-ciphers-for-security-compatibility-with-pfs
# http://en.wikipedia.org/wiki/List_of_HTTP_header_fields
# http://www.pedaldrivenprogramming.com/2015/04/upgrading-wheezy-to-jessie:-nginx-and-php-fpm/
# https://mozilla.github.io/server-side-tls/ssl-config-generator/?server=nginx-1.14.1&openssl=1.1.1&hsts=yes&profile=modern

{% set include_dir = salt['pillar.get']('default:OMV_NGINX_SITE_WEBGUI_INCLUDE_DIR', '/etc/nginx/openmediavault-webgui.d') %}
{% set config = salt['omv_conf.get']('conf.webadmin') %}

configure_nginx_site_webgui:
  file.managed:
    - name: "/etc/nginx/sites-available/openmediavault-webgui"
    - source:
      - salt://{{ tpldir }}/files/site-webgui.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_nginx_security:
  file.managed:
    - name: "{{ include_dir }}/security.conf"
    - source:
      - salt://{{ tpldir }}/files/security.conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

execute_nginx_ensite:
  cmd.run:
    - name: "nginx_ensite openmediavault-webgui"
