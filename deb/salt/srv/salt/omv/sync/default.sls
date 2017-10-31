sync_all:
  module.run:
    - name: saltutil.sync_all
    - refresh: True
    - fire_event: True
