The following stages are available:

prepare
=======

This stage takes care that the pillar and grains are up-to-date and all
modules are being synced to the minions.

setup
=====

This stage is being run only once after the openmediavault Debian package
has been installed on the system. It is used to set up the system to the
desired requirements.

deploy
======

Deploy the configuration of various services like SMB, FTP, ...

all
===

This stage contains the stages prepare and deploy.
