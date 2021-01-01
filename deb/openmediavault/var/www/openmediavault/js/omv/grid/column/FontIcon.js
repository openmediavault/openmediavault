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
 * @class OMV.grid.column.FontIcon
 * @derived Ext.grid.column.Column
 * @param extraIconCls An optional extra CSS class that will be added to
 *   the font icon. Defaults to ''.
 * @param getFontIconCls The callback function that is called to get the
 *   classnames for font icon to be rendered in the column cell.
 *   The callback function should return a string or an array of CSS
 *   classes.
 */
Ext.define("OMV.grid.column.FontIcon", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.fonticoncolumn" ],

	extraIconCls: "",
	getFontIconCls: Ext.emptyFn(),

	defaultRenderer: function(value, metaData) {
		var me = this;
		var cls = [me.extraIconCls];
		if (Ext.isObject(metaData)) {
			metaData.tdCls = Ext.String.format("{0}grid-cell-{1}-td",
				Ext.baseCSSPrefix, me.xtype);
		}
		Ext.Array.push(cls, Ext.String.format("{0}grid-{1}",
			Ext.baseCSSPrefix, me.xtype));
		var fontIconCls = me.getFontIconCls.apply(me, arguments);
		if (!Ext.isArray(fontIconCls))
			fontIconCls = [fontIconCls];
		cls = Ext.Array.merge(cls, fontIconCls);
		var tpl = new Ext.XTemplate("<i class='{cls}'></i>");
		return tpl.apply({cls: cls.join(" ")});
	},

	updater: function(cell, value, record, view, dataSource) {
		cell.firstChild.innerHTML = this.defaultRenderer(value, null);
	}
});
