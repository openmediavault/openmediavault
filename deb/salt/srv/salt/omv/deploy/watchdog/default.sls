# Documentation/Howto:
# http://www.kernel.org/doc/Documentation/watchdog/
# http://www.gentoo-wiki.info/Watchdog
# http://www.pc-freak.net/blog/how-to-automatically-reboot-restart-debian-gnu-lenny-linux-on-kernel-panic-some-general-cpu-overload-or-system-crash-2

{% set watchdog_device = salt['pillar.get']('default:OMV_WATCHDOG_WATCHDOGDEVICE', '/dev/watchdog') %}
{% set watchdog_realtime = salt['pillar.get']('default:OMV_WATCHDOG_REALTIME', 'yes') %}
{% set watchdog_priority = salt['pillar.get']('default:OMV_WATCHDOG_PRIORITY', '1') %}
{% set watchdog_options = salt['pillar.get']('default:OMV_WATCHDOG_WATCHDOGOPTIONS', '') %}
{% set watchdog_module = salt['pillar.get']('default:OMV_WATCHDOG_WATCHDOGMODULE', 'softdog') %}

configure_default_watchdog:
  file.managed:
    - name: "/etc/default/watchdog"
    - contents:
      - # Set run_watchdog to 1 to start watchdog or 0 to disable it.
      - # Not used with systemd for the time being.
      - run_watchdog=1
      - # Specify additional watchdog options here (see manpage).
      - watchdog_options="{{ watchdog_options }}"
      - # Load module before starting watchdog
      - watchdog_module="{{ watchdog_module }}"
      - # Set run_wd_keepalive to 1 to start wd_keepalive after stopping watchdog or 0
      - # to disable it. Running it is the default.
      - run_wd_keepalive=0
    - user: root
    - group: root
    - mode: 644

configure_watchdog_conf:
  file.managed:
    - name: "/etc/watchdog.conf"
    - contents:
      - watchdog-device = {{ watchdog_device }}
      - # This greatly decreases the chance that watchdog won't be scheduled before
      - # your machine is really loaded
      - realtime = {{ watchdog_realtime }}
      - priority = {{ watchdog_priority }}
    - user: root
    - group: root
    - mode: 644
