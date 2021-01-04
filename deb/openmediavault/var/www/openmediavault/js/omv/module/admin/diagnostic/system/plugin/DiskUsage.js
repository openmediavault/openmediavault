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
// require("js/omv/workspace/tab/Panel.js")

/**
 * @class OMV.module.admin.diagnostic.system.plugin.DiskUsage
 * @derived OMV.workspace.tab.Panel
 */
Ext.define("OMV.module.admin.diagnostic.system.plugin.DiskUsage", {
	extend: "OMV.workspace.tab.Panel",
	alias: "omv.plugin.diagnostic.system.diskusage",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model"
	],

	title: _("Disk usage"),

	initComponent: function() {
		var me = this;
		me.items = [];
		me.callParent(arguments);
		// Execute RPC to get the information required to add tab panels.
		OMV.Rpc.request({
			callback: function(id, success, response) {
				// The function used to create the tab configuration.
				var fnBuildRrdGraphConfig = function(item) {
					var config = {
						title: item.title,
						rrdGraphName: "df-" + item.mountpoint.substr(1).
						  replace(/\//g, "-")
					};
					if ("/" == item.mountpoint) {
						Ext.apply(config, {
							rrdGraphName: "df-root"
						});
					}
					return config;
				}
				// Sort the list of filesystems by devicefile or label.
				var store = Ext.create("OMV.data.Store", {
					model: OMV.data.Model.createImplicit({
						idProperty: "devicefile",
						fields: [
							{ name: "devicefile", type: "string" },
							{ name: "mountpoint", type: "string" },
							{ name: "title", type: "string" },
							{ name: "label", type: "string" }
						]
					}),
					sorters: [{
						direction: "ASC",
						property: "devicefile"
					}],
					data: response
				});
				store.each(function(record) {
					var item = record.getData();
					item.title = item.devicefile;
					if (("/" == item.mountpoint) && Ext.isEmpty(item.label)) {
						item.label = _("System");
					}
					if (!Ext.isEmpty(item.label)) {
						item.title = Ext.String.format("{0} [{1}]",
							item.title, item.label);
					}
					// Create a tab panel for each filesystem.
					this.add(Ext.create("OMV.workspace.panel.RrdGraph",
						fnBuildRrdGraphConfig(item)));
				}, me);
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
