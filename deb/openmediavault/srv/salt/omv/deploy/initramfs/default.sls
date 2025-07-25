# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

{% set installed_pkgs = salt['pkg.list_pkgs']() %}

# Prevent empty rendering.
update_initramfs_nop:
  test.nop

{% if 'initramfs-tools' in installed_pkgs or 'dracut' in installed_pkgs %}

update_initramfs:
  cmd.run:
{% if 'initramfs-tools' in installed_pkgs %}
    - name: "update-initramfs -u"
{% endif %}
{% if 'dracut' in installed_pkgs %}
    - name: "dracut -f /boot/initrd.img-{{ grains['kernelrelease'] }}"
{% endif %}

{% endif %}
