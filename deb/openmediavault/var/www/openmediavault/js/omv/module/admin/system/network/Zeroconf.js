/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.system.network.Zeroconf
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.system.network.Zeroconf", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"Ext.grid.plugin.RowEditing"
	],

	hideTopToolbar: true,
	hidePagingToolbar: false,
	stateful: true,
	stateId: "9dcb85a4-5cb3-11e2-a2ee-000c29f7c0eb",
	hideAddButton: true,
	hideEditButton: true,
	hideDeleteButton: true,
	columns: [{
		xtype: "textcolumn",
		text: _("Service"),
		sortable: true,
		dataIndex: "title",
		stateId: "title",
		flex: 1,
		translateText: true
	},{
		xtype: "textcolumn",
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name",
		flex: 1,
		editor: {
			xtype: "textfield",
			allowBlank: false
		}
	},{
		xtype: "booleaniconcolumn",
		text: _("Enable"),
		groupable: false,
		dataIndex: "enable",
		stateId: "enable",
		align: "center",
		width: 80,
		resizable: false,
		iconCls:  Ext.baseCSSPrefix + "grid-cell-booleaniconcolumn-switch",
		editor: {
			xtype: "checkboxfield"
		}
	}],
	plugins: [{
		ptype: "rowediting",
		clicksToEdit: 1
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "uuid",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "id", type: "string" },
						{ name: "name", type: "string" },
						{ name: "title", type: "string", persist: false },
						{ name: "enable", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Zeroconf",
						method: "getList"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "title"
				}]
			}),
			listeners: {
				scope: me,
				edit: function(editor, context, eOpts) {
					OMV.Rpc.request({
						scope: this,
						callback: function(id, success, response) {
							this.store.reload();
						},
						relayErrors: false,
						rpcData: {
							service: "Zeroconf",
							method: "set",
							params: context.record.getData({
								persist: true
							})
						}
					});
				}
			}
		});
		me.callParent(arguments);
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "zeroconf",
	path: "/system/network",
	text: _("Service Discovery"),
	position: 40,
	className: "OMV.module.admin.system.network.Zeroconf"
});
