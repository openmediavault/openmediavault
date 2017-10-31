include:
  - .{{ salt['pillar.get']('deploy_monit_services', 'default') }}
