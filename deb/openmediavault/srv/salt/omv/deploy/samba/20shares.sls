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
# http://www.samba.org/samba/docs/man/Samba-HOWTO-Collection/AccessControls.html#id2611892
# http://us5.samba.org/samba/docs/man/manpages-3/smb.conf.5.html
# https://wiki.samba.org/index.php/Configure_Samba_to_Work_Better_with_Mac_OS_X
# http://www.cyberciti.biz/tips/how-do-i-set-permissions-to-samba-shares.html
# http://oreilly.com/catalog/samba/chapter/book/ch06_02.html
# https://www.bsi.bund.de/ContentBSI/grundschutz/kataloge/m/m04/m04332.html
# http://www.redhat.com/advice/tips/sambatrash.html
# http://askubuntu.com/questions/258284/setting-up-an-anonymous-public-samba-share-to-be-accessed-via-windows-7-and-xbmc
# http://blog.jonaspasche.com/2010/11/24/endlich-verstehen-samba-rechtevergabe

{% set config = salt['omv_conf.get']('conf.service.smb') %}
{% set timemachine_shares = salt['omv_conf.get_by_filter'](
  'conf.service.smb.share',
  {'operator': 'equals', 'arg0': 'timemachine', 'arg1': '1'}) %}

configure_samba_shares:
  file.append:
    - name: "/etc/samba/smb.conf"
    - sources:
      - salt://{{ tpldir }}/files/shares.j2
    - template: jinja
    - context:
        config: {{ config | json }}
        enable_timemachine_vfs: {{ timemachine_shares | length > 0 }}
