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
// require("js/omv/data/Store.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/workspace/grid/Panel.js")

/**
 * @class OMV.module.admin.diagnostic.service.plugin.Overview
 * @derived OMV.workspace.grid.Panel
 * Display statistics from various services, e.g. SSH, FTP or SMB/CIFS.
 */
Ext.define("OMV.module.admin.diagnostic.service.plugin.Overview", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.proxy.Rpc"
	],

	constructor: function(config) {
		var me = this;
		config = Ext.apply({
			hideAddButton: true,
			hideEditButton: true,
			hideDeleteButton: true,
			hideRefreshButton: false,
			hidePagingToolbar: true,
			disableSelection: true,
			stateful: true,
			stateId: "976130ef-a647-40e8-9b08-02ced906680a",
			columns: [{
				text: _("Service"),
				sortable: true,
				dataIndex: "title",
				stateId: "title",
				flex: 1
			},{
				text: _("Enabled"),
				sortable: true,
				dataIndex: "enabled",
				stateId: "enabled",
				width: 80,
				resizable: false,
				align: "center",
				renderer: OMV.util.Format.booleanIconRenderer(
				  "switch_on.png", "switch_off.png")
			},{
				text: _("Running"),
				sortable: true,
				dataIndex: "running",
				stateId: "running",
				width: 80,
				resizable: false,
				align: "center",
				renderer: function(value, metaData, record, rowIndex,
				  colIndex, store, view) {
					var img;
					switch(record.get("enabled")) {
					case 1:
					case true:
						img = (true == value) ? "led_green.png" :
						  "led_red.png";
						break;
					default:
						img = (true == value) ? "led_green.png" :
						  "led_gray.png";
						break;
					}
					return "<img border='0' src='images/" + img + "'>";
				}
			}]
		}, config || {});
		me.callParent([ config ]);
	},

	initComponent: function() {
		var me = this;
		me.store = Ext.create("OMV.data.Store", {
			autoLoad: true,
			fields: [
				{ name: "name", type: "string" },
				{ name: "title", type: "string" },
				{ name: "enabled", type: "boolean" },
				{ name: "running", type: "boolean" }
			],
			proxy: Ext.create("OMV.data.proxy.Rpc", {
				appendSortParams: false,
				rpcData: {
					service: "Services",
					method: "getStatus"
				}
			}),
			sorters: [{
				direction: "ASC",
				property: "name"
			}]
		});
		me.callParent(arguments);
	}
});

OMV.PluginManager.register({
	ptype: "diagnostic",
	id: "service",
	text: _("Overview"),
	position: 10,
	className: "OMV.module.admin.diagnostic.service.plugin.Overview"
});
