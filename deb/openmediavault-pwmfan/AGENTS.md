# Repository Guidelines

## Project Structure & Module Organization
- `usr/sbin/omv-pwmfan` is the Python 3 daemon that reads `/etc/openmediavault/pwmfan.json` and drives sysfs PWM.
- `usr/share/openmediavault/` contains plugin integration assets:
  - `confdb/` config DB create/delete hooks.
  - `datamodels/` config + RPC schemas.
  - `engined/` backend module and RPC implementation.
  - `workbench/` UI routes, navigation, and components (`services.pwmfan.*`).
- `lib/systemd/system/openmediavault-pwmfan.service` defines the systemd unit.
- `debian/` holds Debian packaging metadata for the `.deb`.
- `_omv-core/` and `_omv-developer/` are upstream/dev environment artifacts; edit only when working on those toolchains.

## Build, Test, and Development Commands
- `dpkg-buildpackage -b -us -uc` builds the Debian package (requires Debian packaging toolchain).
- `debuild -b` is an alternative build entrypoint for the same package.
- `sudo systemctl restart openmediavault-pwmfan` reloads the daemon after installing a package.
- `sudo /usr/sbin/omv-pwmfan` runs the daemon manually for a smoke test (requires PWM-capable hardware).

## Coding Style & Naming Conventions
- Python: 4-space indentation, PEP 8 style, keep functions small and focused.
- YAML/UI definitions: 2-space indentation; keep `config`, `request`, and `fields` blocks ordered.
- Naming follows OMV plugin conventions: `pwmfan` for modules/paths and `services.pwmfan.*` for workbench assets.

## Testing Guidelines
- No automated test suite is present. Validate on target hardware:
  - Update `/etc/openmediavault/pwmfan.json`, restart the service, and verify fan speed changes.
  - Check `systemctl status openmediavault-pwmfan` for failures.

## Commit & Pull Request Guidelines
- Git history is minimal (e.g., “Initial ...”), so no formal convention is established. Use short, imperative subject lines.
- PRs should include a clear description, testing notes (hardware + command output), and screenshots if UI YAML changes.
- For packaging changes, update `debian/changelog` with a version and summary.

## Security & Configuration Tips
- The daemon writes to `/sys/class/pwm` and reads SMART data; run with appropriate privileges.
- Avoid hardcoding device paths in code; prefer defaults in JSON config and datamodels.
