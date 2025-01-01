#!/usr/bin/env bash
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

BASE_IMAGE=${BASE_IMAGE:-"docker.io/library/debian:bookworm"}
IMAGE_NAME=${IMAGE_NAME:-"omv7-pkgbuildenv"}

usage() {
	cat <<EOF
Manage the package build environment.

Usage:
  $(basename $0) [options] [command]

Available Commands:
  create      Create but do not start the build environment
  remove      Remove the build environment
  start       Start the build environment
  install     Install all dependencies to create the build environment

Options:
  -h, --help  Show this message.

EOF
	exit 0
}

check_deps() {
	if [ ! $(which buildah) ]; then
	  echo 'Unable to find "buildah". Please make sure it is installed.'
	  exit 1
	fi
	if [ ! $(which podman) ]; then
	  echo 'Unable to find "podman". Please make sure it is installed.'
	  exit 1
	fi
}

create() {
	ctr=$(buildah from ${BASE_IMAGE})
	buildah run ${ctr} /bin/sh -c 'apt -y update'
	buildah run ${ctr} /bin/sh -c 'apt -y install zsh bash-completion sudo fakeroot debhelper gettext doxygen make npm nano debian-keyring devscripts quilt build-essential'
	buildah run ${ctr} /bin/sh -c '
cat <<EOF >> ~/.inputrc
"\C-[OA": history-search-backward
"\C-[[A": history-search-backward
"\C-[OB": history-search-forward
"\C-[[B": history-search-forward
EOF
'
	# buildah config --entrypoint '/bin/zsh' ${ctr}
	buildah config --workingdir '/srv/openmediavault' ${ctr}
	buildah commit --rm ${ctr} ${IMAGE_NAME}
}

remove() {
	podman rm --ignore ${IMAGE_NAME}
	podman image rm ${IMAGE_NAME}
}

start() {
	podman run --interactive --tty --replace \
		--hostname ${IMAGE_NAME} \
		--volume ./deb/:/srv/openmediavault/ \
		--name ${IMAGE_NAME} \
		${IMAGE_NAME} \
		/bin/zsh
}

install() {
	sudo apt-get -y update
	sudo apt-get -y install buildah podman
}

while getopts ":h-:" option
do
	case ${option} in
	-)
		case "${OPTARG}" in
		help)
			usage
			;;
		*)
			echo "Unknown option '${OPTARG}'."
			usage
			;;
		esac
		;;
	h)
		usage
		;;
	esac
done

shift $((OPTIND-1))

case $@ in
create)
	check_deps
	create
	;;
remove)
	check_deps
	remove
	;;
start)
	check_deps
	start
	;;
install)
	install
	;;
*)
	echo "Unknown command '$@'."
	usage
	;;
esac

exit 0
