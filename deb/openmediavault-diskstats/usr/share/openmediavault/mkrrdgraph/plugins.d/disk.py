# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
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
import os


class Plugin(openmediavault.mkrrdgraph.IPlugin):
    def create_graph(self, config):
        # http://paletton.com/#uid=33r0-0kwi++bu++hX++++rd++kX
        config.update(
            {
                'title_disk_octets': 'Disk Traffic',
                'title_disk_ops': 'Disk Operations',
                'title_disk_time': 'Disk time per operation',
                'color_line_disk_read': '#0bb6ff',  # blue
                'color_line_disk_write': '#ff1300',  # red
            }
        )
        # Get the disk devices from the collectd configuration file.
        devnames = openmediavault.mkrrdgraph.load_collectd_config(
            'disk', 'Disk'
        )
        for devname in devnames:
            config['devname'] = devname

            image_filename = '{image_dir}/disk-octets-{devname}-{period}.png'.format(
                **config
            )
            if os.path.exists(
                '{data_dir}/disk-{devname}/disk_octets.rrd'.format(**config)
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_disk_octets} [{devname}]{title_by_period}"'.format(**config),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--lower-limit', '0'])
                args.extend(['--vertical-label', '"Bytes per second"'])
                args.append(
                    'DEF:ravg={data_dir}/disk-{devname}/disk_octets.rrd:read:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:rmin={data_dir}/disk-{devname}/disk_octets.rrd:read:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:rmax={data_dir}/disk-{devname}/disk_octets.rrd:read:MAX'.format(
                        **config
                    )
                )
                args.append('VDEF:rtot=ravg,TOTAL')
                args.append(
                    'DEF:wavg={data_dir}/disk-{devname}/disk_octets.rrd:write:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:wmin={data_dir}/disk-{devname}/disk_octets.rrd:write:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:wmax={data_dir}/disk-{devname}/disk_octets.rrd:write:MAX'.format(
                        **config
                    )
                )
                args.append('VDEF:wtot=wavg,TOTAL')
                args.append(
                    'LINE1:ravg{color_line_disk_read}:"Read"'.format(**config)
                )
                args.append('GPRINT:rmin:MIN:"%5.1lf%s Min"')
                args.append('GPRINT:ravg:AVERAGE:"%5.1lf%s Avg"')
                args.append('GPRINT:rmax:MAX:"%5.1lf%s Max"')
                args.append('GPRINT:rtot:"%5.1lf%s Total\l"')
                args.append(
                    'LINE1:wavg{color_line_disk_write}:"Write"'.format(**config)
                )
                args.append('GPRINT:wmin:MIN:"%5.1lf%s Min"')
                args.append('GPRINT:wavg:AVERAGE:"%5.1lf%s Avg"')
                args.append('GPRINT:wmax:MAX:"%5.1lf%s Max"')
                args.append('GPRINT:wtot:"%5.1lf%s Total\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            else:
                openmediavault.mkrrdgraph.copy_placeholder_image(image_filename)

            image_filename = '{image_dir}/disk-ops-{devname}-{period}.png'.format(
                **config
            )
            if os.path.exists(
                '{data_dir}/disk-{devname}/disk_ops.rrd'.format(**config)
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    ['--title', '"{title_disk_ops} [{devname}]{title_by_period}"'.format(**config)]
                )
                args.append('--slope-mode')
                args.extend(['--lower-limit', '0'])
                args.extend(['--vertical-label', '"Operations per second"'])
                args.append(
                    'DEF:ravg={data_dir}/disk-{devname}/disk_ops.rrd:read:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:rmin={data_dir}/disk-{devname}/disk_ops.rrd:read:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:rmax={data_dir}/disk-{devname}/disk_ops.rrd:read:MAX'.format(
                        **config
                    )
                )
                args.append('VDEF:rtot=ravg,TOTAL')
                args.append(
                    'DEF:wavg={data_dir}/disk-{devname}/disk_ops.rrd:write:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:wmin={data_dir}/disk-{devname}/disk_ops.rrd:write:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:wmax={data_dir}/disk-{devname}/disk_ops.rrd:write:MAX'.format(
                        **config
                    )
                )
                args.append('VDEF:wtot=wavg,TOTAL')
                args.append(
                    'LINE1:ravg{color_line_disk_read}:"Read"'.format(**config)
                )
                args.append('GPRINT:rmin:MIN:"%5.1lf%s Min"')
                args.append('GPRINT:ravg:AVERAGE:"%5.1lf%s Avg"')
                args.append('GPRINT:rmax:MAX:"%5.1lf%s Max"')
                args.append('GPRINT:rtot:"%5.1lf%s Total\l"')
                args.append(
                    'LINE1:wavg{color_line_disk_write}:"Write"'.format(**config)
                )
                args.append('GPRINT:wmin:MIN:"%5.1lf%s Min"')
                args.append('GPRINT:wavg:AVERAGE:"%5.1lf%s Avg"')
                args.append('GPRINT:wmax:MAX:"%5.1lf%s Max"')
                args.append('GPRINT:wtot:"%5.1lf%s Total\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            else:
                openmediavault.mkrrdgraph.copy_placeholder_image(image_filename)

            image_filename = '{image_dir}/disk-time-{devname}-{period}.png'.format(
                **config
            )
            if os.path.exists(
                '{data_dir}/disk-{devname}/disk_time.rrd'.format(**config)
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_disk_time} [{devname}]{title_by_period}"'.format(**config),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--lower-limit', '0'])
                args.extend(['--vertical-label', '"Avg. Time/Op"'])
                args.append(
                    'DEF:ravg={data_dir}/disk-{devname}/disk_time.rrd:read:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:rmin={data_dir}/disk-{devname}/disk_time.rrd:read:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:rmax={data_dir}/disk-{devname}/disk_time.rrd:read:MAX'.format(
                        **config
                    )
                )
                args.append('VDEF:rtot=ravg,TOTAL')
                args.append(
                    'DEF:wavg={data_dir}/disk-{devname}/disk_time.rrd:write:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:wmin={data_dir}/disk-{devname}/disk_time.rrd:write:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:wmax={data_dir}/disk-{devname}/disk_time.rrd:write:MAX'.format(
                        **config
                    )
                )
                args.append('VDEF:wtot=wavg,TOTAL')
                args.append(
                    'LINE1:ravg{color_line_disk_read}:"Read"'.format(**config)
                )
                args.append('GPRINT:rmin:MIN:"%5.1lf%ss Min"')
                args.append('GPRINT:ravg:AVERAGE:"%5.1lf%ss Avg"')
                args.append('GPRINT:rmax:MAX:"%5.1lf%ss Max"')
                args.append('GPRINT:rtot:"%5.1lf%s Total\l"')
                args.append(
                    'LINE1:wavg{color_line_disk_write}:"Write"'.format(**config)
                )
                args.append('GPRINT:wmin:MIN:"%5.1lf%ss Min"')
                args.append('GPRINT:wavg:AVERAGE:"%5.1lf%ss Avg"')
                args.append('GPRINT:wmax:MAX:"%5.1lf%ss Max"')
                args.append('GPRINT:wtot:"%5.1lf%s Total\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            else:
                openmediavault.mkrrdgraph.copy_placeholder_image(image_filename)
        return 0
