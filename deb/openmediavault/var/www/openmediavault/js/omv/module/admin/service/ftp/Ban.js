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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.service.ftp.BanRule
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.ftp.BanRule", {
	extend: "OMV.workspace.window.Form",
	requires: [
	    "OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "FTP",
	rpcGetMethod: "getModBanRule",
	rpcSetMethod: "setModBanRule",
	plugins: [{
		ptype: "configobject"
	}],

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormItems: function() {
		return [{
			xtype: "combo",
			name: "event",
			fieldLabel: _("Event"),
			queryMode: "local",
			store: [
				"AnonRejectPasswords",
				"ClientConnectRate",
				"MaxClientsPerClass",
				"MaxClientsPerHost",
				"MaxClientsPerUser",
				"MaxConnectionsPerHost",
				"MaxHostsPerUser",
				"MaxLoginAttempts",
				"TimeoutIdle",
				"TimeoutNoTransfer"
			],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "MaxConnectionsPerHost",
			plugins: [{
				ptype: "fieldinfo",
				text: _("This rule is triggered whenever the selected event directive occurs.")
			}]
		},{
			xtype: "numberfield",
			name: "occurrence",
			fieldLabel: _("Occurrence"),
			minValue: 1,
			allowDecimals: false,
			allowBlank: false,
			value: 2,
			plugins: [{
				ptype: "fieldinfo",
				text: _("This parameter says that if N occurrences of the event happen within the given time interval, then a ban is automatically added.")
			}]
		},{
			xtype: "textfield",
			name: "timeinterval",
			fieldLabel: _("Time interval"),
			allowBlank: false,
			maskRe: /[\d:]/,
			regex: /^\d{2}:\d{2}:\d{2}$/,
			regexText: _("This field must have the format hh:mm:ss"),
			maxLength: 8,
			value: "00:30:00",
			plugins: [{
				ptype: "fieldinfo",
				text: _("Specifies the time interval in hh:mm:ss in which the given number of occurrences must happen to add the ban.")
			}]
		},{
			xtype: "textfield",
			name: "expire",
			fieldLabel: _("Expire"),
			allowBlank: false,
			maskRe: /[\d:]/,
			regex: /^\d{2}:\d{2}:\d{2}$/,
			regexText: _("This field must have the format hh:mm:ss"),
			maxLength: 8,
			value: "00:10:00",
			plugins: [{
				ptype: "fieldinfo",
				text: _("Specifies the time in hh:mm:ss after which the ban expires.")
			}]
		}];
	}
});

/**
 * @class OMV.module.admin.service.ftp.BanRules
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.ftp.BanRules", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.ftp.BanRule"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "b145dd0c-8fe8-4570-947f-e4c0ee40b900",
	columns: [{
		text: _("Event"),
		sortable: true,
		dataIndex: "event",
		stateId: "event"
	},{
		text: _("Occurrence"),
		sortable: true,
		dataIndex: "occurrence",
		stateId: "occurrence"
	},{
		text: _("Time interval"),
		sortable: true,
		dataIndex: "timeinterval",
		stateId: "timeinterval"
	},{
		text: _("Expire"),
		sortable: true,
		dataIndex: "expire",
		stateId: "expire"
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
						{ name: "event", type: "string" },
						{ name: "occurrence", type: "string" },
						{ name: "timeinterval", type: "string" },
						{ name: "expire", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "FTP",
						method: "getModBanRuleList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.ftp.BanRule", {
			title: _("Add rule"),
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
		Ext.create("OMV.module.admin.service.ftp.BanRule", {
			title: _("Edit rule"),
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
				service: "FTP",
				method: "deleteModBanRule",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "ban",
	path: "/service/ftp",
	text: _("Ban list"),
	position: 30,
	className: "OMV.module.admin.service.ftp.BanRules"
});
