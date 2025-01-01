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
import re
import copy


class Plugin(openmediavault.mkrrdgraph.IPlugin):
    def create_graph(self, config):
        # http://paletton.com/#uid=33r0-0kwi++bu++hX++++rd++kX
        config.update(
            {
                'title_nut_charge': 'UPS charge',
                'color_nut_charge': '#0000fd',  # blue
                'title_nut_load': 'UPS load',
                'color_nut_load': '#0000fd',  # blue
                'title_nut_temperature_battery': 'UPS battery temperature',
                'color_nut_temperature_battery': '#ff0000',  # red
                'title_nut_temperature': 'UPS temperature',
                'color_nut_temperature': '#ff0000',  # red
                'title_nut_voltage': 'UPS voltage',
                'color_nut_voltage_battery': '#ffbf00',  # yellow
                'color_nut_voltage_input': '#0bb6ff',  # blue
                'color_nut_voltage_output': '#ff1300',  # red
            }
        )
        # Get the UPS' from the collectd configuration file.
        # Note, we assume that only ONE UPS is configured.
        # Examples:
        # upsname@hostname[:port]
        # myups@localhost:3493
        ups_configs = openmediavault.mkrrdgraph.load_collectd_config(
            'nut', 'UPS'
        )
        for ups_config in ups_configs:
            # Get the configuration name of the UPS.
            m = re.match(r'^(\S+)@([^\s:]+)(:(\d+))?$', ups_config)
            if not m:
                continue
            config['upsname'] = m.group(1)
            hostname = m.group(2)
            if hostname != 'localhost':
                config = copy.deepcopy(config) # Do not affect original data_dir
                config['data_dir'] = os.path.abspath(os.path.join(
                    config['data_dir'], os.pardir, hostname))

            image_filename = '{image_dir}/nut-charge-{period}.png'.format(
                **config
            )
            if os.path.exists(
                '{data_dir}/nut-{upsname}/percent-charge.rrd'.format(**config)
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_nut_charge}{title_by_period}"'.format(
                            **config
                        ),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--upper-limit', '100'])
                args.extend(['--lower-limit', '0'])
                args.append('--rigid')
                args.extend(['--vertical-label', 'Percent'])
                args.append(
                    'DEF:avg={data_dir}/nut-{upsname}/percent-charge.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:min={data_dir}/nut-{upsname}/percent-charge.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:max={data_dir}/nut-{upsname}/percent-charge.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'LINE1:avg{color_nut_charge}:"Charge"'.format(**config)
                )
                args.append('GPRINT:min:MIN:"%4.2lf Min"')
                args.append('GPRINT:avg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:max:MAX:"%4.2lf Max"')
                args.append('GPRINT:avg:LAST:"%4.2lf Last\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            else:
                openmediavault.mkrrdgraph.copy_placeholder_image(image_filename)

            image_filename = '{image_dir}/nut-load-{period}.png'.format(
                **config
            )
            if os.path.exists(
                '{data_dir}/nut-{upsname}/percent-load.rrd'.format(**config)
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_nut_load}{title_by_period}"'.format(**config),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--upper-limit', '100'])
                args.extend(['--lower-limit', '0'])
                args.append('--rigid')
                args.extend(['--vertical-label', 'Percent'])
                args.append(
                    'DEF:avg={data_dir}/nut-{upsname}/percent-load.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:min={data_dir}/nut-{upsname}/percent-load.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:max={data_dir}/nut-{upsname}/percent-load.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append('LINE1:avg{color_nut_load}:"Load"'.format(**config))
                args.append('GPRINT:min:MIN:"%4.2lf Min"')
                args.append('GPRINT:avg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:max:MAX:"%4.2lf Max"')
                args.append('GPRINT:avg:LAST:"%4.2lf Last\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            else:
                openmediavault.mkrrdgraph.copy_placeholder_image(image_filename)

            # Note, if the UPS supports battery.temperature and ups.temperature,
            # the battery temperature will be used by default because it is
            # the more important value. Only one temperature can be displayed
            # because the WebGUI does not display multiple temperature
            # informations.
            image_filename = '{image_dir}/nut-temperature-{period}.png'.format(
                **config
            )
            if os.path.exists(
                '{data_dir}/nut-{upsname}/temperature-battery.rrd'.format(
                    **config
                )
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_nut_temperature_battery}{title_by_period}"'.format(
                            **config
                        ),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--lower-limit', '0'])
                args.extend(['--vertical-label', 'Celsius'])
                args.append(
                    'DEF:avg={data_dir}/nut-{upsname}/temperature-battery.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:min={data_dir}/nut-{upsname}/temperature-battery.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:max={data_dir}/nut-{upsname}/temperature-battery.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'LINE1:avg{color_nut_temperature_battery}:"Temperature"'.format(
                        **config
                    )
                )
                args.append('GPRINT:min:MIN:"%4.2lf Min"')
                args.append('GPRINT:avg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:max:MAX:"%4.2lf Max"')
                args.append('GPRINT:avg:LAST:"%4.2lf Last\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            elif os.path.exists(
                '{data_dir}/nut-{upsname}/temperature-ups.rrd'.format(**config)
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_nut_temperature}{title_by_period}"'.format(
                            **config
                        ),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--lower-limit', '0'])
                args.extend(['--vertical-label', 'Celsius'])
                args.append(
                    'DEF:avg={data_dir}/nut-{upsname}/temperature-ups.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:min={data_dir}/nut-{upsname}/temperature-ups.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:max={data_dir}/nut-{upsname}/temperature-ups.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'LINE1:avg{color_nut_temperature}:"Temperature"'.format(
                        **config
                    )
                )
                args.append('GPRINT:min:MIN:"%4.2lf Min"')
                args.append('GPRINT:avg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:max:MAX:"%4.2lf Max"')
                args.append('GPRINT:avg:LAST:"%4.2lf Last\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            else:
                openmediavault.mkrrdgraph.copy_placeholder_image(image_filename)

            image_filename = '{image_dir}/nut-voltage-{period}.png'.format(
                **config
            )
            if (
                os.path.exists(
                    '{data_dir}/nut-{upsname}/voltage-battery.rrd'.format(
                        **config
                    )
                )
                and os.path.exists(
                    '{data_dir}/nut-{upsname}/voltage-input.rrd'.format(
                        **config
                    )
                )
                and os.path.exists(
                    '{data_dir}/nut-{upsname}/voltage-output.rrd'.format(
                        **config
                    )
                )
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_nut_voltage}{title_by_period}"'.format(
                            **config
                        ),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--lower-limit', '0'])
                args.extend(['--vertical-label', 'Volt'])
                args.append(
                    'DEF:bavg={data_dir}/nut-{upsname}/voltage-battery.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:bmin={data_dir}/nut-{upsname}/voltage-battery.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:bmax={data_dir}/nut-{upsname}/voltage-battery.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:iavg={data_dir}/nut-{upsname}/voltage-input.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:imin={data_dir}/nut-{upsname}/voltage-input.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:imax={data_dir}/nut-{upsname}/voltage-input.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:oavg={data_dir}/nut-{upsname}/voltage-output.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:omin={data_dir}/nut-{upsname}/voltage-output.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:omax={data_dir}/nut-{upsname}/voltage-output.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'LINE1:bavg{color_nut_voltage_battery}:"Battery"'.format(
                        **config
                    )
                )
                args.append('GPRINT:bmin:MIN:"%4.2lf Min"')
                args.append('GPRINT:bavg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:bmax:MAX:"%4.2lf Max"')
                args.append('GPRINT:bavg:LAST:"%4.2lf Last\l"')
                args.append(
                    'LINE1:iavg{color_nut_voltage_input}:"Input"'.format(
                        **config
                    )
                )
                args.append('GPRINT:imin:MIN:"%4.2lf Min"')
                args.append('GPRINT:iavg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:imax:MAX:"%4.2lf Max"')
                args.append('GPRINT:iavg:LAST:"%4.2lf Last\l"')
                args.append(
                    'LINE1:oavg{color_nut_voltage_output}:"Output"'.format(
                        **config
                    )
                )
                args.append('GPRINT:omin:MIN:"%4.2lf Min"')
                args.append('GPRINT:oavg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:omax:MAX:"%4.2lf Max"')
                args.append('GPRINT:oavg:LAST:"%4.2lf Last\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            elif os.path.exists(
                '{data_dir}/nut-{upsname}/voltage-battery.rrd'.format(**config)
            ) and os.path.exists(
                '{data_dir}/nut-{upsname}/voltage-input.rrd'.format(**config)
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_nut_voltage}{title_by_period}"'.format(
                            **config
                        ),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--lower-limit', '0'])
                args.extend(['--vertical-label', 'Volt'])
                args.append(
                    'DEF:bavg={data_dir}/nut-{upsname}/voltage-battery.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:bmin={data_dir}/nut-{upsname}/voltage-battery.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:bmax={data_dir}/nut-{upsname}/voltage-battery.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:iavg={data_dir}/nut-{upsname}/voltage-input.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:imin={data_dir}/nut-{upsname}/voltage-input.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:imax={data_dir}/nut-{upsname}/voltage-input.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'LINE1:bavg{color_nut_voltage_battery}:"Battery"'.format(
                        **config
                    )
                )
                args.append('GPRINT:bmin:MIN:"%4.2lf Min"')
                args.append('GPRINT:bavg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:bmax:MAX:"%4.2lf Max"')
                args.append('GPRINT:bavg:LAST:"%4.2lf Last\l"')
                args.append(
                    'LINE1:iavg{color_nut_voltage_input}:"Input"'.format(
                        **config
                    )
                )
                args.append('GPRINT:imin:MIN:"%4.2lf Min"')
                args.append('GPRINT:iavg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:imax:MAX:"%4.2lf Max"')
                args.append('GPRINT:iavg:LAST:"%4.2lf Last\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            elif os.path.exists(
                '{data_dir}/nut-{upsname}/voltage-input.rrd'.format(**config)
            ) and os.path.exists(
                '{data_dir}/nut-{upsname}/voltage-output.rrd'.format(**config)
            ):
                args = []
                args.append(image_filename)
                args.extend(config['defaults'])
                args.extend(['--start', config['start']])
                args.extend(
                    [
                        '--title',
                        '"{title_nut_voltage}{title_by_period}"'.format(
                            **config
                        ),
                    ]
                )
                args.append('--slope-mode')
                args.extend(['--lower-limit', '0'])
                args.extend(['--vertical-label', 'Volt'])
                args.append(
                    'DEF:iavg={data_dir}/nut-{upsname}/voltage-input.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:imin={data_dir}/nut-{upsname}/voltage-input.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:imax={data_dir}/nut-{upsname}/voltage-input.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:oavg={data_dir}/nut-{upsname}/voltage-output.rrd:value:AVERAGE'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:omin={data_dir}/nut-{upsname}/voltage-output.rrd:value:MIN'.format(
                        **config
                    )
                )
                args.append(
                    'DEF:omax={data_dir}/nut-{upsname}/voltage-output.rrd:value:MAX'.format(
                        **config
                    )
                )
                args.append(
                    'LINE1:iavg{color_nut_voltage_input}:"Input"'.format(
                        **config
                    )
                )
                args.append('GPRINT:imin:MIN:"%4.2lf Min"')
                args.append('GPRINT:iavg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:imax:MAX:"%4.2lf Max"')
                args.append('GPRINT:iavg:LAST:"%4.2lf Last\l"')
                args.append(
                    'LINE1:oavg{color_nut_voltage_output}:"Output"'.format(
                        **config
                    )
                )
                args.append('GPRINT:omin:MIN:"%4.2lf Min"')
                args.append('GPRINT:oavg:AVERAGE:"%4.2lf Avg"')
                args.append('GPRINT:omax:MAX:"%4.2lf Max"')
                args.append('GPRINT:oavg:LAST:"%4.2lf Last\l"')
                args.append('COMMENT:"{last_update}"'.format(**config))
                openmediavault.mkrrdgraph.call_rrdtool_graph(args)
            else:
                openmediavault.mkrrdgraph.copy_placeholder_image(image_filename)
        return 0
