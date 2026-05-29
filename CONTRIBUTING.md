# Contribution rules

Code contributions must satisfy the following conditions. Contributions that do not fulfill these conditions will not be accepted.

- Before jumping into a PR, be sure to search existing PRs or issues for an open or closed item that relates to your submission.
- If there is no related issue, create one.
- You have to sign the CLA online via GitHub.
- The feature/improvement must be implemented as generic as possible.
- The code/feature/improvement must not affect existing functionality.
- Each commit message in a GitHub pull request must be signed via `Signed-off-by: Frank Mustermann <frank.mustermann@xxx.yyy>`. A Developer Certificate of Origin (DCO) is necessary to legally ensure that code contributions are original or authorized, maintaining clear, trustworthy, and compliant open-source project contributions. See [Developer Certificate of Origin](https://developercertificate.org/) and [GitHub DCO App](https://github.com/apps/dco) for more information.
- If possible, [sign your commits][1] with a GPG key.
- If the PR fixes a GitHub issue, then the line `Fixes: https://github.com/openmediavault/openmediavault/issues/<ISSUE_NR>` must be included.
- Make sure your PR has only one commit.
- New features require comprehensive unit tests. All PHP code contributions must pass the PHPUnit tests that are automatically run in the CI pipeline.

## Coding Guideline

These standards for code formatting and documentation must be followed by anyone contributing to the openmediavault project. Any contributions that do not fulfill these guidelines will not be accepted.

### File Formatting

All files must adhere to the rules defined in the `.editorconfig` file at the root of the project. This ensures consistency in indentation, line endings, and character encoding.

For readability, please try to keep lines at a maximum of 120 characters.

### PHP

The PHP code follows the [PSR-12](https://www.php-fig.org/psr/psr-12/) standard. Please make sure your code adheres to this standard. To automatically format the code and check for issues, you can use the following commands from within a package directory (e.g. `deb/openmediavault`):

- **Formatting**: `fakeroot debian/rules omv_fix_php`
- **Linting**: `fakeroot debian/rules omv_lint_php`

#### Naming Conventions

- **Classes**: Must use the `OMV` namespace and follow StudlyCaps naming.
- **Methods & Variables**: Must use camelCase.
- **Constants**: Should start with `OMV_` and be in UPPER_CASE.

### TypeScript/Angular

The UI is built with Angular and TypeScript. We use Prettier and Angular ESLint to enforce a consistent code style.

- **Formatting**: Run `npm run fix` in the `deb/openmediavault/workbench` directory to automatically format your code.
- **Linting**: Run `npm run lint` in the `deb/openmediavault/workbench` directory to check for any linting errors.

Please make sure your code is free of any linting errors before submitting a pull request.

### Python

Python code should adhere to the [PEP 8](https://www.python.org/dev/peps/pep-0008/) style guide.

- **Formatting**: Use `autopep8` to automatically format your code. To format the Python code within a plugin directory, you can use the command `fakeroot debian/rules omv_fix_py`.
- **Linting**: Run `fakeroot debian/rules omv_lint_py` to check for linting errors.
- **Typing**: Use type hints for all new code.

[1]: https://docs.github.com/en/authentication/managing-commit-signature-verification/signing-commits
