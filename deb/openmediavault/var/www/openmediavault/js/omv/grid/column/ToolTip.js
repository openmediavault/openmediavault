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
 * @class OMV.grid.column.ToolTip
 * @derived Ext.grid.column.Column
 */
Ext.define("OMV.grid.column.ToolTip", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.tooltipcolumn" ],

	getTooltipText: function(value, record) {
		return value;
	},

	defaultRenderer: function(value, metaData, record) {
		var me = this;
		var text = me.getTooltipText(value, record);
		if (!Ext.isEmpty(text)) {
			metaData.tdAttr = Ext.String.format("data-qtip='{0}'",
			  Ext.String.htmlEncode(text));
		}
		return Ext.String.htmlEncode(value);
	},

	updater: function(cell, value) {
		var me = this;
        var metaData = {};
        cell.firstChild.innerHTML = me.defaultRenderer(value, metaData);
        if (metaData.tdCls)
            Ext.fly(cell).addCls(metaData.tdCls);
    }
});
