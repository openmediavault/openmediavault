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

import logging

import openmediavault.productinfo
import openmediavault.settings
import yaml

log = logging.getLogger(__name__)


def populate_pillar():
    """
    Populate the pillar data.
    """
    # Create the pillar containing the default values. Read them from the
    # file /etc/default/openmediavault.
    data = openmediavault.settings.Environment.as_dict()
    filename = '/srv/pillar/omv/default.sls'
    log.info('Writing {}'.format(filename))
    with open(filename, 'w') as fd:
        fd.write('{% raw %}\n')
        fd.write(
            yaml.dump(
                {'default': data},
                Dumper=yaml.SafeDumper,
                default_flow_style=False,
            )
        )
        fd.write('{% endraw %}\n')
    # Create the pillar containing the product information.
    prod_info = openmediavault.productinfo.ProductInfo()
    data = prod_info.as_dict()
    filename = '/srv/pillar/omv/productinfo.sls'
    log.info('Writing {}'.format(filename))
    with open(filename, 'w') as fd:
        fd.write(
            yaml.dump(
                {'productinfo': data},
                Dumper=yaml.SafeDumper,
                default_flow_style=False,
            )
        )
    return True
