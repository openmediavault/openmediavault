include:
  - .{{ salt['pillar.get']('deploy_avahi_services', 'default') }}
