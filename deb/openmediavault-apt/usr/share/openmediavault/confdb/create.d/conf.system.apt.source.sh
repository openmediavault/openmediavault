#!/usr/bin/env dash
#
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

set -e

. /etc/default/openmediavault
. /usr/share/openmediavault/scripts/helper-functions

########################################################################
# Update the configuration.
# See https://repolib.readthedocs.io/en/latest/deb822-format.html#deb822-style-format
# <config>
#   <system>
#     <apt>
#       <sources>
#         <!--
#         <item>
#           <uuid>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</uuid>
#           <enable>0|1</enable>
#           <types>xxx</types>
#           <uris>xxx</uris>
#           <suites>xxx</suites>
#           <components>xxx</components>
#           <signedby>xxx</signedby>
#           <comment>xxx</comment>
#         </item>
#         -->
#       </sources>
#     </apt>
#   </system>
# </config>
########################################################################
if ! omv-confdbadm exists "conf.system.apt.source"; then
	omv_config_add_node "/config/system/apt" "sources"
	# Add the Debian Bookworm Backports repository as an example.
	jq --null-input --compact-output "{uuid: \"${OMV_CONFIGOBJECT_NEW_UUID}\", \
			enable: \"false\", types: \"deb\", \
			uris: \"https://deb.debian.org/debian\", \
			suites: \"bookworm-backports\", \
			components: \"main contrib non-free non-free-firmware\", \
			signedby: \"-----BEGIN PGP PUBLIC KEY BLOCK-----\n\n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\nNlB2T6bKGcYGqL7L\n=UUyy\n-----END PGP PUBLIC KEY BLOCK-----\", \
			comment: \"Bookworm, Backports\"}" |
		omv-confdbadm update "conf.system.apt.source" -
fi


exit 0
