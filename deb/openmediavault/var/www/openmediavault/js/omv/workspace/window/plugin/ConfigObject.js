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
 * A plugin that adds support to the RPC mechanism to load data
 * by a given UUID. The UUID must be a property of the parent/owner
 * class.
 */
Ext.define("OMV.workspace.window.plugin.ConfigObject", {
	extend: "Ext.AbstractPlugin",
	alias: [ "plugin.configobject", "plugin.dbobject" ],

	/**
	 * The class constructor.
	 * The UUID of the database/configuration object MUST be part of
	 * the client component to which this plugin is associated to.
	 */
	constructor: function() {
		var me = this;
		me.callParent(arguments);
		var client = me.getCmp();
		// The client must have an 'uuid' property.
		if(Ext.isDefined(client.uuid)) {
			Ext.apply(client, {
				isNew: function() {
					return (client.uuid == OMV.UUID_UNDEFINED);
				},
				autoLoadData: !((client.mode == "local") ||
				  (client.uuid == OMV.UUID_UNDEFINED)),
				closeIfNotDirty: !(client.uuid == OMV.UUID_UNDEFINED),
				getRpcGetParams: me.interceptAfter(client, "getRpcGetParams"),
				getRpcSetParams: me.interceptAfter(client, "getRpcSetParams")
			});
		} else {
			Ext.Error.raise(Ext.String.format("Failed to initialize " +
			  "plugin. The property 'uuid' is missing in class '{0}'.",
			  Ext.getClassName(client)));
		}
	},

	interceptAfter: function(object, methodName) {
		var method = object[methodName] || Ext.emptyFn;
		return (object[methodName] = function() {
			var params = method.apply(this, arguments);
			return Ext.apply(params || {}, {
				uuid: object.uuid
			});
		});
	}
});
