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
 * @class OMV.grid.column.WhiteSpace
 * @derived Ext.grid.column.Column
 * @param mode Specifies how white-space is handled. This can be: normal,
 *   nowrap, pre, pre-line, pre-wrap or inherit. Defaults to 'normal'.
 */
Ext.define("OMV.grid.column.WhiteSpace", {
	extend: "Ext.grid.column.Column",
	alias: [ "widget.whitespacecolumn" ],

	mode: "normal",

	defaultRenderer: function(value) {
		var me = this;
		return OMV.util.Format.whitespace(value, me.mode);
	}
});
