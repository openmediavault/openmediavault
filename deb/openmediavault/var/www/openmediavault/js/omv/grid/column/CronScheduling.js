/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @ingroup webgui
 * @class OMV.grid.column.CronScheduling
 * @derived Ext.grid.column.Column
 */
Ext.define("OMV.grid.column.CronScheduling", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.cronscheduling" ],

	defaultRenderer: function(value, metaData, record) {
		var me = this;
		var execution = record.get("execution");
		if ("exactly" !== execution) {
			var mapExecution = {
				"hourly": _("Hourly"),
				"daily": _("Daily"),
				"weekly": _("Weekly"),
				"monthly": _("Monthly"),
				"yearly": _("Yearly"),
				"reboot": _("At reboot")
			};
			value = mapExecution[execution];
		} else {
			var minute = record.get("minute");
			var everynminute = record.get("everynminute");
			var hour = record.get("hour");
			var everynhour = record.get("everynhour");
			var dayofmonth = record.get("dayofmonth");
			var everyndayofmonth = record.get("everyndayofmonth");
			var month = record.get("month");
			var dayofweek = record.get("dayofweek");

			if (true === everynminute)
				minute = "*/" + minute;
			if (true === everynhour)
				hour = "*/" + hour;
			if (true === everyndayofmonth)
				dayofmonth = "*/" + dayofmonth;

			value = Ext.String.format("{0} {1} {2} {3} {4}",
			  minute, hour, dayofmonth, month, dayofweek);
		}
		return value;
	}
});
