# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

configure_phpfpm_webgui:
  file.managed:
    - name: "/etc/php/8.4/fpm/pool.d/openmediavault-webgui.conf"
    - contents: |
        [openmediavault-webgui]
        user = openmediavault-webgui
        group = openmediavault-webgui

        listen = /run/php/php8.4-fpm-openmediavault-webgui.sock
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
        ; https://www.php.net/manual/en/ini.core.php#ini.upload-max-filesize
        php_value[upload_max_filesize] = 25M

        ; Maximum size of POST data that PHP will accept.
        ; https://www.php.net/manual/en/ini.core.php#ini.post-max-size
        php_value[post_max_size] = 25M

        ; Do not expose to the world that PHP is installed on the server.
        ; https://www.php.net/manual/en/ini.core.php#ini.expose-php
        php_value[expose_php] = Off

        ; Name of the session (used as cookie name).
        ; https://www.php.net/manual/en/session.configuration.php#ini.session.name
        php_value[session.name] = OPENMEDIAVAULT-SESSIONID

        ; Whether or not to add the httpOnly flag to the cookie, which makes it
        ; inaccessible to browser scripting languages such as JavaScript.
        ; https://www.php.net/manual/en/session.configuration.php#ini.session.cookie-httponly
        php_value[session.cookie_httponly] = On

        ; Add SameSite attribute to cookie to help mitigate Cross-Site Request Forgery (CSRF/XSRF)
        ; Current valid values are "Lax" or "Strict"
        ; https://datatracker.ietf.org/doc/html/draft-west-first-party-cookies-07
        php_value[session.cookie_samesite] = "Strict"

        ; Use a dedicated, volatile session directory that is owned by the
        ; pool user. Debian's default `/var/lib/php/sessions` can not be
        ; listed by unprivileged users, which breaks the PHP garbage
        ; collection, and it is swept by the `phpsessionclean` timer using
        ; the *global* `session.gc_maxlifetime`, ignoring this pool's value.
        ; The directory is created by `/usr/lib/tmpfiles.d/openmediavault.conf`.
        ; https://www.php.net/manual/en/session.configuration.php#ini.session.save-path
        php_value[session.save_path] = "/run/openmediavault/sessions"

        ; After this number of seconds, stored data will be seen as 'garbage' and
        ; cleaned up by the garbage collection process. This must be at least as
        ; long as the maximum "Session timeout" configurable in the webadmin
        ; settings (see conf.webadmin.json, "timeout", max. 1440 minutes),
        ; otherwise a session could be garbage collected before the application
        ; itself considers it expired.
        ; https://www.php.net/manual/en/session.configuration.php#ini.session.gc-maxlifetime
        php_value[session.gc_probability] = 1
        php_value[session.gc_divisor] = 1000
        php_value[session.gc_maxlifetime] = 86400

        ; Default timeout for socket based streams (seconds)
        ; https://www.php.net/manual/en/filesystem.configuration.php#ini.default-socket-timeout
        php_value[default_socket_timeout] = 90

        ; Maximum execution time of each script, in seconds
        ; https://www.php.net/manual/en/info.configuration.php#ini.max-execution-time
        ; Note: This directive is hardcoded to 0 for the CLI SAPI
        php_value[max_execution_time] = 90
    - mode: '0644'
