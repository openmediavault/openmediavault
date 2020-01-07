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
                'title_df': 'Disk usage',
                'color_line_df_free': '#0bb6ff',  # blue
                'color_line_df_used': '#ff1300',  # red
                'color_area_df_free': '#76d6ff',  # blue
                'color_area_df_used': '#ff7a70',  # red
            }
        )
        # Get the mount points from the collectd configuration file.
        mountpoints = openmediavault.mkrrdgraph.load_collectd_config(
            'df', 'MountPoint'
        )
        for mountpoint in mountpoints:
            if mountpoint == '/':
                mountpoint = 'root'
            config['mountpoint'] = mountpoint.lstrip('/').replace('/', '-')
            args = []
            # yapf: disable
            # pylint: disable=line-too-long
            args.append('"{image_dir}/df-{mountpoint}-{period}.png"'.format(**config))
            args.extend(config['defaults'])
            args.extend(['--start', config['start']])
            args.extend(['--title', '"{title_df}{title_by_period}"'.format(**config)])
            args.append('--slope-mode')
            args.extend(['--lower-limit', '0'])
            args.extend(['--vertical-label', 'Bytes'])
            args.append('DEF:favg="{data_dir}/df-{mountpoint}/df_complex-free.rrd":value:AVERAGE'.format(**config))
            args.append('DEF:fmin="{data_dir}/df-{mountpoint}/df_complex-free.rrd":value:MIN'.format(**config))
            args.append('DEF:fmax="{data_dir}/df-{mountpoint}/df_complex-free.rrd":value:MAX'.format(**config))
            args.append('DEF:uavg="{data_dir}/df-{mountpoint}/df_complex-used.rrd":value:AVERAGE'.format(**config))
            args.append('DEF:umin="{data_dir}/df-{mountpoint}/df_complex-used.rrd":value:MIN'.format(**config))
            args.append('DEF:umax="{data_dir}/df-{mountpoint}/df_complex-used.rrd":value:MAX'.format(**config))
            args.append('CDEF:sum=favg,uavg,+')
            args.append('AREA:sum{color_area_df_free}'.format(**config))
            args.append('AREA:uavg{color_area_df_used}'.format(**config))
            args.append('LINE1:sum{color_line_df_free}:"Free"'.format(**config))
            args.append('GPRINT:fmin:MIN:"%5.1lf%sB Min"')
            args.append('GPRINT:favg:AVERAGE:"%5.1lf%sB Avg"')
            args.append('GPRINT:fmax:MAX:"%5.1lf%sB Max"')
            args.append('GPRINT:favg:LAST:"%5.1lf%sB Last\l"')
            args.append('LINE1:uavg{color_line_df_used}:"Used"'.format(**config))
            args.append('GPRINT:umin:MIN:"%5.1lf%sB Min"')
            args.append('GPRINT:uavg:AVERAGE:"%5.1lf%sB Avg"')
            args.append('GPRINT:umax:MAX:"%5.1lf%sB Max"')
            args.append('GPRINT:uavg:LAST:"%5.1lf%sB Last\l"')
            args.append('COMMENT:"{last_update}"'.format(**config))
            # yapf: enable
            openmediavault.mkrrdgraph.call_rrdtool_graph(args)
        return 0
