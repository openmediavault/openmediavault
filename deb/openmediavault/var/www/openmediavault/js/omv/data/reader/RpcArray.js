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

/**
 * @ingroup webgui
 * @class OMV.data.reader.RpcArray
 * @derived Ext.data.reader.Array
 */
Ext.define("OMV.data.reader.RpcArray", {
	extend: "Ext.data.reader.Array",
	alternateClassName: "Ext.data.RpcArrayReader",
	alias: "reader.rpcarray",

	readRecords: function(data) {
		var me = this;
		// Convert the array if necessary, see Ext.data.StoreManager::lookup()
		// for more details.
		// Example:
		// from: [ "test1", "test2" ]
		// to: [ [ "test1" ], [ "test2" ] ]
		if(Ext.isArray(data) && !Ext.isArray(data[0])) {
			Ext.Array.each(data, function(item, index, array) {
				array[index] = [ item ];
			});
		}
		return me.callParent([ data ]);
	}
});
