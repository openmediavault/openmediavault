---
applyTo: "**/*.php"
---

# PHP Security Guidelines

When analyzing PHP code (which is used to implement the backend daemon and RPC services in openmediavault), focus on the following critical vulnerabilities:

- **Command Injection**:
  - **What to look for**: Use of execution functions like `exec()`, `shell_exec()`, `system()`, or `passthru()` with un-sanitized user input from RPC requests.
  - **Best Practice**: Use `escapeshellcmd()` and `escapeshellarg()` to sanitize arguments. Better yet, avoid calling external commands with user input entirely if possible.

- **Path Traversal / File Inclusion**:
  - **What to look for**: File system operations (`file_get_contents`, `include`, `require`, `unlink`) using variables that can be controlled by an RPC caller.
  - **Best Practice**: Avoid using user input to determine file paths. If necessary, strictly validate paths and use an allow-list of permitted files or directories.

- **Insecure Deserialization**:
  - **What to look for**: Use of `unserialize()` on untrusted data from the network, which can lead to object injection and arbitrary code execution.
  - **Best Practice**: Avoid `unserialize()` on user input. Use `json_decode()` for data interchange in RPC services.

- **Improper Input Validation**:
  - **What to look for**: RPC methods trusting the type or content of parameters blindly.
  - **Best Practice**: Always validate and type-cast arguments received via RPC before processing them in the daemon.
