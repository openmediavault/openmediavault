/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
// require("js/omv/util/Format.js")

/**
 * @ingroup webgui
 * @class OMV.grid.column.DeviceInfo
 * @derived Ext.grid.column.Column
 */
Ext.define("OMV.grid.column.DeviceInfo", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.deviceinfocolumn" ],
	requires: [
		"OMV.util.Format"
	],

	defaultRenderer: function(value, metaData, record) {
		var values = record.getData();
		var fields = {
			devicefile: {
				label: _("Device"),
				text: "{devicefile}"
			},
			model: {
				label: _("Model"),
				text: "{model}"
			},
			serialnumber: {
				label: _("S/N"),
				text: "{serialnumber}"
			},
			size: {
				label: _("Capacity"),
				text: "{[OMV.util.Format.binaryUnit(values.size)]}"
			}
		};
		var tplConfig = [];
		Ext.Object.each(fields, function(key, value) {
			// Check if the property exists.
			if (!Ext.Object.hasProperty(values, key))
				return;
			// Skip this property if its value is empty.
			if (Ext.isEmpty(values[key]))
				return;
			Ext.Array.push(tplConfig, Ext.String.format("{0}: {1}",
			  value.label, value.text));
		});
		var tpl = new Ext.XTemplate(tplConfig.join("<br/>"));
		return tpl.apply(record.getData());
	}
});
