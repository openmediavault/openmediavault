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
 * @class OMV.PluginManager
 */
Ext.define("OMV.PluginManager", {
	singleton: true,
	alternateClassName: "OMV.PluginMgr",

	items: [],

	/**
	 * Register a plugin.
	 * @param config The plugin configuration.An object which may contain
	 *   the following properties:
	 *   \em ptype
	 *   \em id 
	 *   \em className
	 *   \em text
	 * @return None
	 */
	register: function(config) {
		var me = this;
		// Check if the given class exists.
		if(!Ext.ClassManager.isCreated(config.className)) {
			Ext.Error.raise(Ext.String.format("Failed to register plugin. " +
			  "The class '{0}' does not exist.", config.className));
			return;
		}
		var ptype = me.items[config.ptype];
		if(!ptype)
			ptype = me.items[config.ptype] = [];
		// Add new plugin configuration.
		Ext.Array.push(ptype, Ext.apply({
			position: 100,
		}, config));
		// Sort array items based on their the 'position' field.
		ptype = Ext.Array.sort(ptype, function(a, b) {
			return a.position < b.position ? -1 : 1;
		});
	},

	/**
	 * Get the list of registered plugins of the given type.
	 * @param ptype The plugin type.
	 * @return The list of registered plugins for the given type.
	 */
	get: function(ptype, id) {
		var me = this;
		var items = me.items[ptype];
		if(!items)
			return [];
		if(Ext.isDefined(id)) {
			items = Ext.Array.filter(items, function(item, index, array) {
				return item.id === id;
			});
		}
		return items;
	}
});
