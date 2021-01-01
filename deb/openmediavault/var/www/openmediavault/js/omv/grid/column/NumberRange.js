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

/**
 * @ingroup webgui
 * @class OMV.grid.column.NumberRange
 * @derived Ext.grid.column.Column
 * @param invalidText The text to be used if the value is not a number or
 *   out of range.
 * @param minValue The minimum value. Defaults to undefined.
 * @param maxValue The maximum value. Defaults to undefined.
 */
Ext.define("OMV.grid.column.NumberRange", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.numberrangecolumn" ],

	invalidText: "-",
	minValue: undefined,
	maxValue: undefined,

	defaultRenderer: function(value) {
		// Is the value to be rendered a number?
		if (!Ext.isNumber(value))
			return this.invalidText;
		// Does it fit into the specified range?
		if (!(Ext.isNumber(this.minValue) && (value >= this.minValue)))
			return this.invalidText;
		if (!(Ext.isNumber(this.maxValue) && (value <= this.maxValue)))
			return this.invalidText;
		return Ext.String.htmlEncode(value);
	},

	updater: function(cell, value) {
		cell.firstChild.innerHTML = this.defaultRenderer(value);
	}
});
