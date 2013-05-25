/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/util/Format.js")

/**
 * @class OMV.module.admin.diagnostic.system.Overview
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.diagnostic.system.Overview", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format"
	],

	disableLoadMaskOnLoad: true,
	autoReload: true,
	reloadInterval: 5000,
	hideTopToolbar: true,
	hidePagingToolbar: true,
	hideHeaders: true,
	disableSelection: true,
	stateful: true,
	stateId: "eda034fd-aa2e-4a7a-9f75-e054d26bcd31",
	columns: [{
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name",
		width: 150,
		tdCls: Ext.baseCSSPrefix + "grid-cell-gray",
		renderer: function(value, metaData, record, rowIndex,
		  colIndex, store, view) {
			return _(value);
		}
	},{
		text: _("Value"),
		sortable: true,
		dataIndex: "value",
		stateId: "value",
		flex: 1,
		renderer: function(value, metaData, record, rowIndex,
		  colIndex, store, view) {
			var me = this;
			var result = value;
			switch(record.get("type")) {
			case "time":
				var renderer = OMV.util.Format.localeTimeRenderer();
				result = renderer.apply(me, arguments);
				break;
			case "progress":
				var renderer = OMV.util.Format.progressBarRenderer(
				  value.value / 100, value.text);
				result = renderer.apply(me, arguments);
				break;
			default:
				// Nothing to do here
				break;
			}
			return result;
		}
	}],
	viewConfig: {
		trackOver: false
	},

	initComponent: function() {
		var me = this;
		me.store = Ext.create("OMV.data.Store", {
			autoLoad: true,
			model: OMV.data.Model.createImplicit({
				idProperty: "index",
				fields: [
					{ name: "index", type: "int" },
					{ name: "type", type: "string" },
					{ name: "name", type: "string" },
					{ name: "value", type: "auto" }
				]
			}),
			proxy: {
				type: "rpc",
				appendSortParams: false,
				rpcData: {
					service: "System",
					method: "getInformation",
					options: {
						updatelastaccess: false
					}
				}
			},
			sorters: [{
				direction: "ASC",
				property: "index"
			}]
		});
		me.callParent(arguments);
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "overview",
	path: "/diagnostic/system",
	text: _("Overview"),
	position: 10,
	className: "OMV.module.admin.diagnostic.system.Overview"
});
