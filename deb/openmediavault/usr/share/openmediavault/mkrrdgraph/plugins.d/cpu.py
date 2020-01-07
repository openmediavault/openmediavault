# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
import openmediavault.mkrrdgraph
import openmediavault.subprocess


class Plugin(openmediavault.mkrrdgraph.IPlugin):
    def create_graph(self, config):
        # http://paletton.com/#uid=33r0-0kwi++bu++hX++++rd++kX
        config.update(
            {
                'title_cpu': 'CPU usage',
                'color_cpu_idle': '#e7e7e7',
                'color_cpu_nice': '#00df00',
                'color_cpu_user': '#0000fd',
                'color_cpu_waitio': '#fdaf00',
                'color_cpu_system': '#fd0000',
                'color_cpu_softirq': '#fd00fd',
                'color_cpu_irq': '#9f009f',
                'color_cpu_steal': '#000000',
            }
        )
        args = []
        # yapf: disable
        # pylint: disable=line-too-long
        args.append('{image_dir}/cpu-0-{period}.png'.format(**config))
        args.extend(config['defaults'])
        args.extend(['--start', config['start']])
        args.extend(['--title', '"{title_cpu}{title_by_period}"'.format(**config)])
        args.append('--slope-mode')
        args.extend(['--upper-limit', '100'])
        args.extend(['--lower-limit', '0'])
        args.append('--rigid')
        args.extend(['--vertical-label', 'Percent'])
        args.append('DEF:idle={data_dir}/cpu-0/cpu-idle.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:nice={data_dir}/cpu-0/cpu-nice.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:user={data_dir}/cpu-0/cpu-user.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:waitio={data_dir}/cpu-0/cpu-wait.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:system={data_dir}/cpu-0/cpu-system.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:softirq={data_dir}/cpu-0/cpu-softirq.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:interrupt={data_dir}/cpu-0/cpu-interrupt.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:steal={data_dir}/cpu-0/cpu-steal.rrd:value:AVERAGE'.format(**config))
        args.append('AREA:steal{color_cpu_steal}:"Steal"'.format(**config))
        args.append('AREA:system{color_cpu_system}:"System":STACK'.format(**config))
        args.append('AREA:waitio{color_cpu_waitio}:"Wait-IO":STACK'.format(**config))
        args.append('AREA:nice{color_cpu_nice}:"Nice":STACK'.format(**config))
        args.append('AREA:user{color_cpu_user}:"User":STACK'.format(**config))
        args.append('AREA:softirq{color_cpu_softirq}:"Soft-IRQ\c":STACK'.format(**config))
        args.append('AREA:interrupt{color_cpu_irq}:"IRQ":STACK'.format(**config))
        args.append('AREA:idle{color_cpu_idle}:"Idle\c":STACK'.format(**config))
        args.append('COMMENT:"{last_update}"'.format(**config))
        # yapf: enable
        openmediavault.mkrrdgraph.call_rrdtool_graph(args)
        return 0
