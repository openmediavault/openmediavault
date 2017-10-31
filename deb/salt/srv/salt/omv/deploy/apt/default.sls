{% set config = salt['omv.get_config']('conf.system.apt.distribution') %}
{% set use_kernel_backports = salt['pillar.get']('default:OMV_APT_USE_KERNEL_BACKPORTS', True) -%}

configure_apt_sources_list_openmediavault:
  file.managed:
    - name: "/etc/apt/sources.list.d/openmediavault.list"
    - source:
      - salt://{{ slspath }}/files/etc-apt-sources_list_d-openmediavault_list.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

{% if not use_kernel_backports | to_bool %}

remove_apt_pref_kernel_backports:
  file.absent:
    - name: "/etc/apt/preferences.d/openmediavault-kernel-backports.pref"

remove_apt_sources_list_kernel_backports:
  file.absent:
    - name: "/etc/apt/sources.list.d/openmediavault-kernel-backports.list"

{% else %}

configure_apt_pref_kernel_backports:
  file.managed:
    - name: "/etc/apt/preferences.d/openmediavault-kernel-backports.pref"
    - source:
      - salt://{{ slspath }}/files/etc-apt-preferences_d-openmediavault-kernel-backports_pref.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

configure_apt_sources_list_kernel_backports:
  file.managed:
    - name: "/etc/apt/sources.list.d/openmediavault-kernel-backports.list"
    - source:
      - salt://{{ slspath }}/files/etc-apt-sources_list_d-openmediavault-kernel-backports_list.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% endif %}

refresh_apt_database:
  module.run:
    - name: pkg.refresh_db
