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
 * @class OMV.grid.column.BooleanText
 * @derived Ext.grid.column.Column
 */
Ext.define("OMV.grid.column.BooleanText", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.booleantextcolumn" ],

	trueValue: [ true, 1, "true", "ok", "1", "y", "yes", "on" ],
	trueText: _("Yes"),
	falseText: _("No"),

	defaultRenderer: function(value) {
		var me = this;
		return Ext.Array.contains(me.trueValue, value) ?
		  me.trueText : me.falseText;
	},

	updater: function(cell, value, record, view, dataSource) {
		cell.firstChild.innerHTML = this.defaultRenderer(value, null);
	}
});
