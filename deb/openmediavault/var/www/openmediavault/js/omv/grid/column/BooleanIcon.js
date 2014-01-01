/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
 * @class OMV.grid.column.BooleanIcon
 * @derived Ext.grid.column.Column
 */
Ext.define("OMV.grid.column.BooleanIcon", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.booleaniconcolumn" ],
	uses: [
		"OMV.util.Format"
	],

	trueIcon: "yes.png",
	falseIcon: "no.png",
	undefinedText: "&#160;",

	defaultRenderer: function(value, metaData) {
		var me = this;
		metaData.tdAttr = 'style="vertical-align: middle;"';
		if(value === undefined)
			return me.undefinedText;
		var img = me.trueIcon;
		if(!value || value === "false") {
			img = me.falseIcon;
		}
		return "<img border='0' src='images/" + img + "' alt='" +
		  OMV.util.Format.boolean(value) + "'>";
	}
});
