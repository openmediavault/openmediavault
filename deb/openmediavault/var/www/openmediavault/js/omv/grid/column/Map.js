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
// require("js/omv/util/Format.js")

/**
 * @ingroup webgui
 * @class OMV.grid.column.Map
 * @derived Ext.grid.column.Column
 * @param mapItems The object containing the key/value pairs used to map
 *   the value to be rendered or an array. Defaults to empty object.
 * @param undefinedText The text displayed if the value to be rendered
 *   does not exist in the \em mapItems config property.
 */
Ext.define("OMV.grid.column.Map", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.mapcolumn" ],

	mapItems: {},
	undefinedText: "&#160;",

	defaultRenderer: function(value) {
		var me = this;
		var result = me.undefinedText;
		if (Ext.isObject(me.mapItems)) {
			if (Ext.isDefined(me.mapItems[value]))
				result = me.mapItems[value];
		} else if (Ext.isArray(me.mapItems)) {
			Ext.Array.each(me.mapItems, function(item) {
				if (item[0] == value) {
					result = item[1];
					return false;
				}
			});
		}
		return result;
	}
});
