---
applyTo:
  - "**/*.sh"
  - "**/debian/*.preinst"
  - "**/debian/*.postinst"
  - "**/debian/*.prerm"
  - "**/debian/*.postrm"
  - "**/debian/*.config"
---

# Shell Script Security Guidelines

When analyzing shell scripts (such as install scripts, helpers, or Debian package maintainer scripts), ensure they adhere to secure coding practices. Flag the following common pitfalls:

- **Missing Quotes**: Variables must be quoted (e.g., `"$VAR"`) to prevent unintended word splitting and globbing.
- **Command Injection**: Avoid interpolating untrusted input into commands, particularly when using `eval` or command substitution (`$(...)`).
- **Error Handling**: Scripts should fail fast. Ensure commands that can fail are checked (e.g., `cmd || exit 1`) or use `set -e`.
- **Predictable Temporary Files**: Never use hardcoded paths in `/tmp`. Always utilize `mktemp` to create temporary files safely and prevent race conditions.
- **Input Validation**: Validate all external arguments before using them, especially when constructing file paths, to prevent traversal attacks.
- **Overly Permissive Rights**: Check that newly created files or directories do not have excessive permissions (like `777`).

### Best Practices to Enforce

- **Prefer `dash` over `bash`**: Whenever possible, use `dash` as the interpreter (e.g., `#!/bin/sh`) for better performance and strict POSIX compliance.
- **Avoid Bashisms**: Ensure scripts intended for `/bin/sh` do not use bash-specific features (bashisms) such as `[[ ... ]]`, `arrays`, or `function foo()`. This is crucial for Debian package maintainer scripts.
- **Argument Validation**: Always validate the bounds and types of script arguments (`$1`, `$2`, etc.) before using them.
