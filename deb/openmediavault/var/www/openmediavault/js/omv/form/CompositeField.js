/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
 * @class OMV.form.CompositeField
 * @derived Ext.form.FieldContainer
 */
Ext.define("OMV.form.CompositeField", {
	extend: "Ext.form.FieldContainer",
	alias: "widget.compositefield",
	uses: [
		"Ext.layout.container.HBox"
	],

	isComposite: true,
	combineErrors: true,
	layout: "hbox",
	baseDefaults: {
		hideLabel: true
	},

	initComponent: function() {
		var me = this;
		me.defaults = Ext.apply({}, me.defaults, me.baseDefaults);
		Ext.Array.each(me.items, function(item, index, array) {
			// Apply a margin to each item except the last one.
			if(index < (array.length - 1)) {
				Ext.apply(item, {
					margin: "0 5 0 0"
				});
			}
		});
		me.callParent(arguments);
	}
});
