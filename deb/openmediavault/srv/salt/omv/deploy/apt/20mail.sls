# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2024 Volker Theile
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

{% set notification_config = salt['omv_conf.get_by_filter'](
  'conf.system.notification.notification',
  {'operator': 'stringEquals', 'arg0': 'id', 'arg1': 'apt'})[0] %}

{% if notification_config.enable %}

create_apt_listchanges_mail:
  file.managed:
    - name: "/etc/apt/listchanges.conf.d/98openmediavault-mail.conf"
    - contents: |
        [apt]
        email_address=root

create_apt_unattended_upgrades_mail:
  file.managed:
    - name: "/etc/apt/apt.conf.d/98openmediavault-unattended-upgrades-mail"
    - contents: |
        Unattended-Upgrade::Mail "root";

{% else %}

remove_apt_listchanges_mail:
  file.absent:
    - name: "/etc/apt/listchanges.conf.d/98openmediavault-mail.conf"

remove_apt_unattended_upgrades_mail:
  file.absent:
    - name: "/etc/apt/apt.conf.d/98openmediavault-unattended-upgrades-mail"

{% endif %}
