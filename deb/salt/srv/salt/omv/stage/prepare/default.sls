populate_pillar:
  salt.runner:
    - name: omv.populate_pillar

sync:
  salt.state:
    - tgt: '*'
    - sls: omv.sync
    - failhard: True
