include:
  - .{{ salt['pillar.get']('deploy_iptables', 'default') }}
