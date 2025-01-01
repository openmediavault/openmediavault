#!/usr/bin/env python3
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
import glob
import os.path
import sys

import natsort
import openmediavault.confdbadm
import openmediavault.config.datamodel

import openmediavault


class Command(
    openmediavault.confdbadm.ICommand, openmediavault.confdbadm.CommandHelper
):
    @property
    def description(self):
        return "List all data model IDs."

    def execute(self, *args):
        datamodel_ids = []
        # Load all data models.
        datamodels_path = openmediavault.getenv(
            "OMV_DATAMODELS_DIR", "/usr/share/openmediavault/datamodels"
        )
        for f in glob.glob(os.path.join(datamodels_path, "conf.*.json")):
            datamodel_id = os.path.splitext(os.path.basename(f))[0]
            # Note, currently the filename is the data model id, but
            # this may change someday, so we load the data model and
            # ask for its identifier to be on the safe side.
            datamodel = openmediavault.config.Datamodel(datamodel_id)
            datamodel_ids.append(datamodel.id)
        # Print the data model identifiers.
        for datamodel_id in natsort.humansorted(datamodel_ids):
            print(datamodel_id)
        return 0


if __name__ == "__main__":
    sys.argv.insert(1, "list-ids")
    command = Command()
    rc = command.execute(*sys.argv)
    sys.exit(rc)
