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
                'color_interface_incoming': '#0bb6ff',  # blue
                'color_interface_outgoing': '#ff1300',  # red
            }
        )
        # Get the network interfaces from the collectd configuration file.
        interfaces = openmediavault.mkrrdgraph.load_collectd_config(
            'interface', 'Interface'
        )
        for interface in interfaces:
            config['interface'] = interface
            args = []
            # yapf: disable
            # pylint: disable=line-too-long
            args.append('{image_dir}/interface-{interface}-{period}.png'.format(**config))
            args.extend(config['defaults'])
            args.extend(['--start', config['start']])
            args.extend(['--title', '"{interface} traffic{title_by_period}"'.format(**config)])
            args.append('--slope-mode')
            args.extend(['--lower-limit', '0'])
            args.extend(['--vertical-label', 'Bits/s'])
            args.append('DEF:oavgraw={data_dir}/interface-{interface}/if_octets.rrd:tx:AVERAGE'.format(**config))
            args.append('DEF:ominraw={data_dir}/interface-{interface}/if_octets.rrd:tx:MIN'.format(**config))
            args.append('DEF:omaxraw={data_dir}/interface-{interface}/if_octets.rrd:tx:MAX'.format(**config))
            args.append('DEF:iavgraw={data_dir}/interface-{interface}/if_octets.rrd:rx:AVERAGE'.format(**config))
            args.append('DEF:iminraw={data_dir}/interface-{interface}/if_octets.rrd:rx:MIN'.format(**config))
            args.append('DEF:imaxraw={data_dir}/interface-{interface}/if_octets.rrd:rx:MAX'.format(**config))
            args.append('CDEF:oavg=oavgraw,8,*')
            args.append('CDEF:omin=ominraw,8,*')
            args.append('CDEF:omax=omaxraw,8,*')
            args.append('VDEF:otot=oavg,TOTAL')
            args.append('CDEF:iavg=iavgraw,8,*')
            args.append('CDEF:imin=iminraw,8,*')
            args.append('CDEF:imax=imaxraw,8,*')
            args.append('VDEF:itot=iavg,TOTAL')
            args.append('CDEF:tavg=oavg,iavg,+')
            args.append('CDEF:tmin=omin,imin,+')
            args.append('CDEF:tmax=omax,imax,+')
            args.append('LINE1:oavg{color_interface_outgoing}:"Outgoing"'.format(**config))
            args.append('GPRINT:oavg:AVERAGE:"%5.1lf%s Avg"')
            args.append('GPRINT:omax:MAX:"%5.1lf%s Max"')
            args.append('GPRINT:oavg:LAST:"%5.1lf%s Last"')
            args.append('GPRINT:otot:"%5.1lf%s Total\l"')
            args.append('LINE1:iavg{color_interface_incoming}:"Incoming"'.format(**config))
            args.append('GPRINT:iavg:AVERAGE:"%5.1lf%s Avg"')
            args.append('GPRINT:imax:MAX:"%5.1lf%s Max"')
            args.append('GPRINT:iavg:LAST:"%5.1lf%s Last"')
            args.append('GPRINT:itot:"%5.1lf%s Total\l"')
            args.append('COMMENT:"  Total   "')
            args.append('GPRINT:tavg:AVERAGE:"%5.1lf%s Avg"')
            args.append('GPRINT:tmax:MAX:"%5.1lf%s Max"')
            args.append('GPRINT:tavg:LAST:"%5.1lf%s Last\l"')
            args.append('COMMENT:"{last_update}"'.format(**config))
            # yapf: enable
            openmediavault.mkrrdgraph.call_rrdtool_graph(args)
        return 0
