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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.system.network.route.Route
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.system.network.route.Route", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "NetworkRoute",
	rpcGetMethod: "get",
	rpcSetMethod: "set",

	plugins: [{
		ptype: "configobject"
	}],

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormConfig: function() {
		return {
			layout: {
				type: "vbox",
				align: "stretch"
			}
		};
	},

	getFormItems: function() {
		return [{
			xtype: "textfield",
			name: "network",
			fieldLabel: _("Network"),
			vtype: "IPNetCIDR",
			allowBlank: false,
			plugins: [{
				ptype: "fieldinfo",
				text: _("IP or network address.")
			}]
		},{
			xtype: "textfield",
			name: "gateway",
			fieldLabel: _("Gateway"),
			vtype: "IP",
			allowBlank: false,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Gateway used to reach the above network address.")
			}]
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			flex: 1
		}];
	}
});

/**
 * @class OMV.module.admin.system.network.route.Routes
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.system.network.route.Routes", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.system.network.route.Route"
	],

	hidePagingToolbar: false,
	hideEditButton: true, // Simplifies duplicate checks.
	stateful: true,
	stateId: "a6faec48-f389-11e1-8b67-00221568ca88",
	columns: [{
		text: _("Network"),
		sortable: true,
		dataIndex: "network",
		stateId: "network"
	},{
		text: _("Gateway"),
		sortable: true,
		dataIndex: "gateway",
		stateId: "gateway"
	},{
		text: _("Comment"),
		sortable: true,
		dataIndex: "comment",
		stateId: "comment"
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
						{ name: "network", type: "string" },
						{ name: "gateway", type: "string" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "NetworkRoute",
						method: "getList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "network"
				}]
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.system.network.route.Route", {
			title: _("Add static route"),
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.system.network.route.Route", {
			title: _("Edit static route"),
			uuid: record.get("uuid"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	doDeletion: function(record) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "NetworkRoute",
				method: "delete",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "route",
	path: "/system/network",
	text: _("Static Routes"),
	position: 70,
	className: "OMV.module.admin.system.network.route.Routes"
});
