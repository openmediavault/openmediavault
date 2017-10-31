# Documentation/Howto:
# http://ticktoo.com/blog/32-Linux+Software+Raid%3A+mdadm+Performance+Tuning
# http://www.cyberciti.biz/tips/linux-raid-increase-resync-rebuild-speed.html

{% set speed_limit_min = salt['pillar.get']('default:OMV_SYSCTL_DEV_RAID_SPEEDLIMITMIN', '10000') %}

# Improve maximun md array reconstruction speed.
configure_sysctl_mdadm:
  file.managed:
    - name: "/etc/sysctl.d/99-openmediavault-mdadm.conf"
    - contents:
      - "{{ pillar['headers']['auto_generated'] }}"
      - "{{ pillar['headers']['warning'] }}"
      - "#"
      - "# Default values:"
      - "# dev.raid.speed_limit_min = 1000"
      - "dev.raid.speed_limit_min = {{ speed_limit_min }}"
    - user: root
    - group: root
    - mode: 644
