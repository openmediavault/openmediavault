#!/bin/sh

set -e

export LANG=C.UTF-8
export DEBIAN_FRONTEND=noninteractive
export APT_LISTCHANGES_FRONTEND=none

apt-get update
apt-get install --yes gnupg nano wget systemd udev

cat <<EOF >> ~/.inputrc
"\C-[OA": history-search-backward
"\C-[[A": history-search-backward
"\C-[OB": history-search-forward
"\C-[[B": history-search-forward
EOF
chmod 600 ~/.inputrc
