#!/usr/bin/env dash
#
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

set -e

########################################################################
# Update the configuration.
# <config>
#   <services>
#     <tftp>
#     	<!--
#     	<blocksize>[512-65464]</blocksize>
#     	<sharedfolderref>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</sharedfolderref>
#     	-->
#     	<enable>0</enable>
#     	<port>69</port>
#     	<retransmit>1000000</retransmit>
#     	<blocksize>512</blocksize>
#     	<allownewfiles>0</allownewfiles>
#     	<sharedfolderref></sharedfolderref>
#     	<extraoptions></extraoptions>
#     </tftp>
#   </services>
# </config>
########################################################################
if ! omv-confdbadm exists "conf.service.tftp"; then
	omv-confdbadm read --defaults "conf.service.tftp" | omv-confdbadm update "conf.service.tftp" -
fi

exit 0
