#!/bin/sh
#
# This file is part of openmediavault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
#
# openmediavault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# openmediavault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with openmediavault. If not, see <http://www.gnu.org/licenses/>.

set -e

# Apply some fixes to the Debian system. Hopefully the will fix that
# upstream soonish.
# https://lists.debian.org/debian-cloud/2021/05/msg00016.html
#
# Force predictable network device files.
# Note, a reboot is required to take this changes into account.
echo "Patching network configuration ..."
sed --in-place --expression='s/eth0/ens6/' --expression='s/eth2/ens8/' /etc/network/interfaces
echo "Patching & rebuilding grub configuration ..."
sed --in-place --expression='s/net.ifnames=0/net.ifnames=1/' --expression='s/biosdevname=0/biosdevname=1/' /etc/default/grub
update-grub
