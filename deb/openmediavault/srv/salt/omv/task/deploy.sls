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

# Prevent empty rendering.
deploy_task_nop:
  test.nop

# The content of /srv/pillar/omv/tasks.sls may look like:
# tasks:
#   - sls: omv.task.user.setpassword
#     pillar:
#       name: admin
#       password: $1$/xn7hYMv$2jEtiYm25x1NQ6sH4B2wc.
{% set tasks = salt['pillar.get']('tasks', None) %}

{% if tasks is not none %}
{% for task in tasks %}

deploy_task_{{ loop.index0 }}:
  salt.state:
    - tgt: '*'
    - sls: {{ task.sls }}
    - pillar:
        data: {{ task.pillar }}

{% endfor %}
{% endif %}
