# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2016 Volker Theile
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
import dbus
import dbus.mainloop.glib
import openmediavault as omv

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

class SystemdException(Exception):
	def __init__(self, error):
		super().__init__("{}: {}".format(error.get_dbus_name(),
			error.get_dbus_message()))

class SystemdObject(object):
	def __init__(self, object_path, interface_path):
		self.__object_path = object_path
		self.__interface_path = interface_path
		self.__bus = dbus.SystemBus()
		self.__proxy = self.__bus.get_object(
			"org.freedesktop.systemd1",
			self.__object_path)
		self.__interface = dbus.Interface(
			self.__proxy,
			self.__interface_path)
		self.__properties_interface = dbus.Interface(
			self.__proxy,
			"org.freedesktop.DBus.Properties")
		self.__properties_interface.connect_to_signal(
			"PropertiesChanged",
			self.__on_properties_changed)
		self.__update_properties()

	@property
	def interface(self):
		return self.__interface

	@property
	def properties_interface(self):
		return self.__properties_interface

	def __on_properties_changed(self, *args, **kargs):
		self.__update_properties()

	def __update_properties(self):
		properties = self.properties_interface.GetAll(
			self.interface.dbus_interface)
		for key, value in properties.items():
			key = omv.string.camelcase_to_underscore(key)
			setattr(self, key, value)

	def get_property(self, name):
		"""
		Get the object property by its real name.
		"""
		return self.properties_interface.Get(self.__interface_path, name)

class Job(SystemdObject):
	def __init__(self, job_path):
		super().__init__(job_path, "org.freedesktop.systemd1.Job")

	def cancel(self):
		try:
			self.interface.Cancel()
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

class Manager(SystemdObject):
	def __init__(self):
		super().__init__("/org/freedesktop/systemd1",
			"org.freedesktop.systemd1.Manager")

	def halt(self):
		try:
			self.interface.Halt()
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def power_off(self):
		try:
			self.interface.PowerOff()
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def reboot(self):
		try:
			self.interface.Reboot()
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def reload(self):
		try:
			self.interface.Reload()
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def list_units(self):
		try:
			units = []
			for unit in self.interface.ListUnits():
				units.append(Unit(unit[6]))
			return tuple(units)
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def get_unit(self, name):
		try:
			unit_path = self.interface.GetUnit(name)
			unit = Unit(unit_path)
			return unit
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def start_unit(self, name, mode):
		try:
			job_path = self.interface.StartUnit(name, mode)
			job = Job(job_path)
			return job
		except dbus.exceptions.DBusException as e:
			raise SystemdError(e)

	def stop_unit(self, name, mode):
		try:
			job_path = self.interface.StopUnit(name, mode)
			job = Job(job_path)
			return job
		except dbus.exceptions.DBusException as e:
			raise SystemdError(e)

class Unit(SystemdObject):
	def __init__(self, unit_path):
		super().__init__(unit_path, "org.freedesktop.systemd1.Unit")

	def start(self, mode):
		try:
			job_path = self.interface.Start(mode)
			job = Job(job_path)
			return job
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def stop(self, mode):
		try:
			job_path = self.interface.Stop(mode)
			job = Job(job_path)
			return job
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def reload(self, mode):
		try:
			job_path = self.interface.Reload(mode)
			job = Job(job_path)
			return job
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def restart(self, mode):
		try:
			job_path = self.interface.Restart(mode)
			job = Job(job_path)
			return job
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)

	def kill(self, who, mode, signal):
		try:
			self.interface.KillUnit(who, mode, signal)
		except dbus.exceptions.DBusException as e:
			raise SystemdException(error)
