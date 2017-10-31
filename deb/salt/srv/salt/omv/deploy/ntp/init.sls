include:
  - .{{ salt['pillar.get']('deploy_ntp', 'default') }}
