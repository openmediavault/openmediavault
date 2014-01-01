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
// require("js/omv/tree/Panel.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/data/reader/RpcArray.js")

/**
 * @class OMV.tree.Folder
 * @derived OMV.tree.Panel
 * @param uuid The UUID of the volume to process. Required.
 * @param type The type of the configuration object. This can be "mntent"
 *   or "sharedfolder". Defaults to "mntent".
 * @param root The root node for this panel. This can be used to override
 *   the default configuration. Note, this is different to the Ext.tree.Panel
 *   behaviour.
 */
Ext.define("OMV.tree.Folder", {
	extend: "OMV.tree.Panel",
	requires: [
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.data.reader.RpcArray"
	],

	type: "mntent",

	autoScroll: true,
	rootVisible: false,

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("Ext.data.TreeStore", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					fields: [
						{ name: "text", type: "string", mapping: 0 },
						{ name: "name", type: "string", mapping: 0 },
						{ name: "path", type: "string", defaultValue: "" }
					]
				}),
				proxy: {
					type: "rpc",
					reader: "rpcarray",
					appendSortParams: false,
					rpcData: {
						service: "FolderBrowser",
						method: "get",
						params: {
							uuid: me.uuid,
							type: me.type,
							path: "" // Will be updated before a node
									  // is expanded.
						}
					}
				},
				sorters: [{
					direction: "ASC",
					property: "text"
				}],
				root: Ext.apply({
					expanded: false, // Load children async.
					text: "",
					name: "",
					path: ""
				}, me.root || {}),
				listeners: {
					scope: me,
					beforeload: function(store, operation) {
						// Modify the RPC parameters.
						Ext.apply(store.proxy.rpcData.params, {
							path: operation.node.get("path")
						});
					},
					load: function(store, node, records, successful) {
						// Prepare node data.
						var path = node.get("path");
						Ext.Array.each(records, function(record) {
							record.set("path", Ext.String.format(
							  "{0}{1}/", path, record.get("name")));
						});
					}
				}
			})
		});
		// Delete the 'root' config object, otherwise the ExtJS procedure
		// takes action which might result in unexpected behaviour.
		if(Ext.isDefined(me.root))
			delete me.root;
		me.callParent(arguments);
	}
});
