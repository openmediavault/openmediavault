# Contribution rules

Code contributions must satisfy the following conditions. Contributions that do not fulfill these conditions will not be accepted.

- Before jumping into a PR, be sure to search existing PRs or issues for an open or closed item that relates to your submission.
- If there is no related issue, create one.
- You have to sign the CLA online via GitHub.
- The [coding guidelines][1] outlined in the reference manual must be followed.
- The feature/improvement must be implemented as generic as possible.
- The code/feature/improvement must not affect existing functionality.
- Each commit message in a GitHub pull request must be signed via `Signed-off-by: Frank Mustermann <frank.mustermann@xxx.yyy>`. A Developer Certificate of Origin (DCO) is necessary to legally ensure that code contributions are original or authorized, maintaining clear, trustworthy, and compliant open-source project contributions. See [Developer Certificate of Origin](https://developercertificate.org/) and [GitHub DCO App](https://github.com/apps/dco) for more information.
- If possible, [sign your commits][2] with a GPG key.
- If the PR fixes a GitHub issue, then the line `Fixes: https://github.com/openmediavault/openmediavault/issues/<ISSUE_NR>` must be included.
- Make sure your PR has only one commit.
- New features require comprehensive unit tests.
  **Note: This requirement is usually the biggest hurdle for being merged!**

[1]: https://docs.openmediavault.org/en/stable/development/coding_guideline.html
[2]: https://docs.github.com/en/authentication/managing-commit-signature-verification/signing-commits
