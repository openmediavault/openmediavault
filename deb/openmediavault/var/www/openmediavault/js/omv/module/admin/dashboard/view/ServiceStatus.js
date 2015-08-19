/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
// require("js/omv/workspace/dashboard/View.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/workspace/grid/Panel.js")

/**
 * @class OMV.module.admin.dashboard.view.ServiceStatus
 * @derived OMV.workspace.dashboard.View
 */
Ext.define("OMV.module.admin.dashboard.view.ServiceStatus", {
	extend: "OMV.workspace.dashboard.View",
	alias: "widget.module.admin.dashboard.view.servicestatus",
	requires: [
		"OMV.workspace.grid.Panel",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],

	height: 200,
	refreshInterval: 10000,

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			items: [ me.gp = Ext.create("OMV.workspace.grid.Panel", {
				disableLoadMaskOnLoad: true,
				hideTopToolbar: true,
				hidePagingToolbar: true,
				disableSelection: true,
				stateful: true,
				stateId: "08ee3b76-e325-11e3-ad0a-00221568ca88",
				columns: [{
					text: _("Service"),
					sortable: true,
					dataIndex: "title",
					stateId: "title",
					flex: 1
				},{
					xtype: "booleaniconcolumn",
					text: _("Enabled"),
					sortable: true,
					dataIndex: "enabled",
					stateId: "enabled",
					width: 80,
					resizable: false,
					align: "center",
					trueIcon: Ext.supports.Svg ? "switch_on.svg" :
					  "switch_on.png",
					falseIcon: Ext.supports.Svg ? "switch_off.svg" :
					  "switch_off.png"
				},{
					text: _("Running"),
					sortable: true,
					dataIndex: "running",
					stateId: "running",
					width: 80,
					resizable: false,
					align: "center",
					renderer: function(value, metaData, record) {
						var img;
						switch (record.get("enabled")) {
						case 1:
						case true:
							img = (true == value) ?
							  (Ext.supports.Svg ? "led_blue.svg" : "led_blue.png" ) :
							  (Ext.supports.Svg ? "led_red.svg" : "led_red.png" );
							break;
						default:
							img = (true == value) ?
							  (Ext.supports.Svg ? "led_blue.svg" : "led_blue.png" ) :
							  (Ext.supports.Svg ? "led_gray.svg" : "led_gray.png" );
							break;
						}
						return "<img border='0' src='images/" + img + "'>";
					}
				}],
				viewConfig: {
					trackOver: false
				},
				store: Ext.create("OMV.data.Store", {
					autoLoad: true,
					model: OMV.data.Model.createImplicit({
						idProperty: "name",
						fields: [
							{ name: "name", type: "string" },
							{ name: "title", type: "string" },
							{ name: "enabled", type: "boolean" },
							{ name: "running", type: "boolean" }
						]
					}),
					proxy: {
						type: "rpc",
						appendSortParams: false,
						rpcData: {
							service: "Services",
							method: "getStatus",
							options: {
								updatelastaccess: false
							}
						}
					},
					sorters: [{
						direction: "ASC",
						property: "name"
					}]
				})
			}) ]
		});
		me.callParent(arguments);
	},

	doRefresh: function() {
		var me = this;
		me.gp.doReload();
	}
});
