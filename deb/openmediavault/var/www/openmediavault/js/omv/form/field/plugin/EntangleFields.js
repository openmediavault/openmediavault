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

/**
 * @ingroup webgui
 * @class OMV.form.field.plugin.EntangleFields
 * @derived Ext.AbstractPlugin
 * Using this plugin it is possible to entangle various form fields together.
 * Changing the origin form field will automatically update the entangled
 * fields.
 * @param members An array of fields that are entangled with this one.
 */
Ext.define("OMV.form.field.plugin.EntangleFields", {
	extend: "Ext.AbstractPlugin",
	alias: "plugin.entanglefields",

	members: [],

	constructor: function(config) {
		var me = this;
		Ext.apply(config, {
			listeners: {
				scope: me,
				change: me.onChange
			}
		});
		me.callParent([ config ]);
	},

	init: function(c) {
		var me = this;
		me.field = c;
	},

	onChange: function(c, newValue) {
		var me = this;
		Ext.Array.each(me.members, function(member) {
			var field = me.findField(name);
			if(!field || !field.isFormField)
				return;
			field.setValue(newValue);
		});
	},

	/**
	 * Helper method to get the form panel associated to this field.
	 * @return The form panel, or null if none was found.
	 */
	getForm: function() {
		var me = this;
		var result = null;
		me.field.bubble(function(c) {
			if(c.isPanel) {
				result = c;
				return false;
			}
		}, me);
		return result;
	},

	/**
	 * Helper method to get a field by name or id.
	 * @param id The value to search for (specify either a id or name).
	 * @return The first matching field, or null if none was found.
	 */
	findField: function(id) {
		var me = this;
		var selector = Ext.String.format("[name={0}], #{0}", id);
		var field = me.getForm().query(selector)[0];
		return Ext.isObject(field) ? field : null;
	}
});
