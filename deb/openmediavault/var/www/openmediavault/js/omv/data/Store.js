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
 * @class OMV.data.Store
 * @derived Ext.data.Store
 */
Ext.define("OMV.data.Store", {
	extend: "Ext.data.Store",

	constructor: function() {
		var me = this;
		me.callParent(arguments);
		me.on({
			scope: me,
			beforeload: function(store) {
				var result = true;
				// Workaround for bug in ExtJS 4.2.2:
				// If 'autoLoad' and 'remoteSort' is enabled, then the store is
				// loaded twice which might cause problems in the backend RPC
				// (e.g. locked structures). To prevent such a situation simply
				// check if the store is currently loaded and abort the current
				// load request.
				if(me.autoLoad && (me.remoteSort || me.remoteGroup ||
				  me.remoteFilter))
					result = !store.isLoading();
				return result;
			}
		})
	}
});
