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
                'title_load': 'Load average',
                'color_load_shortterm': '#ffbf00',  # yellow
                'color_load_midterm': '#0bb6ff',  # blue
                'color_load_longterm': '#ff1300',  # red
            }
        )
        args = []
        # yapf: disable
        # pylint: disable=line-too-long
        args.append('{image_dir}/load-{period}.png'.format(**config))
        args.extend(config['defaults'])
        args.extend(['--start', config['start']])
        args.extend(['--title', '"{title_load}{title_by_period}"'.format(**config)])
        args.append('--slope-mode')
        args.extend(['--lower-limit', '0'])
        args.extend(['--units-exponent', '0'])
        args.append('DEF:savg={data_dir}/load/load.rrd:shortterm:AVERAGE'.format(**config))
        args.append('DEF:smin={data_dir}/load/load.rrd:shortterm:MIN'.format(**config))
        args.append('DEF:smax={data_dir}/load/load.rrd:shortterm:MAX'.format(**config))
        args.append('DEF:mavg={data_dir}/load/load.rrd:midterm:AVERAGE'.format(**config))
        args.append('DEF:mmin={data_dir}/load/load.rrd:midterm:MIN'.format(**config))
        args.append('DEF:mmax={data_dir}/load/load.rrd:midterm:MAX'.format(**config))
        args.append('DEF:lavg={data_dir}/load/load.rrd:longterm:AVERAGE'.format(**config))
        args.append('DEF:lmin={data_dir}/load/load.rrd:longterm:MIN'.format(**config))
        args.append('DEF:lmax={data_dir}/load/load.rrd:longterm:MAX'.format(**config))
        args.append('LINE1:savg{color_load_shortterm}:" 1 min"'.format(**config))
        args.append('GPRINT:smin:MIN:"%4.2lf Min"')
        args.append('GPRINT:savg:AVERAGE:"%4.2lf Avg"')
        args.append('GPRINT:smax:MAX:"%4.2lf Max"')
        args.append('GPRINT:savg:LAST:"%4.2lf Last\l"')
        args.append('LINE1:mavg{color_load_midterm}:" 5 min"'.format(**config))
        args.append('GPRINT:mmin:MIN:"%4.2lf Min"')
        args.append('GPRINT:mavg:AVERAGE:"%4.2lf Avg"')
        args.append('GPRINT:mmax:MAX:"%4.2lf Max"')
        args.append('GPRINT:mavg:LAST:"%4.2lf Last\l"')
        args.append('LINE1:lavg{color_load_longterm}:"15 min"'.format(**config))
        args.append('GPRINT:lmin:MIN:"%4.2lf Min"')
        args.append('GPRINT:lavg:AVERAGE:"%4.2lf Avg"')
        args.append('GPRINT:lmax:MAX:"%4.2lf Max"')
        args.append('GPRINT:lavg:LAST:"%4.2lf Last\l"')
        args.append('COMMENT:"{last_update}"'.format(**config))
        # yapf: enable
        openmediavault.mkrrdgraph.call_rrdtool_graph(args)
        return 0
