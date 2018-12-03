The following stages are available:

prepare
=======

This stage takes care that the pillar and grains are up to date and all modules are sync'ed to the minions.

setup
=====

This stage is run only once after the openmediavault Debian package has been installed on the system. It is used to setup the system to the desired requirements.

deploy
======

Deploy the configuration of various services like SMB, FTP, ...

all
===

This stage containes the stages prepare and deploy.
