refresh_pillar:
  salt.state:
    - tgt: '*'
    - sls: omv.refresh

deploy:
  salt.state:
    - tgt: '*'
    - sls: omv.deploy
