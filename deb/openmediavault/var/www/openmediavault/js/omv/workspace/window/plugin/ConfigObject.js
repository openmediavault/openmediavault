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

/**
 * @ingroup webgui
 * @class OMV.workspace.window.plugin.ConfigObject
 * @derived Ext.AbstractPlugin
 * A plugin that adds support to the RPC mechanism to load data by a given
 * id. The id must be a property of the parent/owner class.
 * @param idProperty The name of the field treated as this configuration
 *   object's unique id. Defaults to 'uuid'.
 * @param newIdValue The value of the id that identifies a new configuration
 *   object. Defaults to OMV.UUID_UNDEFINED.
 */
Ext.define("OMV.workspace.window.plugin.ConfigObject", {
	extend: "Ext.AbstractPlugin",
	alias: [ "plugin.configobject", "plugin.dbobject" ],

	idProperty: "uuid",
	newIdValue: OMV.UUID_UNDEFINED,

	/**
	 * The class constructor.
	 * The id of the database/configuration object MUST be part of
	 * the client component to which this plugin is associated to.
	 */
	constructor: function() {
		var me = this;
		me.callParent(arguments);
		var client = me.getCmp();
		// The client must have the given id property.
		if (Ext.isDefined(client[me.idProperty])) {
			// Apply the 'isNew' function which is used to check whether the
			// configuration object already exists or is a new one.
			Ext.apply(client, {
				isNew: function() {
					return (client[me.idProperty] == me.newIdValue);
				}
			});
			// Set other fields.
			Ext.apply(client, {
				autoLoadData: !((client.mode == "local") || client.isNew()),
				closeIfNotDirty: !client.isNew(),
				getRpcGetParams: me.interceptAfter(client, "getRpcGetParams"),
				getRpcSetParams: me.interceptAfter(client, "getRpcSetParams")
			});
		} else {
			Ext.Error.raise(Ext.String.format("Failed to initialize " +
			  "plugin. The property '{0}' is missing in class '{1}'.",
			  me.idProperty, Ext.getClassName(client)));
		}
	},

	interceptAfter: function(object, methodName) {
		var me = this;
		var method = object[methodName] || Ext.emptyFn;
		return (object[methodName] = function() {
			var o = {};
			o[me.idProperty] = object[me.idProperty];
			var params = method.apply(me, arguments);
			params = Ext.apply(params || {}, o);
			return params;
		});
	}
});
