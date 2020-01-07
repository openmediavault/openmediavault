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
                'title_uptime': 'System uptime',
                'color_uptime_current': '#f17742',  # orange
                'color_uptime_max': '#ff1300',  # red
                'color_uptime_min': '#ffdb70',  # yellow
                'color_uptime_avg': '#76d6ff',  # blue
            }
        )
        args = []
        # yapf: disable
        # pylint: disable=line-too-long
        args.append('{image_dir}/uptime-{period}.png'.format(**config))
        args.extend(config['defaults'])
        args.extend(['--start', config['start']])
        args.extend(['--title', '"{title_uptime}{title_by_period}"'.format(**config)])
        args.append('--slope-mode')
        args.extend(['--lower-limit', '0'])
        args.append('--rigid')
        args.extend(['--vertical-label', 'Days'])
        # Based on https://mailman.verplant.org/pipermail/collectd/2010-June/003898.html
        args.append('DEF:uptime_sec_avg={data_dir}/uptime/uptime.rrd:value:AVERAGE'.format(**config))
        args.append('DEF:uptime_sec_max={data_dir}/uptime/uptime.rrd:value:MAX'.format(**config))
        args.append('CDEF:uptime_no_unkn=uptime_sec_max,UN,0,uptime_sec_max,IF')
        args.append('CDEF:uptime_peaks=uptime_no_unkn,PREV\(uptime_no_unkn\),LT,PREV\(uptime_no_unkn\),UNKN,IF')
        args.append('VDEF:minimum_uptime_secs=uptime_peaks,MINIMUM')
        args.append('CDEF:minimum_uptime_graph=uptime_sec_max,minimum_uptime_secs,EQ,uptime_sec_max,86400,/,0,IF')
        args.append('CDEF:minimum_uptime_days=uptime_sec_max,minimum_uptime_secs,EQ,uptime_sec_max,86400,/,FLOOR,0,IF')
        args.append('CDEF:minimum_uptime_hours=uptime_sec_max,minimum_uptime_secs,EQ,uptime_sec_max,86400,%,3600,/,FLOOR,0,IF')
        args.append('CDEF:minimum_uptime_mins=uptime_sec_max,minimum_uptime_secs,EQ,uptime_sec_max,86400,%,3600,%,60,/,FLOOR,0,IF')
        args.append('VDEF:min_uptime_graph=minimum_uptime_graph,MAXIMUM')
        args.append('VDEF:min_uptime_days=minimum_uptime_days,MAXIMUM')
        args.append('VDEF:min_uptime_hours=minimum_uptime_hours,MAXIMUM')
        args.append('VDEF:min_uptime_mins=minimum_uptime_mins,MAXIMUM')
        args.append('VDEF:maximum_uptime_secs=uptime_sec_max,MAXIMUM')
        args.append('CDEF:maximum_uptime_graph=uptime_sec_max,maximum_uptime_secs,EQ,uptime_sec_max,86400,/,0,IF')
        args.append('CDEF:maximum_uptime_days=uptime_sec_max,maximum_uptime_secs,EQ,uptime_sec_max,86400,/,FLOOR,0,IF')
        args.append('CDEF:maximum_uptime_hours=uptime_sec_max,maximum_uptime_secs,EQ,uptime_sec_max,86400,%,3600,/,FLOOR,0,IF')
        args.append('CDEF:maximum_uptime_mins=uptime_sec_max,maximum_uptime_secs,EQ,uptime_sec_max,86400,%,3600,%,60,/,FLOOR,0,IF')
        args.append('VDEF:max_uptime_graph=maximum_uptime_graph,MAXIMUM')
        args.append('VDEF:max_uptime_days=maximum_uptime_days,MAXIMUM')
        args.append('VDEF:max_uptime_hours=maximum_uptime_hours,MAXIMUM')
        args.append('VDEF:max_uptime_mins=maximum_uptime_mins,MAXIMUM')
        args.append('VDEF:average_uptime_secs=uptime_sec_max,AVERAGE')
        args.append('CDEF:average_uptime_graph=uptime_sec_max,POP,average_uptime_secs,86400,/')
        args.append('CDEF:average_uptime_days=uptime_sec_max,POP,average_uptime_secs,86400,/,FLOOR')
        args.append('CDEF:average_uptime_hours=uptime_sec_max,POP,average_uptime_secs,86400,%,3600,/,FLOOR')
        args.append('CDEF:average_uptime_mins=uptime_sec_max,POP,average_uptime_secs,86400,%,3600,%,60,/,FLOOR')
        args.append('VDEF:avg_uptime_days=average_uptime_days,LAST')
        args.append('VDEF:avg_uptime_hours=average_uptime_hours,LAST')
        args.append('VDEF:avg_uptime_mins=average_uptime_mins,LAST')
        args.append('CDEF:current_uptime_graph=uptime_sec_max,86400,/')
        args.append('CDEF:current_uptime_days=uptime_sec_max,86400,/,FLOOR')
        args.append('CDEF:current_uptime_hours=uptime_sec_max,86400,%,3600,/,FLOOR')
        args.append('CDEF:current_uptime_mins=uptime_sec_max,86400,%,3600,%,60,/,FLOOR')
        args.append('VDEF:curr_uptime_days=current_uptime_days,LAST')
        args.append('VDEF:curr_uptime_hours=current_uptime_hours,LAST')
        args.append('VDEF:curr_uptime_mins=current_uptime_mins,LAST')
        args.append('CDEF:time=uptime_sec_max,POP,TIME')
        args.append('VDEF:start=time,FIRST')
        args.append('VDEF:last=time,LAST')
        args.append('CDEF:time_window=uptime_sec_max,UN,0,uptime_sec_max,IF,POP,TIME')
        args.append('CDEF:time_window2=PREV\(time_window\)')
        args.append('VDEF:window_start=time_window,FIRST')
        args.append('VDEF:window_last=time_window,LAST')
        args.append('CDEF:delta=uptime_sec_max,POP,window_last,window_start,-')
        args.append('CDEF:system_on_un=uptime_sec_avg,UN,UNKN,1,IF')
        args.append('VDEF:total_uptime_secs=system_on_un,TOTAL')
        args.append('CDEF:total_uptime_days=uptime_sec_max,POP,total_uptime_secs,86400,/,FLOOR')
        args.append('CDEF:total_uptime_hours=uptime_sec_max,POP,total_uptime_secs,86400,%,3600,/,FLOOR')
        args.append('CDEF:total_uptime_mins=uptime_sec_max,POP,total_uptime_secs,86400,%,3600,%,60,/,FLOOR')
        args.append('VDEF:tot_uptime_days=total_uptime_days,LAST')
        args.append('VDEF:tot_uptime_hours=total_uptime_hours,LAST')
        args.append('VDEF:tot_uptime_mins=total_uptime_mins,LAST')
        args.append('CDEF:temp_perc_on=uptime_sec_max,POP,total_uptime_secs,delta,/,100,*')
        args.append('VDEF:new_perc_on=temp_perc_on,LAST')
        args.append('AREA:current_uptime_graph#66666640')
        args.append('LINE1:current_uptime_graph{color_uptime_current}:Current'.format(**config))
        args.append('GPRINT:curr_uptime_days:"%5.0lf days"')
        args.append('GPRINT:curr_uptime_hours:"%3.0lf hours"')
        args.append('GPRINT:curr_uptime_mins:"%3.0lf mins"')
        args.append('GPRINT:curr_uptime_mins:"  %T %x\l":strftime')
        args.append('LINE1:max_uptime_graph{color_uptime_max}:Maximum:dashes'.format(**config))
        args.append('GPRINT:max_uptime_days:"%5.0lf days"')
        args.append('GPRINT:max_uptime_hours:"%3.0lf hours"')
        args.append('GPRINT:max_uptime_mins:"%3.0lf mins"')
        args.append('GPRINT:max_uptime_mins:"  %T %x\l":strftime')
        args.append('HRULE:min_uptime_graph{color_uptime_min}:Minimum:dashes'.format(**config))
        args.append('GPRINT:min_uptime_days:"%5.0lf days"')
        args.append('GPRINT:min_uptime_hours:"%3.0lf hours"')
        args.append('GPRINT:min_uptime_mins:"%3.0lf mins"')
        args.append('GPRINT:min_uptime_mins:"  %T %x\l":strftime')
        args.append('LINE1:average_uptime_graph{color_uptime_avg}:Average:dashes'.format(**config))
        args.append('GPRINT:avg_uptime_days:"%5.0lf days"')
        args.append('GPRINT:avg_uptime_hours:"%3.0lf hours"')
        args.append('GPRINT:avg_uptime_mins:"%3.0lf mins"')
        args.append('GPRINT:avg_uptime_mins:"  %T %x\l":strftime')
        args.append('COMMENT:"  Total  "')
        args.append('GPRINT:tot_uptime_days:"%5.0lf days"')
        args.append('GPRINT:tot_uptime_hours:"%3.0lf hours"')
        args.append('GPRINT:tot_uptime_mins:"%3.0lf mins"')
        args.append('GPRINT:new_perc_on:"  %3.2lf%% up\l"')
        args.append('COMMENT:"{last_update}"'.format(**config))
        # yapf: enable
        openmediavault.mkrrdgraph.call_rrdtool_graph(args)
        return 0
