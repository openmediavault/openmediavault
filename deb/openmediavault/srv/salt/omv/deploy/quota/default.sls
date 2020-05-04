# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
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
# http://www.ibm.com/developerworks/linux/library/l-lpic1-v3-104-4/index.html
# https://wiki.archlinux.org/index.php/disk_quota

{% set config = salt['omv_conf.get']('conf.system.filesystem.quota') %}

# Workaround for Jinja2 variables can't be modified from an inner block scope.
{% set ns = namespace(enable_service=False) %}

{% for quota in config %}

{% set device = salt['cmd.run']('blkid -U ' ~ quota.fsuuid) %}
{% set enabled = quota.usrquota | length > 0 or quota.grpquota | length > 0 %}

# Always disable the quota and enable it later if necessary.
# This is the easiest way to do not have to check if quota is already
# enabled (quotaon does not like to be executed when it is already on).
quota_off_{{ quota.fsuuid }}:
  cmd.run:
    - name: quotaoff --group --user "{{ device }}"

{% if enabled | to_bool %}

{% set ns.enable_service = True %}

{% set fstype = salt['disk.fstype'](device) %}
{% if fstype | check_whitelist_blacklist(blacklist=['xfs']) %}
quota_check_{{ quota.fsuuid }}:
  cmd.run:
    - name: quotacheck --user --group --create-files --try-remount --use-first-dquot --verbose "{{ device }}"
{% endif %}

quota_on_{{ quota.fsuuid }}:
  cmd.run:
    - name: quotaon --group --user "{{ device }}"

{% for usrquota in quota.usrquota %}
quota_set_user_{{ quota.fsuuid }}_{{ usrquota.name }}:
  cmd.run:
    - name: setquota --user {{ usrquota.name }} {{ usrquota.bsoftlimit }} {{ usrquota.bhardlimit }} {{ usrquota.isoftlimit }} {{ usrquota.ihardlimit }} "{{ device }}"
{% endfor %}

{% for grpquota in quota.grpquota %}
quota_set_group_{{ quota.fsuuid }}_{{ grpquota.name }}:
  cmd.run:
    - name: setquota --group {{ grpquota.name }} {{ grpquota.bsoftlimit }} {{ grpquota.bhardlimit }} {{ grpquota.isoftlimit }} {{ grpquota.ihardlimit }} "{{ device }}"
{% endfor %}

{% endif %}

{% endfor %}

# Enable or disable quota service.
{% if ns.enable_service | to_bool %}

enable_quota_service:
  service.enabled:
    - name: quota

{% else %}

disable_quota_service:
  service.disabled:
    - name: quota

{% endif %}
