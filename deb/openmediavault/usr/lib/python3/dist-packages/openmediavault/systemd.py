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
__all__ = ["SystemdException", "Properties", "Job", "Manager", "Unit"]

import dbus
import dbus.mainloop.glib

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)


class SystemdException(Exception):
    def __init__(self, error):
        super().__init__(
            "{}: {}".format(error.get_dbus_name(), error.get_dbus_message())
        )


class _Object:
    def __init__(self, object_path, interface_path):
        self._object_path = object_path
        self._interface_path = interface_path
        self._bus = dbus.SystemBus()
        self._proxy = self._bus.get_object(
            "org.freedesktop.systemd1", self._object_path
        )
        self._interface = dbus.Interface(self._proxy, self._interface_path)
        self._properties_interface = dbus.Interface(
            self._proxy, "org.freedesktop.DBus.Properties"
        )
        self._properties_interface.connect_to_signal(
            "PropertiesChanged", self._on_properties_changed
        )
        self._update_properties()

    @property
    def interface(self):
        return self._interface

    @property
    def properties_interface(self):
        return self._properties_interface

    # pylint: disable=unused-argument
    def _on_properties_changed(self, *args, **kargs):
        self._update_properties()

    def _update_properties(self):
        properties = self.properties_interface.GetAll(
            self.interface.dbus_interface
        )
        properties_attr = Properties()
        for key, value in properties.items():
            setattr(properties_attr, key, value)
        setattr(self, "properties", properties_attr)

    def __getitem__(self, property):
        """
        Get the given property from this object, e.g. 'LogLevel',
        'RequiredBy', 'LoadState' or 'ActiveState'.
        """
        # return self.properties_interface.Get(self._interface_path, property)
        return getattr(self.properties, property)


class Properties:
    pass


class Job(_Object):  # lgtm[py/missing-call-to-init]
    def __init__(self, job_path):
        super().__init__(job_path, "org.freedesktop.systemd1.Job")

    def cancel(self):
        try:
            self.interface.Cancel()
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)


class Manager(_Object):  # lgtm[py/missing-call-to-init]
    def __init__(self):
        super().__init__(
            "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager"
        )

    def halt(self):
        try:
            self.interface.Halt()
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def power_off(self):
        try:
            self.interface.PowerOff()
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def reboot(self):
        try:
            self.interface.Reboot()
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def reload(self):
        try:
            self.interface.Reload()
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def list_units(self):
        try:
            units = []
            for unit in self.interface.ListUnits():
                units.append(Unit(unit[6]))
            return tuple(units)
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def get_unit(self, name):
        try:
            unit_path = self.interface.GetUnit(name)
            unit = Unit(unit_path)
            return unit
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def start_unit(self, name, mode):
        try:
            job_path = self.interface.StartUnit(name, mode)
            job = Job(job_path)
            return job
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def stop_unit(self, name, mode):
        try:
            job_path = self.interface.StopUnit(name, mode)
            job = Job(job_path)
            return job
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)


class Unit(_Object):  # lgtm[py/missing-call-to-init]
    def __init__(self, unit_path):
        super().__init__(unit_path, "org.freedesktop.systemd1.Unit")

    def start(self, mode):
        try:
            job_path = self.interface.Start(mode)
            job = Job(job_path)
            return job
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def stop(self, mode):
        try:
            job_path = self.interface.Stop(mode)
            job = Job(job_path)
            return job
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def reload(self, mode):
        try:
            job_path = self.interface.Reload(mode)
            job = Job(job_path)
            return job
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    def restart(self, mode):
        try:
            job_path = self.interface.Restart(mode)
            job = Job(job_path)
            return job
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)

    @property
    def active(self):
        """
        :returns: Returns ``True`` if the unit is active, otherwise ``False``.
        :rtype: bool
        """
        return self['ActiveState'] == 'active'

    @property
    def running(self):
        """
        :returns: Returns ``True`` if the unit is running, otherwise ``False``.
        :rtype: bool
        """
        return self['SubState'] == 'running'

    def kill(self, who, mode, signal):
        try:
            self.interface.KillUnit(who, mode, signal)
        except dbus.exceptions.DBusException as e:
            raise SystemdException(e)
