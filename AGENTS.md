# Agent Instructions

## Git Usage

- You **may** read repository contents, inspect history, and stage changes.
- You **must not** run `git push` or any command that pushes commits to a remote repository.
- You **must not** use `git push --force`, `git push --tags`, or any variant that writes to a remote.
- If a task requires publishing changes, stop and ask the user to confirm before proceeding.

## Repository Layout

- The source code for the `openmediavault` core and plugin Debian packages is located in the `deb/` directory.
- The `pbuilder/` directory contains the scripts used to build packages for different platforms.
- The `vagrant/` directory contains the files for the Debian Vagrant box used for developing `openmediavault` and a Windows Vagrant box for testing Samba and other services.

## UI

- The frontend of `openmediavault` is built with Angular and Material.
- The UI source code is located in `deb/openmediavault/workbench`.
- To install dependencies, run `npm install` inside the `deb/openmediavault/workbench` directory.
- To build the UI, run `npm run build` or `npm run build:prod`.
- To fix and lint the Angular code, run `npm run lint` and `npm run fix`.

