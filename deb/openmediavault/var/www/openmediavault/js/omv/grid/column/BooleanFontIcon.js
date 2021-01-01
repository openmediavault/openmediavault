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
// require("js/omv/grid/column/FontIcon.js")

/**
 * @ingroup webgui
 * @class OMV.grid.column.BooleanFontIcon
 * @derived OMV.grid.column.FontIcon
 * @param falseCls The classname for the font icon in case of a FALSE
 *   condition. Defaults to 'mdi mdi-minus'.
 * @param trueCls The classname for the font icon in case of a TRUE
 *   condition. Defaults to 'mdi mdi-check'.
 */
Ext.define("OMV.grid.column.BooleanFontIcon", {
	extend: "OMV.grid.column.FontIcon",
	alias: ["widget.booleanfonticoncolumn"],

	trueValue: [true, 1, "true", "ok", "1", "y", "yes", "on"],
	trueCls: "mdi mdi-check",
	falseCls: "mdi mdi-minus",

	align: "center",

	getFontIconCls: function(value) {
		var me = this;
		var cls = [];
		if (Ext.Array.contains(me.trueValue, value)) {
			Ext.Array.push(cls, Ext.String.format("{0}grid-{1}-true",
				Ext.baseCSSPrefix, me.xtype));
			Ext.Array.push(cls, me.trueCls);
		} else {
			Ext.Array.push(cls, Ext.String.format("{0}grid-{1}-false",
				Ext.baseCSSPrefix, me.xtype));
			Ext.Array.push(cls, me.falseCls);
		}
		return cls;
	}
});
