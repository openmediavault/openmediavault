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
import os

import openmediavault.config
import openmediavault.device
import openmediavault.stringutils
from salt.utils.decorators.jinja import jinja_filter


@jinja_filter('omv_conf_get')
def get(id_, identifier=None):
    """
    Get the specified configuration object.
    :param id_: The data model identifier, e.g. 'conf.service.ftp'.
    :param identifier: The identifier of the configuration object, e.g.
        the UUID. Defaults to None.
    :returns: Depending on the configuration object and whether *identifier*
        is set, a list of configuration objects or a single object is
        returned.
    """
    db = openmediavault.config.Database()
    objs = db.get(id_, identifier)
    if isinstance(objs, list):
        return [obj.get_dict() for obj in objs]
    return objs.get_dict()


@jinja_filter('omv_conf_get_by_identifier')
def get_by_identifier(identifier, id_):
    """
    Get the specified configuration object.

    Jinja example:
    Get the UUID of the mount point configuration object that is
    associated with the given shared folder.

    'xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx' | omv_conf_get_by_identifier('conf.system.sharedfolder') | attr('mntentref')

    :param identifier: The identifier of the configuration object, e.g.
        the UUID.
    :param id_: The data model identifier, e.g. 'conf.service.ftp'.
    :returns: A single configuration object is returned.
    """
    return get(id_, identifier)


def get_by_filter(id_, filter_, **kwargs):
    """
    Get the iterable configuration objects that are matching the specified
    constraints.
    :param id_: The data model identifier, e.g. 'conf.service.ftp'.
    :param filter_: A filter specifying constraints on the objects
        to retrieve.
        ``
        Example 1:
        {
            "operator": "stringEquals",
            "arg0": "fsname",
            "arg1": "xyz"
        }
        Example 2:
        {
            "operator": "and",
            "arg0": {
                "operator": "stringEquals",
                "arg0": "type",
                "arg1": "bond"
            },
            "arg1": {
                "operator": "stringEquals",
                "arg0": "devicename",
                "arg1": "bond0"
            }
        }
        ``
    :param min_result: The minimum number of objects that are expected.
    :param max_result: The maximum number of objects that are expected.
    :returns: A list containing the requested configuration objects.
        If *max_result* is set to 1, then the first found object
        is returned. In this case the method does not return a
        list of configuration objects.
    """
    db = openmediavault.config.Database()
    objs = db.get_by_filter(
        id_, openmediavault.config.DatabaseFilter(filter_), **kwargs
    )
    if isinstance(objs, list):
        return [obj.get_dict() for obj in objs]
    return objs.get_dict()


def get_sharedfolder_path(uuid):
    """
    Get the prettified absolute path of the given shared folder.
    :param uuid: The UUID of the shared folder configuration object.
    :type uuid: str
    :return: Returns the prettified absolute path of the shared folder.
        A '/' is automatically appended to the end.
    :rtype: str
    """
    sf_obj = get('conf.system.sharedfolder', uuid)
    mp_obj = get('conf.system.filesystem.mountpoint', sf_obj['mntentref'])
    return openmediavault.stringutils.path_prettify(
        # !!! Attention !!! If a component is an absolute path, all previous
        # components are thrown away and joining continues from the absolute
        # path component.
        os.path.join(mp_obj['dir'], sf_obj['reldirpath'].lstrip(os.sep))
    )


def get_sharedfolder_name(uuid):
    """
    Get the name of the given shared folder.
    :param uuid: The UUID of the shared folder configuration object.
    :type uuid: str
    :return: Returns the name of the shared folder.
    :rtype: str
    """
    sf_obj = get('conf.system.sharedfolder', uuid)
    return sf_obj['name']


def get_sharedfolder_mount_path(uuid):
    """
    Get the prettified mount path of the given shared folder.
    :param uuid: The UUID of the shared folder configuration object.
    :type uuid: str
    :return: Returns the prettified mount path of the shared folder.
        A '/' is automatically appended to the end.
    :rtype: str
    """
    sf_obj = get('conf.system.sharedfolder', uuid)
    mp_obj = get('conf.system.filesystem.mountpoint', sf_obj['mntentref'])
    return openmediavault.stringutils.path_prettify(mp_obj['dir'])
