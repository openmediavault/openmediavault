#!/bin/sh
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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

set -e
set -o noglob

. /etc/os-release

OMV_VERSION="8"
OMV_VERSION_CODENAME="synchrony"
DEBIAN_VERSION_CODENAME="trixie"

if [ "$(id -u)" -ne 0 ]; then
	echo "The installation script must be executed as user 'root'." >&2
	exit 77
fi

if [ "${VERSION_CODENAME}" != "${DEBIAN_VERSION_CODENAME}" ]; then
	echo "The Debian version must be '${DEBIAN_VERSION_CODENAME}', found '${VERSION_CODENAME}'." >&2
	exit 1
fi

arch="$(dpkg --print-architecture)"
case "${arch}" in
	amd64|arm64)
		;;
	*)
		echo "The architecture '${arch}' is not supported." >&2
		exit 1
		;;
esac

if dpkg --list | grep --quiet --extended-regexp --word-regexp "gdm3|lightdm|lxdm|sddm|slim|wdm|xdm"; then
  echo "The system is running a desktop environment! Please check https://docs.openmediavault.org/en/${OMV_VERSION}.x/installation/on_debian.html for more details." >&2
  exit 1
fi

echo "Installing openmediavault (${OMV_VERSION_CODENAME}) ..."

export LANG=C.UTF-8
export DEBIAN_FRONTEND=noninteractive
export APT_LISTCHANGES_FRONTEND=none

apt-get install --yes gnupg
wget --output-document=- https://packages.openmediavault.org/public/archive.key | \
	gpg --dearmor --yes --output "/usr/share/keyrings/openmediavault-archive-keyring.gpg"

cat <<EOF > /etc/apt/sources.list.d/openmediavault.list
deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://packages.openmediavault.org/public ${OMV_VERSION_CODENAME} main
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://downloads.sourceforge.net/project/openmediavault/packages ${OMV_VERSION_CODENAME} main
## Uncomment the following line to add software from the proposed repository.
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://packages.openmediavault.org/public ${OMV_VERSION_CODENAME}-proposed main
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://downloads.sourceforge.net/project/openmediavault/packages ${OMV_VERSION_CODENAME}-proposed main
## This software is not part of OpenMediaVault, but is offered by third-party
## developers as a service to OpenMediaVault users.
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://packages.openmediavault.org/public ${OMV_VERSION_CODENAME} partner
# deb [signed-by=/usr/share/keyrings/openmediavault-archive-keyring.gpg] http://downloads.sourceforge.net/project/openmediavault/packages ${OMV_VERSION_CODENAME} partner
EOF

apt-get update
apt-get --yes --auto-remove --show-upgraded \
	--allow-downgrades --allow-change-held-packages \
	--no-install-recommends \
	--option DPkg::Options::="--force-confdef" \
	--option DPkg::Options::="--force-confold" \
	install openmediavault

# Populate the database.
omv-confdbadm populate

# Deploy the /etc/hosts file to ensure the hostname can be resolved
# properly for IPv4 and IPv6. Otherwise building the Salt grains
# (core.fqdns and core.ip_fqdn) will take a very long time.
omv-salt deploy run hosts

# Display the login information.
cat /etc/issue

exit 0
