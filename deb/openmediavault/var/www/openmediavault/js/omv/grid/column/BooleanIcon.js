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
 * @class OMV.grid.column.BooleanIcon
 * @derived Ext.grid.column.Column
 * @param iconCls A CSS class used to render the boolean value. This can
 *   be for example:<ul>
 *   \li x-grid-cell-booleaniconcolumn-yesno
 *   \li x-grid-cell-booleaniconcolumn-led
 *   \li x-grid-cell-booleaniconcolumn-switch
 *   </ul>
 *   Defaults to 'x-grid-cell-booleaniconcolumn-yesno'.
 */
Ext.define("OMV.grid.column.BooleanIcon", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.booleaniconcolumn" ],

	iconBaseCls: Ext.baseCSSPrefix + "grid-cell-booleaniconcolumn",
	iconCls: Ext.baseCSSPrefix + "grid-cell-booleaniconcolumn-yesno",
	undefinedText: "&#160;",
	trueValue: [ true, 1, "true", "ok", "1", "y", "yes", "on" ],

	align: "center",

	defaultRenderer: function(value, metaData) {
		var me = this;
		var iconCls = me.iconCls + "-false";
		if (!Ext.isDefined(value))
			return me.undefinedText;
		if (Ext.Array.contains(me.trueValue, value))
			iconCls = me.iconCls + "-true";
		metaData.tdCls = Ext.String.format("{0} {1}",
		  me.iconBaseCls, iconCls);
		return "";
	},

	updater: function(cell, value) {
		var me = this;
		var metaData = {};
		cell.firstChild.innerHTML = me.defaultRenderer(value, metaData);
		if (metaData.tdCls)
			Ext.fly(cell).addCls(metaData.tdCls);
	}
});
