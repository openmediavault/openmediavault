---
applyTo:
  - "**/locale/**/*.po"
  - "**/locale/**/*.pot"
---

# Translation and Localization Guidelines (.po / .pot files)

When analyzing or reviewing gettext translation files (`*.po`, `*.pot`), ensure they adhere to strict syntax and placeholder integrity rules:

- **Template placeholders (`{{ ... }}`) must NEVER be translated or modified**:
  - Variable names (e.g. `{{ username }}`, `{{ hostname }}`, `{{ name }}`) and filter pipelines (e.g. `{{ items | join(", ") }}`) must remain identical to the original `msgid`.
  - Translating variable names (e.g. translating `{{ username }}` into a local language) breaks the Nunjucks/Angular template parser at runtime and causes UI failures (e.g. `Template render error: expected variable end`).
- **Preserve exact syntax and spacing inside expressions**:
  - Do not introduce spaces into variable paths or array indexers (e.g. `{{ _selected[0].name }}` must NOT become `{{ _selected [0] .name }}`).
- **Printf format specifiers**:
  - Keep standard format specifiers (e.g. `%s`, `%d`, `%CPU`, `%MEM`) intact as defined in the source `msgid`.
- **HTML tags**:
  - Ensure HTML tags (e.g. `<strong>...</strong>`, `<em>...</em>`, `<code>...</code>`) match between `msgid` and `msgstr` without altering tag names or attributes.
