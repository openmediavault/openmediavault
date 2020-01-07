/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
// require("js/omv/workspace/window/Form.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.system.network.zeroconf.Service
 * @derived OMV.workspace.window.Form
 * @param uuid The UUID of the configuration object.
 * @param service The name of the service.
 */
Ext.define("OMV.module.admin.system.network.zeroconf.Service", {
	extend: "OMV.workspace.window.Form",

	rpcService: "Zeroconf",
	rpcGetMethod: "get",
	rpcSetMethod: "set",
	title: _("Edit service"),
	width: 350,

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: false
		},{
			xtype: "textfield",
			fieldLabel: _("Service"),
			readOnly: true,
			submitValue: false,
			value: me.service
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name")
		},{
			xtype: "hidden",
			name: "uuid",
			value: me.uuid
		},{
			xtype: "hidden",
			name: "id"
		}];
	},

	getRpcGetParams: function() {
		var me = this;
		return Ext.apply({}, {
			uuid: me.uuid
		});
	}
});

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
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.system.network.zeroconf.Service"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "9dcb85a4-5cb3-11e2-a2ee-000c29f7c0eb",
	hideAddButton: true,
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
		flex: 1
	},{
		xtype: "enabledcolumn",
		text: _("Enabled"),
		groupable: false,
		dataIndex: "enable",
		stateId: "enable"
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
			})
		});
		me.callParent(arguments);
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.system.network.zeroconf.Service", {
			uuid: record.get("uuid"),
			service: record.get("title"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "zeroconf",
	path: "/system/network",
	text: _("Service Discovery"),
	position: 40,
	className: "OMV.module.admin.system.network.Zeroconf"
});
