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
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/workspace/grid/Panel.js")

/**
 * @class OMV.module.admin.diagnostic.service.plugin.Overview
 * @derived OMV.workspace.grid.Panel
 * Display statistics from various services, e.g. SSH, FTP or SMB/CIFS.
 */
Ext.define("OMV.module.admin.diagnostic.service.plugin.Overview", {
	extend: "OMV.workspace.grid.Panel",
	alias: "omv.plugin.diagnostic.service.overview",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],

	title: _("Overview"),
	position: 10,

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
				xtype: "textcolumn",
				text: _("Service"),
				sortable: true,
				dataIndex: "title",
				stateId: "title",
				flex: 1
			},{
				xtype: "enabledcolumn",
				text: _("Enabled"),
				sortable: true,
				dataIndex: "enabled",
				stateId: "enabled"
			},{
				xtype: "fonticoncolumn",
				text: _("Running"),
				sortable: true,
				dataIndex: "running",
				stateId: "running",
				width: 80,
				resizable: false,
				align: "center",
				getFontIconCls: function(value, metaData, record) {
					var cls = ["mdi mdi-checkbox-blank-circle"];
					switch (record.get("enabled")) {
					case 1:
					case true: // Service enabled
						Ext.Array.push(cls, (true == value) ?
							Ext.baseCSSPrefix + "color-boolean-true" :
							Ext.baseCSSPrefix + "color-error");
						break;
					default: // Service disabled
						Ext.Array.push(cls, (true == value) ?
							Ext.baseCSSPrefix + "color-boolean-true" :
							Ext.baseCSSPrefix + "color-boolean-false");
						break;
					}
					return cls;
				}
			}]
		}, config || {});
		me.callParent([ config ]);
	},

	initComponent: function() {
		var me = this;
		me.store = Ext.create("OMV.data.Store", {
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
					method: "getStatus"
				}
			},
			sorters: [{
				direction: "ASC",
				property: "name"
			}]
		});
		me.callParent(arguments);
	}
});
