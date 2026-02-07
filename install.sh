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

OMV_VERSION="8"
OMV_VERSION_CODENAME="synchrony"
OMV_GIT_URL="https://github.com/openmediavault/openmediavault/raw/master/deb/openmediavault/"
DEBIAN_VERSION_CODENAME="trixie"

info() {
	echo "[INFO] $@"
}

warn() {
	echo "[WARN] $@" >&2
}

error() {
	echo "[ERROR] $@" >&2
}

install_verify_user() {
	if [ "$(id -u)" -ne 0 ]; then
		error "The installation script must be executed as user 'root'."
		exit 77
	fi
}

install_verify_codename() {
	. /etc/os-release
	if [ "${VERSION_CODENAME}" != "${DEBIAN_VERSION_CODENAME}" ]; then
		error "The Debian version must be '${DEBIAN_VERSION_CODENAME}', found '${VERSION_CODENAME}'."
		exit 1
	fi
}

install_verify_arch() {
	arch="$(dpkg --print-architecture)"
	case "${arch}" in
		amd64|arm64)
			;;
		*)
			error "The architecture '${arch}' is not supported."
			exit 1
			;;
	esac
}

install_verify_de() {
	if dpkg --list | grep --quiet --extended-regexp --word-regexp "gdm3|lightdm|lxdm|sddm|slim|wdm|xdm"; then
		error "The system is running a desktop environment! Please check https://docs.openmediavault.org/en/${OMV_VERSION}.x/installation/on_debian.html for more details."
		exit 1
	fi
}

do_install() {
	info "Installing openmediavault (${OMV_VERSION_CODENAME}) ..."

	install_verify_user
	install_verify_codename
	install_verify_arch
	install_verify_de

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
}

do_fix() {
	info "Fixing the openmediavault (${OMV_VERSION_CODENAME}) installation ..."

	install_verify_user

	# Fix dpkg-divert problems.
	file="/usr/sbin/omv-mkaptidx"
	wget --output-document="${file}" "${OMV_GIT_URL}/${file}"
	chmod 755 "${file}"

	# Rebuild APT plugin and update indices.
	omv-mkaptidx
}

command="install"
while [ $# -gt 0 ]; do
	case "$1" in
	--fix)
		command="fix"
		shift
		;;
	*)
		error "Unknown argument: $1"
		exit 1
		;;
	esac
done

case "${command}" in
	fix)
		do_fix
		;;
	install)
		do_install
		;;
esac

exit 0
