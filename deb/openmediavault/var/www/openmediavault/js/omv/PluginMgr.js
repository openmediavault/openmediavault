/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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

Ext.ns("OMV");

/**
 * @class OMV.PluginMgr
 */
OMV.PluginMgr = function() {
	var plugins = {
		"log": [],
		"sysinfo": []
	};
	return {
		/**
		 * Register a plugin.
		 * @param type The plugin type, e.g. 'log' or 'sysinfo'
		 * @return None
		 */
		register : function() {
			var args = Ext.toArray(arguments);
			switch (args[0]) {
			case "log":
				plugins.log.push({
					ptype: args[1],
					cls: args[2]
				});
				break;
			case "sysinfo":
				plugins.sysinfo.push({
					type: args[1],
					cls: args[2]
				});
				break;
			}
		},

		/**
		 * Get the list of registered plugins of the given type.
		 * @param mtype The module class.
		 * @return The list of registered modules for the given class/type.
		 */
		get : function(type) {
			return plugins[type];
		}
	};
}();
OMV.preg = OMV.PluginMgr.register;
