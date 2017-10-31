include:
  - .{{ salt['pillar.get']('deploy_systemd', 'default') }}
