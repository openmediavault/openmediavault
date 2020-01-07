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
                'title_memory': 'Memory usage',
                'color_memory_free': '#76d6ff',  # blue
                'color_memory_cached': '#ffdb70',  # yellow
                'color_memory_buffered': '#c979ff',  # pink
                'color_memory_used': '#ff7a70',  # red
            }
        )
        args = []
        # yapf: disable
        # pylint: disable=line-too-long
        args.append('{image_dir}/memory-{period}.png'.format(**config))
        args.extend(config['defaults'])
        args.extend(['--start', config['start']])
        args.extend(['--title', '"{title_memory}{title_by_period}"'.format(**config)])
        args.append('--slope-mode')
        args.extend(['--lower-limit', '0'])
        args.extend(['--base', '1024'])
        args.extend(['--vertical-label', 'Bytes'])
        args.append('DEF:bavg={data_dir}/memory/memory-buffered.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:bmin={data_dir}/memory/memory-buffered.rrd:value:MIN'.format(**config))
        args.append('DEF:bmax={data_dir}/memory/memory-buffered.rrd:value:MAX'.format(**config))
        args.append('DEF:cavg={data_dir}/memory/memory-cached.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:cmin={data_dir}/memory/memory-cached.rrd:value:MIN'.format(**config))
        args.append('DEF:cmax={data_dir}/memory/memory-cached.rrd:value:MAX'.format(**config))
        args.append('DEF:favg={data_dir}/memory/memory-free.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:fmin={data_dir}/memory/memory-free.rrd:value:MIN'.format(**config))
        args.append('DEF:fmax={data_dir}/memory/memory-free.rrd:value:MAX'.format(**config))
        args.append('DEF:uavg={data_dir}/memory/memory-used.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:umin={data_dir}/memory/memory-used.rrd:value:MIN'.format(**config))
        args.append('DEF:umax={data_dir}/memory/memory-used.rrd:value:MAX'.format(**config))
        args.append('AREA:uavg{color_memory_used}:"Used        "'.format(**config))
        args.append('GPRINT:umin:MIN:"%5.1lf%sB Min"')
        args.append('GPRINT:uavg:AVERAGE:"%5.1lf%sB Avg"')
        args.append('GPRINT:umax:MAX:"%5.1lf%sB Max"')
        args.append('GPRINT:uavg:LAST:"%5.1lf%sB Last\l"')
        args.append('AREA:bavg{color_memory_buffered}:"Buffer cache":STACK'.format(**config))
        args.append('GPRINT:bmin:MIN:"%5.1lf%sB Min"')
        args.append('GPRINT:bavg:AVERAGE:"%5.1lf%sB Avg"')
        args.append('GPRINT:bmax:MAX:"%5.1lf%sB Max"')
        args.append('GPRINT:bavg:LAST:"%5.1lf%sB Last\l"')
        args.append('AREA:cavg{color_memory_cached}:"Page cache  ":STACK'.format(**config))
        args.append('GPRINT:cmin:MIN:"%5.1lf%sB Min"')
        args.append('GPRINT:cavg:AVERAGE:"%5.1lf%sB Avg"')
        args.append('GPRINT:cmax:MAX:"%5.1lf%sB Max"')
        args.append('GPRINT:cavg:LAST:"%5.1lf%sB Last\l"')
        args.append('AREA:favg{color_memory_free}:"Free        ":STACK'.format(**config))
        args.append('GPRINT:fmin:MIN:"%5.1lf%sB Min"')
        args.append('GPRINT:favg:AVERAGE:"%5.1lf%sB Avg"')
        args.append('GPRINT:fmax:MAX:"%5.1lf%sB Max"')
        args.append('GPRINT:favg:LAST:"%5.1lf%sB Last\l"')
        args.append('COMMENT:"{last_update}"'.format(**config))
        # yapf: enable
        openmediavault.mkrrdgraph.call_rrdtool_graph(args)
        return 0
