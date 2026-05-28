# Agent Instructions

This document provides a comprehensive overview of the openmediavault project for developers and AI agents.

## Repository Layout

The project uses a monorepo approach for historical reasons and to facilitate the sharing of build scripts. The main directories are structured as follows:

```
.
├── deb/                  # Source code for all Debian packages
│   ├── openmediavault/   # The 'core' package
│   ├── openmediavault-*/ # Official plugins
│   ├── php-pam/          # Example of an essential, maintained package
│   └── openmediavault.mk # Shared Make script for building packages
├── pbuilder/             # Scripts for building packages for different platforms
└── vagrant/              # Vagrant boxes for development and testing
```

## Tech Stack

| Component | Technology |
|---|---|
| Backend | PHP |
| Frontend | Angular, TypeScript, Material Design |
| Build System | Make, dpkg-buildpackage |
| Scripting | Python, Bash |
| Dependency Management | Composer (PHP), npm (JS) |
| CI/CD | GitHub Actions |
| Testing | PHPUnit, Jest, BATS |

## Build & Run

All build and maintenance tasks are managed via `make` from within a specific package directory (e.g., `deb/openmediavault-ftp/`) or via `npm` for the UI.

```bash
# Build a Debian package for a plugin
cd deb/openmediavault-someplugin/
fakeroot debian/rules clean binary

# Format Python code in a plugin
fakeroot debian/rules omv_fix_py

# Lint Python code in a plugin
fakeroot debian/rules omv_lint_py

# Format PHP code in a plugin
fakeroot debian/rules omv_fix_php

# Lint PHP code in a plugin
fakeroot debian/rules omv_lint_php

# Test PHP code in a plugin
fakeroot debian/rules omv_test_php

# Install UI dependencies
cd deb/openmediavault/workbench/
npm ci

# Build the UI for production
npm run build:prod

# Lint and fix UI code
npm run fix
```

## Git Usage

- You **may** read repository contents, inspect history, and stage changes.
- You **must not** run `git push` or any command that pushes commits to a remote repository.
- If a task requires publishing changes, stop and ask the user to confirm before proceeding.
- All commits must be signed-off (`git commit -s`).

## Coding Guidelines

- You **must** strictly follow the coding standards and conventions outlined in the `CONTRIBUTING.md` file.
- Ensure all file modifications comply with the rules defined in the `.editorconfig` file.
- **PHP**: Adhere to PSR-12. Use the provided `make` targets for formatting, linting, and testing. All tests are run automatically in CI.
- **TypeScript/Angular**: Align with the project's Prettier and ESLint configurations.
- **Python**: Adhere to PEP 8. Use the provided `make` targets for formatting and linting.
