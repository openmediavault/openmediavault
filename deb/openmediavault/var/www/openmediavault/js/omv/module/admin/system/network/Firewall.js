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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.system.network.firewall.Rule
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.system.network.firewall.Rule", {
	extend: "OMV.workspace.window.Form",

	mode: "local",
	width: 550,
	height: 400,

	getFormItems: function() {
		return [{
			xtype: "combo",
			name: "chain",
			fieldLabel: _("Direction"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "INPUT", "INPUT" ],
					[ "OUTPUT", "OUTPUT" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "INPUT"
		},{
			xtype: "combo",
			name: "action",
			fieldLabel: _("Action"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "ACCEPT", "ACCEPT" ],
					[ "REJECT", "REJECT" ],
					[ "DROP", "DROP" ],
					[ "LOG", "LOG" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "REJECT"
		},{
			xtype: "textfield",
			name: "source",
			fieldLabel: _("Source"),
			vtype: "IPv4Fw",
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Source address can be either a network IP address (with /mask), a IP range or a plain IP address. A '!' argument before the address specification inverts the sense of the address.")
			}]
		},{
			xtype: "textfield",
			name: "sport",
			fieldLabel: _("Source port"),
			vtype: "portFw",
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Match if the source port is one of the given ports. E.g. 21 or !443 or 1024-65535.")
			}]
		},{
			xtype: "textfield",
			name: "destination",
			fieldLabel: _("Destination"),
			vtype: "IPv4Fw",
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Destination address can be either a network IP address (with /mask), a IP range or a plain IP address. A '!' argument before the address specification inverts the sense of the address.")
			}]
		},{
			xtype: "textfield",
			name: "dport",
			fieldLabel: _("Destination port"),
			vtype: "portFw",
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Match if the destination port is one of the given ports. E.g. 21 or !443 or 1024-65535.")
			}]
		},{
			xtype: "combo",
			name: "protocol",
			fieldLabel: _("Protocol"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "tcp", "TCP" ],
					[ "udp", "UDP" ],
					[ "icmp", "ICMP" ],
					[ "all", _("All") ],
					[ "!tcp", _("Not TCP") ],
					[ "!udp", _("Not UDP") ],
					[ "!icmp", _("Not ICMP") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "tcp"
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: _("Extra options"),
			allowBlank: true
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		}];
	},

	isValid: function() {
		var me = this;
		if(!me.callParent(arguments))
			return false;
		var valid = true;
		// Do additional checks
		var values = me.getValues();
		if(!Ext.isEmpty(values.sport) && (values.protocol == "all")) {
			me.markInvalid([
				{ id: "protocol", msg: _("'All' is not allowed") }
			]);
			valid = false;
		}
		if(!Ext.isEmpty(values.dport) && (values.protocol == "all")) {
			me.markInvalid([
				{ id: "protocol", msg: _("'All' is not allowed") }
			]);
			valid = false;
		}
		return valid;
	}
});

