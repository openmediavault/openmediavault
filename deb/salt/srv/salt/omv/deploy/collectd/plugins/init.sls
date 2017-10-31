include:
  - .{{ salt['pillar.get']('deploy_collectd_plugins', 'default') }}
