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
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/workspace/panel/Panel.js")
// require("js/omv/workspace/panel/RrdGraph.js")
// require("js/omv/workspace/tab/Panel.js")

/**
 * @class OMV.module.admin.diagnostic.system.plugin.DiskIO
 * @derived OMV.workspace.tab.Panel
 */
Ext.define("OMV.module.admin.diagnostic.system.plugin.DiskIO", {
	extend: "OMV.workspace.tab.Panel",
	alias: "omv.plugin.diagnostic.system.diskio",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model"
	],

	title: _("Disk I/O"),

	initComponent: function() {
		var me = this;
		me.items = [];
		me.callParent(arguments);
		// Execute RPC to get the information required to add tab panels.
		OMV.Rpc.request({
			callback: function(id, success, response) {
				var store = Ext.create("OMV.data.Store", {
					model: OMV.data.Model.createImplicit({
						idProperty: "parentdevicefile",
						fields: [{
							name: "parentdevicefile",
							type: "string"
						}]
					}),
					sorters: [{
						direction: "ASC",
						property: "parentdevicefile"
					}],
					data: response
				});
				store.filter("parentdevicefile", /^\/dev\/.+$/);
				store.each(function(record) {
					var deviceName = record.get("parentdevicefile").replace(
						/^\/dev\//, "");
					// Create a tab panel for each disk.
					var tab = Ext.create("OMV.workspace.tab.Panel", {
						title: record.get("parentdevicefile")
					});
					tab.add(Ext.create("OMV.workspace.panel.RrdGraph", {
						title: _("Traffic"),
						rrdGraphName: "disk-octets-" + deviceName
					}));
					tab.add(Ext.create("OMV.workspace.panel.RrdGraph", {
						title: _("Operations"),
						rrdGraphName: "disk-ops-" + deviceName
					}));
					tab.add(Ext.create("OMV.workspace.panel.RrdGraph", {
						title: _("Time per operation"),
						rrdGraphName: "disk-time-" + deviceName
					}));
					tab.setActiveTab(0);
					this.add(tab);
				}, me);
				me.setActiveTab(0);
			},
			relayErrors: false,
			rpcData: {
				service: "FileSystemMgmt",
				method: "enumerateMountedFilesystems",
				params: {
					includeroot: true
				}
			}
		});
	}
});
