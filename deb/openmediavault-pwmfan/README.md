# openmediavault-pwmfan
OpenMediaVault 7 PWM fan controller plugin for Raspberry Pi.

## Install (global repo)
This plugin is published via GitHub Pages from the `docs/` folder.

1. Enable GitHub Pages for this repository (Settings → Pages → Deploy from `/docs`).
2. On your OMV system, add the repo and install:

```
deb [trusted=yes] https://ashcal9669.github.io/openmediavault-pwmfan ./
```

```
sudo apt update
sudo apt install openmediavault-pwmfan
```

If it does not appear in the OMV Plugins UI, run:

```
sudo omv-salt deploy run apt
```

## Build
```
dpkg-buildpackage -b -us -uc
```

## Notes
- Architectures: `armhf`, `arm64`.
- The daemon reads `/etc/openmediavault/pwmfan.json`.
