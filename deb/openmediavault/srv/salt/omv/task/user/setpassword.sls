# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2019 Volker Theile
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

# Example configuration for this task.
# tasks:
#   - sls: omv.task.user.setpassword
#     pillar:
#       name: admin
#       password: $1$/xn7hYMv$2jEtiYm25x1NQ6sH4B2wc.

{% set user = salt['pillar.get']('data', None) %}

{% if user is not none %}

set_password_{{ user.name }}:
  module.run:
    - shadow.set_password:
      - name: {{ user.name }}
      - password: {{ user.password }}

{% endif %}