/**
 * @class OMV.module.admin.system.network.firewall.Rules
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.system.network.firewall.Rules", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format",
		"OMV.module.admin.system.network.firewall.Rule"
	],

	hideUpButton: false,
	hideDownButton: false,
	hideApplyButton: false,
	hideRefreshButton: false,
	hidePagingToolbar: true,
	stateful: true,
	stateId: "edb8c917-abd1-4b59-a67f-fc4ef3ab8a5f",
	columns: [{
		text: _("Direction"),
		sortable: false,
		dataIndex: "chain",
		stateId: "chain"
	},{
		text: _("Action"),
		sortable: false,
		dataIndex: "action",
		stateId: "action"
	},{
		xtype: "emptycolumn",
		emptyText: "-",
		text: _("Source"),
		sortable: false,
		dataIndex: "source",
		stateId: "source"
	},{
		xtype: "emptycolumn",
		emptyText: "-",
		text: _("Port"),
		sortable: false,
		dataIndex: "sport",
		stateId: "sport"
	},{
		xtype: "emptycolumn",
		emptyText: "-",
		text: _("Destination"),
		sortable: false,
		dataIndex: "destination",
		stateId: "destination"
	},{
		xtype: "emptycolumn",
		emptyText: "-",
		text: _("Port"),
		sortable: false,
		dataIndex: "dport",
		stateId: "dport"
	},{
		text: _("Protocol"),
		sortable: false,
		dataIndex: "protocol",
		stateId: "protocol",
		renderer: OMV.util.Format.arrayRenderer([
			[ "tcp", "TCP" ],
			[ "udp", "UDP" ],
			[ "icmp", "ICMP" ],
			[ "all", "All" ],
			[ "!tcp", "Not TCP" ],
			[ "!udp", "Not UDP" ],
			[ "!icmp", "Not ICMP" ]
		])
	},{
		text: _("Comment"),
		sortable: false,
		dataIndex: "comment",
		stateId: "comment"
	}],
	viewConfig: {
		autoScroll: true,
		loadMask: true,
		stripeRows: true,
		plugins: {
			ptype: "gridviewdragdrop"
		}
	},

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: null,
					fields: [
						{ name: "uuid" },
						{ name: "rulenum" },
						{ name: "chain" },
						{ name: "action" },
						{ name: "source" },
						{ name: "sport" },
						{ name: "destination" },
						{ name: "dport" },
						{ name: "protocol" },
						{ name: "comment" },
						{ name: "extraoptions" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Iptables",
						method: "getRules"
					},
					extraParams: {
						type: [ "userdefined" ]
					}
				},
				sorters: [{
					direction: "ASC",
					property: "rulenum"
				}],
				listeners: {
					scope: me,
					load: function(store, records, options) {
						this.setToolbarButtonDisabled("apply", true);
					},
					add: function(store, records, index) {
						this.setToolbarButtonDisabled("apply", false);
					},
					update: function(store, record, operation) {
						this.setToolbarButtonDisabled("apply", false);
					},
					remove: function(store, record, index) {
						this.setToolbarButtonDisabled("apply", false);
					}
				}
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.system.network.firewall.Rule", {
			title: _("Add firewall rule"),
			listeners: {
				scope: me,
				submit: function(c, values) {
					var lastRowNum = this.store.getCount();
					Ext.apply(values, {
						uuid: OMV.UUID_UNDEFINED,
						rulenum: lastRowNum
					});
					var newRecord = new this.store.model(values);
					me.store.insert(lastRowNum, newRecord);
				}
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var selModel = this.getSelectionModel();
		var record = selModel.getSelection()[0];
		var wnd = Ext.create("OMV.module.admin.system.network.firewall.Rule", {
			title: _("Edit firewall rule"),
			listeners: {
				scope: me,
				submit: function(c, values) {
					// Update the selected record.
					record.beginEdit();
					record.set(values);
					record.endEdit();
				}
			}
		});
		wnd.setValues(record.getData());
		wnd.show();
	},

	doDeletion: function(record) {
		var me = this;
		// Delete the local record only.
		me.store.remove(record);
		me.onDeletion(null, true, null);
	},

	afterDeletion: function() {
		// Do not reload the store but update the 'rulenum' fields.
		this.updateRuleNums();
	},

	afterMoveRows: function(records, index) {
		var me = this;
		me.callParent(arguments);
		// Update the 'rulenum' fields.
		me.updateRuleNums();
	},

	onApplyButton: function() {
		var me = this;
		var records = me.store.getRange();
		var params = [];
		Ext.Array.each(records, function(record) {
			params.push(record.getData());
		});
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			relayErrors: false,
			callback: function(id, success, response) {
				OMV.MessageBox.success();
				this.store.reload();
			},
			rpcData: {
				service: "Iptables",
				method: "setRules",
				params: params
			}
		});
	},

	/**
	 * Helper method to update the rule numbers.
	 * @private
	 */
	updateRuleNums: function() {
		var me = this;
		me.store.each(function(record, index) {
			record.beginEdit();
			record.set("rulenum", index);
			record.endEdit();
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "firewall",
	path: "/system/network",
	text: _("Firewall"),
	position: 60,
	className: "OMV.module.admin.system.network.firewall.Rules"
});
