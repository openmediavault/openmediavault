# Documentation/Howto:
# http://technowizah.com/2007/01/debian-how-to-cpu-frequency-management.html
# http://wiki.hetzner.de/index.php/Cool%27n%27Quiet
# http://wiki.ubuntuusers.de/powernowd

{% set config = salt['omv.get_config']('conf.system.powermngmnt') %}
{%- set governor = salt['grains.filter_by']({
    "default": "ondemand",
    "amd64": "conservative",
    "i386": "conservative"
  }, grain="osarch") %}

configure_default_cpufrequtils:
  file.managed:
    - name: "/etc/default/cpufrequtils"
    - source:
      - salt://{{ slspath }}/files/cpufrequtils.j2
    - template: jinja
    - context:
        cpufreq: {{ config.cpufreq }}
        governor: {{ governor }}
    - user: root
    - group: root
    - mode: 644

configure_default_loadcpufreq:
  file.managed:
    - name: "/etc/default/loadcpufreq"
    - source:
      - salt://{{ slspath }}/files/loadcpufreq.j2
    - template: jinja
    - context:
        cpufreq: {{ config.cpufreq }}
    - user: root
    - group: root
    - mode: 644

{% if config.cpufreq %}

start_cpufrequtils_service:
  service.running:
    - name: cpufrequtils
    - enable: True
    - watch:
      - file: configure_default_cpufrequtils

start_loadcpufreq_service:
  service.running:
    - name: loadcpufreq
    - enable: True
    - watch:
      - file: configure_default_loadcpufreq

{% endfor %}

{% else %}

# Note, when disabling the 'cpufrequtils' service the changes
# will not take effect before the system is rebooted.
{% for service in ['loadcpufreq', 'cpufrequtils'] %}

stop_{{ service }}_service:
  service.dead:
    - name: {{ service }}
    - enable: False

{% endfor %}

{% endif %}
