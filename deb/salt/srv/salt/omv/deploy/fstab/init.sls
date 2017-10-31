include:
  - .{{ salt['pillar.get']('deploy_fstab', 'default') }}
