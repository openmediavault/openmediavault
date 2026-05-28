# Copilot Instructions

This repository contains the source code for the openmediavault project, a Debian-based network attached storage (NAS) solution. The backend is written in PHP, and the frontend is built with Angular/TypeScript. The project also includes various Python and Bash scripts for build and maintenance tasks.

For a complete overview of the project structure, tech stack, and build commands, please refer to the `AGENTS.md` file in the root of the repository.

When reviewing code, use the conventions outlined below to ensure consistency and maintainability. Pay particular attention to the [Security Checks](#security-checks) section.

## General Conventions

- All code must adhere to the standards defined in the `CONTRIBUTING.md` file.
- All file modifications must comply with the rules defined in the `.editorconfig` file.
- All commits must be signed-off (`git commit -s`).

## Security Checks

- For **Shell scripts**, refer to the instructions in `.github/instructions/shell-sec.instructions.md`.
- For **Python scripts**, refer to the instructions in `.github/instructions/python-sec.instructions.md`.
- For **PHP code**, refer to the instructions in `.github/instructions/php-sec.instructions.md`.
- Skip security checks for files that do not contain executable code (e.g., markdown, YAML, JSON, etc.).

### Security Review Summary

Please provide a security review summary in your review comments, even if no issues are found. Use the following format:

```
**Security review**

- Shell scripts: [PASS | N issues — list types] (if applicable)
- Python scripts: [PASS | N issues — list types] (if applicable)
- PHP code: [PASS | N issues — list types] (if applicable)
- Pre-existing issues noted: [none | list]
```
