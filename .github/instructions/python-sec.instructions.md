---
applyTo: "**/*.py"
---

# Python Security Guidelines

When analyzing Python code, watch for the following common security vulnerabilities, especially in backend scripts and services.

- **Command Injection**:
  - **What to look for**: Usage of `os.system`, `subprocess.run` with `shell=True`, or any function that executes shell commands with un-sanitized input.
  - **Best Practice**: Avoid `shell=True`. Pass arguments as a list (e.g., `subprocess.run(["ls", "-l", untrusted_input])`).

- **Insecure Deserialization**:
  - **What to look for**: Use of `pickle`, `cPickle`, or `yaml.load`. These can execute arbitrary code.
  - **Best Practice**: Use safe serialization formats like `json` for data exchange. If complex objects are needed, use `json.dumps` with a custom `default` handler.

- **Hardcoded Secrets**:
  - **What to look for**: Passwords, API keys, or other secrets directly in the source code.
  - **Best Practice**: Secrets should be loaded from environment variables, configuration files (outside the repo), or a secrets management system.

- **Insecure Temporary Files**:
  - **What to look for**: Predictable temporary file paths, especially in `/tmp`.
  - **Best Practice**: Always use the `tempfile` module (e.g., `tempfile.mkstemp()` or `tempfile.NamedTemporaryFile()`) to create temporary files securely.

- **Weak Cryptography**:
  - **What to look for**: Use of broken or weak cryptographic algorithms like MD5 or SHA1 for hashing passwords or sensitive data.
  - **Best Practice**: Use modern, strong hashing algorithms like Argon2 or bcrypt via libraries like `passlib`.
