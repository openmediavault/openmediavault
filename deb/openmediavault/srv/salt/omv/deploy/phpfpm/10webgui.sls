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

configure_phpfpm_webgui:
  file.managed:
    - name: "/etc/php/8.2/fpm/pool.d/openmediavault-webgui.conf"
    - contents: |
        [openmediavault-webgui]
        user = openmediavault-webgui
        group = openmediavault-webgui

        listen = /run/php/php8.2-fpm-openmediavault-webgui.sock
        listen.owner = www-data
        listen.group = www-data
        listen.mode = 0600

        pm = ondemand
        pm.max_children = 25
        pm.process_idle_timeout = 10s

        chdir = /

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; openmediavault php.ini settings ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ; Paths and Directories
        php_value[include_path] = ".:/usr/share/php:/var/www/openmediavault"

        ; Pam Authentication Support (see /etc/pam.d)
        php_value[pam.servicename] = "openmediavault-webgui";

        ; Maximum allowed size for uploaded files.
        ; http://php.net/upload-max-filesize
        php_value[upload_max_filesize] = 25M

        ; Maximum size of POST data that PHP will accept.
        ; http://php.net/post-max-size
        php_value[post_max_size] = 25M

        ; Do not expose to the world that PHP is installed on the server.
        ; http://php.net/expose-php
        php_value[expose_php] = Off

        ; Name of the session (used as cookie name).
        ; http://php.net/session.name
        php_value[session.name] = OPENMEDIAVAULT-SESSIONID

        ; Whether or not to add the httpOnly flag to the cookie, which makes it
        ; inaccessible to browser scripting languages such as JavaScript.
        ; http://php.net/session.cookie-httponly
        php_value[session.cookie_httponly] = On

        ; Add SameSite attribute to cookie to help mitigate Cross-Site Request Forgery (CSRF/XSRF)
        ; Current valid values are "Lax" or "Strict"
        ; https://tools.ietf.org/html/draft-west-first-party-cookies-07
        php_value[session.cookie_samesite] = "Strict"

        ; After this number of seconds, stored data will be seen as 'garbage' and
        ; cleaned up by the garbage collection process.
        ; http://php.net/session.gc-maxlifetime
        php_value[session.gc_maxlifetime] = 86400

        ; Default timeout for socket based streams (seconds)
        ; http://php.net/default-socket-timeout
        php_value[default_socket_timeout] = 90

        ; Maximum execution time of each script, in seconds
        ; http://php.net/max-execution-time
        ; Note: This directive is hardcoded to 0 for the CLI SAPI
        php_value[max_execution_time] = 90
    - mode: 644
