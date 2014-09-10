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
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.system.network.firewall.Rule
 * @derived OMV.workspace.window.Form
 * @param family The address family, e.g. 'inet' or 'inet6'. Defaults
 *   to 'inet'.
 */
Ext.define("OMV.module.admin.system.network.firewall.Rule", {
	extend: "OMV.workspace.window.Form",

	family: "inet",

	mode: "local",
	width: 550,
	height: 400,

	getFormConfig: function() {
		return {
			layout: {
				type: "vbox",
				align: "stretch"
			}
		};
	},

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "combo",
			name: "family",
			fieldLabel: _("Family"),
			queryMode: "local",
			store: [
				[ "inet", "IPv4" ],
				[ "inet6", "IPv6" ]
			],
			readOnly: true,
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: me.family
		},{
			xtype: "combo",
			name: "chain",
			fieldLabel: _("Direction"),
			queryMode: "local",
			store: [
				[ "INPUT", "INPUT" ],
				[ "OUTPUT", "OUTPUT" ]
			],
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
			store: [
				[ "ACCEPT", "ACCEPT" ],
				[ "REJECT", "REJECT" ],
				[ "DROP", "DROP" ],
				[ "LOG", "LOG" ],
				[ "", _("Nothing") ]
			],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "REJECT",
			plugins: [{
				ptype: "fieldinfo",
				text: _("This specifies what to do if the packet matches.")
			}]
		},{
			xtype: "textfield",
			name: "source",
			fieldLabel: _("Source"),
			vtype: (me.family == "inet") ? "IPv4Fw" : "IPv6Fw",
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
			vtype: (me.family == "inet") ? "IPv4Fw" : "IPv6Fw",
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
			store: [
				[ "tcp", "TCP" ],
				[ "udp", "UDP" ],
				(me.family == "inet") ? [ "icmp", "ICMP" ] :
				  [ "icmpv6", "ICMPv6" ],
				[ "all", _("All") ],
				[ "!tcp", _("Not TCP") ],
				[ "!udp", _("Not UDP") ],
				(me.family == "inet") ? [ "!icmp", _("Not ICMP") ] :
				  [ "!icmpv6", _("Not ICMPv6") ]
			],
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
			allowBlank: true,
			flex: 1
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
 * @param family The address family, e.g. 'inet' or 'inet6'. Defaults
 *   to 'inet'.
 */
Ext.define("OMV.module.admin.system.network.firewall.Rules", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.module.admin.system.network.firewall.Rule"
	],

	family: "inet",

	hideUpButton: false,
	hideDownButton: false,
	hideApplyButton: false,
	hideRefreshButton: false,
	hidePagingToolbar: true,
	stateful: true,
	stateId: "edb8c917-abd1-4b59-a67f-fc4ef3ab8a5f",
	columnsTpl: [{
/*
		sortable: false,
		dataIndex: "rulenum",
		stateId: "rulenum",
		align: "center",
		width: 80,
		resizable: false
	},{
*/
		text: _("Direction"),
		sortable: false,
		dataIndex: "chain",
		stateId: "chain"
	},{
		xtype: "emptycolumn",
		emptyText: "-",
		text: _("Action"),
		sortable: false,
		dataIndex: "action",
		stateId: "action"
	},{
		xtype: "mapcolumn",
		text: _("Family"),
		sortable: false,
		dataIndex: "family",
		stateId: "family",
		mapItems: {
			inet: _("IPv4"),
			inet6: _("IPv6")
		}
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
		xtype: "mapcolumn",
		text: _("Protocol"),
		sortable: false,
		dataIndex: "protocol",
		stateId: "protocol",
		mapItems: {
			"tcp": "TCP",
			"udp": "UDP",
			"icmp": "ICMP",
			"icmpv6": "ICMPv6",
			"all": _("All"),
			"!tcp": _("Not TCP"),
			"!udp": _("Not UDP"),
			"!icmp": _("Not ICMP"),
			"!icmpv6": _("Not ICMPv6")
		}
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
			columns: Ext.clone(me.columnsTpl),
			store: me.createStore()
		});
		me.callParent(arguments);
	},

	/**
	 * Helper function to create a store.
	 * @private
	 */
	createStore: function() {
		var me = this;
		return Ext.create("OMV.data.Store", {
			autoLoad: true,
			model: OMV.data.Model.createImplicit({
				idgen: "uuid", // Populate 'id' field automatically.
				fields: [
					{ name: "id", type: "string", persist: false },
					{ name: "uuid" },
					{ name: "rulenum" },
					{ name: "chain" },
					{ name: "action" },
					{ name: "family" },
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
					method: (me.family == "inet") ? "getRules" : "getRules6"
				},
				extraParams: {
					type: [ "userdefined" ]
				},
				writer: {
					type: "json",
					writeRecordId: false
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
		});
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		Ext.Array.insert(items, 0, [{
			id: me.getId() + "-family",
			xtype: "combo",
			queryMode: "local",
			store: [
				[ "inet", "IPv4" ],
				[ "inet6", "IPv6" ]
			],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: me.family,
			listeners: {
				scope: me,
				change: function(combo, value) {
					this.family = value;
					// Create a new store.
					var store = this.createStore();
					this.reconfigure(store, Ext.clone(this.columnsTpl));
				}
			}
		}]);
		return items;
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.system.network.firewall.Rule", {
			title: _("Add firewall rule"),
			family: me.family,
			listeners: {
				scope: me,
				submit: function(c, values) {
					var nextRowNum = this.store.getCount();
					Ext.apply(values, {
						uuid: OMV.UUID_UNDEFINED,
						rulenum: nextRowNum
					});
					this.store.addData(values);
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
			family: me.family,
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

	afterDeletion: function(success) {
		// Do not reload the store but update the 'rulenum' fields.
		this.updateRuleNums();
	},

	afterMoveRows: function(records, index) {
		var me = this;
		me.updateRuleNums(); // Update the 'rulenum' fields.
		me.callParent(arguments);
	},

	onApplyButton: function() {
		var me = this;
		// Get the rules.
		var params = me.getValues();
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			relayErrors: false,
			callback: function(id, success, response) {
				this.store.reload();
			},
			rpcData: {
				service: "Iptables",
				method: (me.family == "inet") ? "setRules" : "setRules6",
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
			record.commit();
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
