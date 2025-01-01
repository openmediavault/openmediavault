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

# Make sure locale is set up correct. The Python click library requires
# a valid configuration.
# https://click.palletsprojects.com/en/7.x/python3/#python-3-surrogate-handling
# https://www.thomas-krenn.com/de/wiki/Locales_unter_Ubuntu_konfigurieren

# Get the current configured locale.
{% set lang = salt['environ.get']('LANG') %}

generate_C.UTF-8_locale:
  locale.present:
    - name: "C.UTF-8"

{% if lang.upper() != 'C.UTF-8' %}

generate_{{ lang }}_locale:
  locale.present:
    - name: "{{ lang }}"

{% endif %}

# Update /etc/default/locale
set_system_locale:
  cmd.run:
    - name: "update-locale LANG={{ lang }} LANGUAGE= LC_ALL={{ lang }}"
