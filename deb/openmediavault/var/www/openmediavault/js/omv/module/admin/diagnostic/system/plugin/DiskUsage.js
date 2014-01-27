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
// require("js/omv/PluginManager.js")
// require("js/omv/Rpc.js")
// require("js/omv/workspace/tab/Panel.js")

/**
 * @class OMV.module.admin.diagnostic.system.plugin.DiskUsage
 * @derived OMV.workspace.tab.Panel
 */
Ext.define("OMV.module.admin.diagnostic.system.plugin.DiskUsage", {
	extend: "OMV.workspace.tab.Panel",
	requires: [
		"OMV.Rpc"
	],

	initComponent: function() {
		var me = this;
		me.items = [];
		me.callParent(arguments);
		// Execute RPC to get the information required to add tab panels.
		OMV.Rpc.request({
			callback: function(id, success, response) {
				var fnBuildRrdGraphConfig = function(item) {
					var config = {
						title: item.label || item.devicefile,
						rrdGraphName: "df-" + item.mountpoint.substr(1).
						  replace(/\//g, "-")
					};
					if("/dev/root" === item.devicefile) {
						Ext.apply(config, {
							title: _("System"),
							rrdGraphName: "df-root"
						});
					}
					return config;
				}
				// Create a tab panel for each filesystem.
				Ext.Array.each(response, function(item) {
					me.add(Ext.create("OMV.workspace.panel.RrdGraph",
					  fnBuildRrdGraphConfig(item)));
				});
			},
			relayErrors: false,
			rpcData: {
				service: "FileSystemMgmt",
				method: "enumerateMountedFilesystems",
				params: {
					includeRoot: true
				}
			}
		});
	}
});

OMV.PluginManager.register({
	ptype: "diagnostic",
	id: "system",
	text: _("Disk usage"),
	className: "OMV.module.admin.diagnostic.system.plugin.DiskUsage"
});
