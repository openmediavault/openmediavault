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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/util/Format.js")
// require("js/omv/window/Execute.js")
// require("js/omv/form/field/CheckboxGrid.js")
// require("js/omv/form/field/Password.js")
// require("js/omv/toolbar/Tip.js")

/**
 * @class OMV.module.admin.system.network.interface.window.Generic
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.system.network.interface.window.Generic", {
	extend: "OMV.workspace.window.Form",
	requires: [
	    "OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "Network",
	plugins: [{
		ptype: "configobject"
	}],
	height: 450,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 * @param devicename The name of the network interface device, e.g. eth0.
	 *   Required.
	 */

	getFormConfig: function() {
		var me = this;
		var correlations = [];
		var config = me.getFormSectionConfig();
		Ext.Array.each(config, function(item) {
			Ext.Array.push(correlations, item.correlations);
		});
		return {
			plugins: [{
				ptype: "linkedfields",
				correlations: correlations
			}]
		};
	},

	getFormSectionConfig: function(name) {
		return [{
			id: "general",
			position: 10,
			title: _("General settings"),
			correlations: []
		},{
			id: "ipv4",
			position: 20,
			title: _("IPv4"),
			correlations: [{
				name: [
					"address",
					"netmask"
				],
				conditions: [
					{ name: "method", value: "static" }
				],
				properties: [
					"!allowBlank",
					"!readOnly"
				]
			},{
				name: "gateway", // Optional in 'Static' mode.
				conditions: [
					{ name: "method", value: "static" }
				],
				properties: "!readOnly"
			}]
		},{
			id: "ipv6",
			position: 30,
			title: _("IPv6"),
			correlations: [{
				name: [
					"address6",
					"netmask6"
				],
				conditions: [
					{ name: "method6", value: "static" }
				],
				properties: [
					"!allowBlank",
					"!readOnly"
				]
			},{
				name: "gateway6", // Optional in 'Static' mode.
				conditions: [
					{ name: "method6", value: "static" }
				],
				properties: "!readOnly"
			}]
		},{
			id: "advanced",
			position: 40,
			title: _("Advanced settings"),
			correlations: []
		}];
	},

	getFormItemsBySection: function(name) {
		var me = this;
		var items = [];
		switch (name) {
		case "general":
			Ext.Array.push(items, [{
				xtype: "textfield",
				name: "devicename",
				fieldLabel: _("Device"),
				readOnly: true,
				allowBlank: true,
				value: me.devicename
			},{
				xtype: "textfield",
				name: "comment",
				fieldLabel: _("Comment"),
				allowBlank: true
			}]);
			break;
		case "ipv4":
			Ext.Array.push(items, [{
				xtype: "combo",
				name: "method",
				fieldLabel: _("Method"),
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: [
						[ "manual", _("Disabled") ],
						[ "dhcp", _("DHCP") ],
						[ "static", _("Static") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "manual"
			},{
				xtype: "textfield",
				name: "address",
				fieldLabel: _("Address"),
				vtype: "IPv4",
				readOnly: true,
				allowBlank: true
			},{
				xtype: "textfield",
				name: "netmask",
				fieldLabel: _("Netmask"),
				vtype: "netmask",
				readOnly: true,
				allowBlank: true
			},{
				xtype: "textfield",
				name: "gateway",
				fieldLabel: _("Gateway"),
				vtype: "IPv4",
				readOnly: true,
				allowBlank: true
			}]);
			break;
		case "ipv6":
			Ext.Array.push(items, [{
				xtype: "combo",
				name: "method6",
				fieldLabel: _("Method"),
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: [
						[ "manual", _("Disabled") ],
						[ "dhcp", _("DHCP") ],
						[ "auto", _("Auto") ],
						[ "static", _("Static") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "manual"
			},{
				xtype: "textfield",
				name: "address6",
				fieldLabel: _("Address"),
				vtype: "IPv6",
				readOnly: true,
				allowBlank: true
			},{
				xtype: "numberfield",
				name: "netmask6",
				fieldLabel: _("Prefix length"),
				allowBlank: true,
				allowDecimals: false,
				minValue: 0,
				maxValue: 128,
				value: 64
			},{
				xtype: "textfield",
				name: "gateway6",
				fieldLabel: _("Gateway"),
				vtype: "IPv6",
				readOnly: true,
				allowBlank: true
			}]);
			break;
		case "advanced":
			Ext.Array.push(items, [{
				xtype: "textfield",
				name: "dnsnameservers",
				fieldLabel: _("DNS servers"),
				vtype: "IPList",
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("IP addresses of domain name servers used to resolve host names.")
				}]
			},{
				xtype: "textfield",
				name: "dnssearch",
				fieldLabel: _("Search domains"),
				vtype: "domainnameList",
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Domains used when resolving host names.")
				}]
			},{
				xtype: "numberfield",
				name: "mtu",
				fieldLabel: _("MTU"),
				allowBlank: false,
				allowDecimals: false,
				minValue: 0,
				maxValue: 65535,
				value: 0,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The maximum transmission unit in bytes to set for the device. Set to 0 to use the default value.")
				}]
			},{
				xtype: "checkbox",
				name: "wol",
				fieldLabel: _("Wake-on-LAN"),
				checked: false
			}]);
			break;
		}
		return items;
	},

	getFormItems: function() {
		var me = this;
		var items = [];
		var config = me.getFormSectionConfig();
		var coll = new Ext.util.MixedCollection();
		Ext.Array.each(config, function(item) {
			coll.add(item.id, item);
		});
		coll.sort("position", "ASC");
		coll.eachKey(function(key, item) {
			Ext.Array.push(items, {
				xtype: "fieldset",
				title: item.title,
				fieldDefaults: {
					labelSeparator: ""
				},
				items: me.getFormItemsBySection(key)
			});
		});
		return items;
	}
});

/**
 * @class OMV.module.admin.system.network.interface.window.Ethernet
 * @derived OMV.module.admin.system.network.interface.window.Generic
 */
Ext.define("OMV.module.admin.system.network.interface.window.Ethernet", {
	extend: "OMV.module.admin.system.network.interface.window.Generic",

	rpcGetMethod: "getEthernetIface",
	rpcSetMethod: "setEthernetIface",

	getFormItemsBySection: function(name) {
		var me = this;
		var items = me.callParent(arguments);
		switch (name) {
		case "general":
			if (me.uuid == OMV.UUID_UNDEFINED) {
				items = [{
					xtype: "combo",
					name: "devicename",
					fieldLabel: _("Device"),
					emptyText: _("Select a device ..."),
					queryMode: "local",
					store: Ext.create("OMV.data.Store", {
						autoLoad: true,
						model: OMV.data.Model.createImplicit({
							idProperty: "devicename",
							fields: [
								{ name: "devicename", type: "string" },
								{ name: "description", type: "string" }
							]
						}),
						proxy: {
							type: "rpc",
							rpcData: {
								service: "Network",
								method: "getEthernetCandidates"
							}
						},
						sorters: [{
							direction: "ASC",
							property: "devicename"
						}]
					}),
					displayField: "description",
					valueField: "devicename",
					allowBlank: false,
					forceSelection: true,
					triggerAction: "all"
				},{
					xtype: "textfield",
					name: "comment",
					fieldLabel: _("Comment"),
					allowBlank: true
				}];
			}
			break;
		}
		return items;
	}
});

/**
 * @class OMV.module.admin.system.network.interface.window.Bond
 * @derived OMV.module.admin.system.network.interface.window.Generic
 */
Ext.define("OMV.module.admin.system.network.interface.window.Bond", {
	extend: "OMV.module.admin.system.network.interface.window.Generic",
	uses: [
		"OMV.form.field.CheckboxGrid"
	],

	width: 500,
	rpcGetMethod: "getBondIface",
	rpcSetMethod: "setBondIface",

	getFormSectionConfig: function() {
		var me = this;
		var config = me.callParent(arguments);
		Ext.Array.push(config, {
			id: "bond",
			position: 15,
			title: _("Bond"),
			correlations: [{
				name: "bondprimary",
				conditions: [
					{ name: "bondmode", value: [ 1, 5, 6 ] }
				],
				properties: [
					"!allowBlank"
				]
			}]
		});
		return config;
	},

	getFormItemsBySection: function(name) {
		var me = this;
		var items = me.callParent(arguments);
		switch (name) {
		case "bond":
			Ext.Array.push(items, [{
				xtype: "checkboxgridfield",
				name: "slaves",
				fieldLabel: _("Slaves"),
				height: 105,
				minSelections: 1,
				allowBlank: false,
				valueField: "devicename",
				store: Ext.create("OMV.data.Store", {
					autoLoad: true,
					model: OMV.data.Model.createImplicit({
						idProperty: "devicename",
						fields: [
							{ name: "devicename", type: "string" },
							{ name: "ether", type: "string" }
						]
					}),
					proxy: {
						type: "rpc",
						rpcData: {
							service: "Network",
							method: "enumerateBondSlaves"
						},
						extraParams: {
							uuid: me.uuid,
							unused: true
						},
						appendSortParams: false
					},
					sorters: [{
						direction: "ASC",
						property: "devicename"
					}]
				}),
				gridConfig: {
					stateful: true,
					stateId: "0c92444c-a911-11e2-ba78-00221568ca88",
					columns: [{
						xtype: "textcolumn",
						text: _("Device"),
						sortable: true,
						dataIndex: "devicename",
						stateId: "devicename",
						flex: 1
					},{
						xtype: "textcolumn",
						text: _("Type"),
						sortable: true,
						dataIndex: "type",
						stateId: "type",
						flex: 1
					},{
						xtype: "textcolumn",
						text: _("MAC address"),
						sortable: true,
						dataIndex: "ether",
						stateId: "ether",
						flex: 1
					}]
				},
				listeners: {
					scope: me,
					selectionchange: function(grid, model, selected, value) {
						me.updatePrimaryField();
					}
				}
			},{
				xtype: "combo",
				name: "bondmode",
				fieldLabel: _("Mode"),
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: [
						[ 0, "balance-rr" ],
						[ 1, "active-backup" ],
						[ 2, "balance-xor" ],
						[ 3, "broadcast" ],
						[ 4, "802.3ad" ],
						[ 5, "balance-tlb" ],
						[ 6, "balance-alb" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 1,
				listeners: {
					scope: me,
					select: function(combo, records) {
						me.updatePrimaryField();
					}
				},
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies one of the bonding policies.")
				}]
			},{
				xtype: "combo",
				name: "bondprimary",
				fieldLabel: _("Primary"),
				emptyText: _("Select a device ..."),
				queryMode: "local",
				store: Ext.create("OMV.data.Store", {
					model: OMV.data.Model.createImplicit({
						identifier: "empty",
						idProperty: "devicename",
						fields: [
							{ name: "text", type: "string" },
							{ name: "devicename", type: "string" }
						]
					}),
					data: [{
						text: _("None"),
						devicename: ""
					}],
					sorters: [{
						direction: "ASC",
						property: "devicename"
					}]
				}),
				displayField: "text",
				valueField: "devicename",
				editable: false,
// Do not force selection, otherwise the value will not be displayed
// when the dialog is displayed in edit mode.
//				forceSelection: true,
				triggerAction: "all",
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies which slave is the primary device.")
				}]
			},{
				xtype: "numberfield",
				name: "bondmiimon",
				fieldLabel: _("MII monitoring frequency"),
				allowBlank: true,
				allowDecimals: false,
				minValue: 0,
				value: 100,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the MII link monitoring frequency in milliseconds.")
				}]
			},{
				xtype: "numberfield",
				name: "bonddowndelay",
				fieldLabel: _("Down delay"),
				allowBlank: true,
				allowDecimals: false,
				minValue: 0,
				value: 200,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the time, in milliseconds, to wait before disabling a slave after a link failure has been detected.")
				}]
			},{
				xtype: "numberfield",
				name: "bondupdelay",
				fieldLabel: _("Up delay"),
				allowBlank: true,
				allowDecimals: false,
				minValue: 0,
				value: 200,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the time, in milliseconds, to wait before enabling a slave after a link recovery has been detected.")
				}]
			}]);
			break;
		}
		return items;
	},

	updatePrimaryField: function() {
		var field;
		// Get the 'slaves' field component.
		field = this.findField("slaves");
		var slaves = field.getValue();
		// Get the 'bondmode' field component.
		field = this.findField("bondmode");
		var bondmode = field.getValue();
		// Get the 'bondprimary' field component.
		field = this.findField("bondprimary");
		var bondprimary = field.getValue();
		// Clear selected value and the whole store.
		field.clearValue();
		field.store.removeAll();
		// Prepare the data to be displayed in the combobox.
		// The primary option is only valid for active-backup(1),
		// balance-tlb (5) and balance-alb (6) mode.
		var data = [];
		if (!Ext.Array.contains([ 1, 5, 6 ], bondmode)) {
			Ext.Array.push(data, [{
				text: _("None"),
				devicename: ""
			}]);
			bondprimary = "";
		}
		if (Ext.Array.contains([ 1, 5, 6 ], bondmode)) {
			Ext.Array.each(slaves, function(slave) {
				Ext.Array.push(data, [{
					text: slave,
					devicename: slave
				}]);
			});
		}
		field.store.loadData(data);
		// Reselect the old value if it is still in the list.
		if (field.findRecordByValue(bondprimary))
			field.setValue(bondprimary);
	}
});

/**
 * @class OMV.module.admin.system.network.interface.window.Vlan
 * @derived OMV.module.admin.system.network.interface.window.Generic
 */
Ext.define("OMV.module.admin.system.network.interface.window.Vlan", {
	extend: "OMV.module.admin.system.network.interface.window.Generic",

	rpcGetMethod: "getVlanIface",
	rpcSetMethod: "setVlanIface",

	getFormSectionConfig: function() {
		var me = this;
		var config = me.callParent(arguments);
		Ext.Array.push(config, {
			id: "vlan",
			position: 15,
			title: _("VLAN"),
			correlations: []
		});
		return config;
	},

	getFormItemsBySection: function(name) {
		var me = this;
		var items = me.callParent(arguments);
		switch (name) {
		case "advanced":
			Ext.apply(Ext.Array.findObject(items, "name", "wol"),{
				hidden: true
			});
			break;
		case "vlan":
			Ext.Array.push(items, [{
				xtype: "combo",
				name: "vlanrawdevice",
				fieldLabel: _("Parent device"),
				emptyText: _("Select a device ..."),
				queryMode: "local",
				store: Ext.create("OMV.data.Store", {
					autoLoad: true,
					model: OMV.data.Model.createImplicit({
						idProperty: "devicename",
						fields: [
							{ name: "devicename", type: "string" },
							{ name: "description", type: "string" }
						]
					}),
					proxy: {
						type: "rpc",
						appendSortParams: false,
						rpcData: {
							service: "Network",
							method: "getVlanCandidates"
						}
					},
					sorters: [{
						direction: "ASC",
						property: "devicename"
					}]
				}),
				displayField: "description",
				valueField: "devicename",
				allowBlank: false,
				readOnly: me.uuid !== OMV.UUID_UNDEFINED,
				editable: false,
				forceSelection: true,
				triggerAction: "all"
			},{
				xtype: "numberfield",
				name: "vlanid",
				fieldLabel: _("VLAN id"),
				readOnly: me.uuid !== OMV.UUID_UNDEFINED,
				allowDecimals: false,
				minValue: 1,
				maxValue: 4095,
				value: 1
			}]);
			break;
		}
		return items;
	}
});

/**
 * @class OMV.module.admin.system.network.interface.window.Wifi
 * @derived OMV.module.admin.system.network.interface.window.Generic
 */
Ext.define("OMV.module.admin.system.network.interface.window.Wifi", {
	extend: "OMV.module.admin.system.network.interface.window.Generic",
	requires: [
		"OMV.form.field.Password",
	],

	rpcGetMethod: "getWirelessIface",
	rpcSetMethod: "setWirelessIface",

	getFormSectionConfig: function() {
		var me = this;
		var config = me.callParent(arguments);
		Ext.Array.push(config, {
			id: "wifi",
			position: 15,
			title: _("Wi-Fi"),
			correlations: []
		});
		return config;
	},

	getFormItemsBySection: function(name) {
		var me = this;
		var items = me.callParent(arguments);
		switch (name) {
		case "general":
			if (me.uuid == OMV.UUID_UNDEFINED) {
				items = [{
					xtype: "combo",
					name: "devicename",
					fieldLabel: _("Device"),
					emptyText: _("Select a device ..."),
					queryMode: "local",
					store: Ext.create("OMV.data.Store", {
						autoLoad: true,
						model: OMV.data.Model.createImplicit({
							idProperty: "devicename",
							fields: [
								{ name: "devicename", type: "string" },
								{ name: "description", type: "string" }
							]
						}),
						proxy: {
							type: "rpc",
							rpcData: {
								service: "Network",
								method: "getWirelessCandidates"
							}
						},
						sorters: [{
							direction: "ASC",
							property: "devicename"
						}]
					}),
					displayField: "description",
					valueField: "devicename",
					allowBlank: false,
					forceSelection: true,
					triggerAction: "all"
				},{
					xtype: "textfield",
					name: "comment",
					fieldLabel: _("Comment"),
					allowBlank: true
				}];
			}
			break;
		case "wifi":
			Ext.Array.push(items, [{
				xtype: "textfield",
				name: "wpassid",
				fieldLabel: _("SSID"),
				allowBlank: false,
				value: ""
			},{
				xtype: "passwordfield",
				name: "wpapsk",
				fieldLabel: _("Password"),
				allowBlank: false,
				autoComplete: false,
				value: ""
			}]);
			break;
		}
		return items;
	}
});

/**
 * @class OMV.module.admin.system.network.interface.window.Bridge
 * @derived OMV.module.admin.system.network.interface.window.Generic
 */
Ext.define("OMV.module.admin.system.network.interface.window.Bridge", {
	extend: "OMV.module.admin.system.network.interface.window.Generic",
	uses: [
		"OMV.form.field.CheckboxGrid"
	],


	width: 500,
	rpcGetMethod: "getBridgeIface",
	rpcSetMethod: "setBridgeIface",

	getFormSectionConfig: function() {
		var me = this;
		var config = me.callParent(arguments);
		Ext.Array.push(config, {
			id: "bridge",
			position: 15,
			title: _("Bridge"),
			correlations: []
		});
		return config;
	},

	getFormItemsBySection: function(name) {
		var me = this;
		var items = me.callParent(arguments);
		switch (name) {
		case "bridge":
			Ext.Array.push(items, [{
				xtype: "checkboxgridfield",
				name: "slaves",
				fieldLabel: _("Interfaces"),
				height: 105,
				minSelections: 1,
				allowBlank: false,
				valueField: "devicename",
				store: Ext.create("OMV.data.Store", {
					autoLoad: true,
					model: OMV.data.Model.createImplicit({
						idProperty: "devicename",
						fields: [
							{ name: "devicename", type: "string" },
							{ name: "ether", type: "string" }
						]
					}),
					proxy: {
						type: "rpc",
						rpcData: {
							service: "Network",
							method: "enumerateBridgeSlaves"
						},
						extraParams: {
							uuid: me.uuid,
							unused: true
						},
						appendSortParams: false
					},
					sorters: [{
						direction: "ASC",
						property: "devicename"
					}]
				}),
				gridConfig: {
					stateful: true,
					stateId: "c8b68148-9abc-11ea-a11c-53b783ac356b",
					columns: [{
						xtype: "textcolumn",
						text: _("Device"),
						sortable: true,
						dataIndex: "devicename",
						stateId: "devicename",
						flex: 1
					},{
						xtype: "textcolumn",
						text: _("Type"),
						sortable: true,
						dataIndex: "type",
						stateId: "type",
						flex: 1
					},{
						xtype: "textcolumn",
						text: _("MAC address"),
						sortable: true,
						dataIndex: "ether",
						stateId: "ether",
						flex: 1
					}]
				}
			}]);
			break;
		}
		return items;
	}
});

/**
 * @class OMV.module.admin.system.network.interface.Identify
 * @derived OMV.workspace.window.Form
 * @param devicename The name of the network interface device, e.g. eth0.
 */
Ext.define("OMV.module.admin.system.network.interface.Identify", {
	extend: "OMV.workspace.window.Form",
	uses: [
		"OMV.window.Execute",
		"OMV.toolbar.Tip"
	],

	title: _("Identify network interface device"),
	okButtonText: _("Start"),
	hideResetButton: true,
	mode: "local",

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		// Add the tip toolbar at the bottom of the window.
		me.addDocked({
			xtype: "tiptoolbar",
			dock: "bottom",
			ui: "footer",
			icon: OMV.toolbar.Tip.WARNING,
			text: _("Please note that no communication with the system is possible during this test.")
		});
	},

	getFormItems: function() {
		return [{
			xtype: "numberfield",
			name: "seconds",
			fieldLabel: _("Seconds"),
			minValue: 1,
			maxValue: 30,
			allowDecimals: false,
			allowBlank: false,
			value: 10,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Length of time in seconds to blink one or more LEDs on the specific ethernet port.")
			}]
		}];
	},

	onOkButton: function() {
		var me = this;
		var values = me.getValues();
		// Execute the interface identify RPC.
		Ext.create("OMV.window.Execute", {
			title: me.title,
			width: 350,
			rpcService: "Network",
			rpcMethod: "identify",
			rpcParams: {
				devicename: me.devicename,
				seconds: values.seconds
			},
			hideStartButton: true,
			hideStopButton: true,
			hideCloseButton: true,
			progress: true,
			listeners: {
				start: function(c) {
					// Close the dialog window.
					me.close();
					// Show the execute dialog window.
					c.show();
				},
				finish: function(c) {
					var value = c.getValue();
					c.close();
					if(value.length > 0) {
						OMV.MessageBox.error(null, value);
					}
				},
				exception: function(c, error) {
					c.close();
					OMV.MessageBox.error(null, error);
				},
				// Do not set scope to 'me', otherwise the listeners will get
				// invalid when the 'me' dialog is closed (see start listener).
				scope: null
			}
		}).start();
	}
});

/**
 * @class OMV.module.admin.system.network.interface.Interfaces
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.system.network.interface.Interfaces", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format",
		"OMV.module.admin.system.network.interface.Identify",
		"OMV.module.admin.system.network.interface.window.Ethernet",
		"OMV.module.admin.system.network.interface.window.Bond",
		"OMV.module.admin.system.network.interface.window.Vlan",
		"OMV.module.admin.system.network.interface.window.Bridge"
	],
	uses: [
		"Ext.XTemplate"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "85093f5d-9f9f-45bf-a46f-ead6bc36884a",
	columns: [{
		xtype: "textcolumn",
		text: _("Device"),
		sortable: true,
		dataIndex: "devicename",
		stateId: "devicename",
		flex: 1
	},{
		xtype: "textcolumn",
		text: _("Type"),
		sortable: true,
		dataIndex: "type",
		stateId: "type",
		hidden: true,
		flex: 1
	},{
		text: _("Method"),
		sortable: true,
		stateId: "method",
		renderer: function(value, metaData, record) {
			var tpl = Ext.create("Ext.XTemplate",
			  _("IPv4"),': {[this.renderValue(values.method)]}<br/>',
			  _("IPv6"),': {[this.renderValue(values.method6)]}',
			  {
				  renderValue: function(value) {
					  var methods = {
						  manual: _("Disabled"),
						  dhcp: _("DHCP"),
						  auto: _("Auto"),
						  static: _("Static")
					  };
					  return Ext.util.Format.defaultValue(methods[value], "-");
				  }
			  });
			return tpl.apply(record.data);
		},
		flex: 1
	},{
		xtype: "templatecolumn",
		text: _("Address"),
		sortable: true,
		stateId: "address",
		tpl: Ext.String.format(
		  '{0}: {[Ext.util.Format.defaultValue(values.address, "-")]}<br/>' +
		  '{1}: {[Ext.util.Format.defaultValue(values.address6, "-")]}',
		  _("IPv4"), _("IPv6")),
		flex: 1
	},{
		xtype: "templatecolumn",
		text: _("Netmask"),
		sortable: true,
		stateId: "netmask",
		tpl: Ext.String.format(
		  '{0}: {[Ext.util.Format.defaultValue(values.netmask, "-")]}<br/>' +
		  '{1}: {[Ext.util.Format.defaultValue((values.netmask6 < 0) ? "" : values.netmask6, "-")]}',
		  _("IPv4"), _("IPv6")),
		flex: 1
	},{
		xtype: "templatecolumn",
		text: _("Gateway"),
		sortable: true,
		stateId: "gateway",
		tpl: Ext.String.format(
		  '{0}: {[Ext.util.Format.defaultValue(values.gateway, "-")]}<br/>' +
		  '{1}: {[Ext.util.Format.defaultValue(values.gateway6, "-")]}',
		  _("IPv4"), _("IPv6")),
		flex: 1
	},{
		xtype: "numberrangecolumn",
		text: _("MTU"),
		sortable: true,
		dataIndex: "mtu",
		stateId: "mtu",
		width: 60,
		minValue: 1,
		maxValue: 65535
	},{
		xtype: "booleantextcolumn",
		text: _("WOL"),
		sortable: true,
		dataIndex: "wol",
		stateId: "wol",
		width: 60
	},{
		xtype: "textcolumn",
		text: _("Comment"),
		sortable: true,
		hidden: true,
		dataIndex: "comment",
		stateId: "comment"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicename",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "type", type: "string" },
						{ name: "devicename", type: "string" },
						{ name: "method", type: "string" },
						{ name: "address", type: "string" },
						{ name: "netmask", type: "string" },
						{ name: "gateway", type: "string" },
						{ name: "method6", type: "string" },
						{ name: "address6", type: "string" },
						{ name: "netmask6", type: "int" },
						{ name: "gateway6", type: "string" },
						{ name: "dnsnameservers", type: "string" },
						{ name: "dnssearch", type: "string" },
						{ name: "mtu", type: "int" },
						{ name: "wol", type: "boolean" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Network",
						method: "getInterfaceList"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicename"
				}]
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		// Replace the default 'Add' button.
		Ext.Array.erase(items, 0, 1);
		Ext.Array.insert(items, 0, [{
			id: me.getId() + "-add",
			xtype: "splitbutton",
			text: _("Add"),
			iconCls: "x-fa fa-plus",
			handler: function() {
				this.showMenu();
			},
			menu: Ext.create("Ext.menu.Menu", {
				items: [{
					iconCls: "mdi mdi-ethernet",
					text: _("Ethernet"),
					value: "ethernet"
				},{
					iconCls: "mdi mdi-wifi",
					text: _("Wi-Fi"),
					value: "wifi"
				},{
					iconCls: "mdi mdi-link-variant",
					text: _("Bond"),
					value: "bond"
				},{
					iconCls: "mdi mdi-lan",
					text: _("VLAN"),
					value: "vlan"
				},{
					iconCls: "mdi mdi-bridge",
					text: _("Bridge"),
					value: "bridge"
				}],
				listeners: {
					scope: me,
					click: function(menu, item, e, eOpts) {
						this.onAddButton(item.value);
					}
				}
			})
		}]);
		// Add 'Identify' button to top toolbar.
		Ext.Array.insert(items, 2, [{
			id: me.getId() + "-identify",
			xtype: "button",
			text: _("Identify"),
			iconCls: "x-fa fa-search",
			handler: Ext.Function.bind(me.onIdentifyButton, me, [ me ]),
			scope: me,
			disabled: true
		}]);
		return items;
	},

	onSelectionChange: function(model, records) {
		var me = this;
		me.callParent(arguments);
		var tbarBtnDisabled = {
			"edit": true,
			"delete": true,
			"identify": true
		};
		// Disable 'Delete' button if the selected interface has a
		// configuration (uuid !== OMV.UUID_UNDEFINED).
		if (records.length <= 0) {
			// Nothing to do here.
		} else if (records.length == 1) {
			tbarBtnDisabled["edit"] = false;
			if (records[0].get("type") == "ethernet") {
				tbarBtnDisabled["identify"] = false;
			}
			tbarBtnDisabled["delete"] = false;
		} else {
			// Nothing to do here.
		}
		// Disable 'Delete' button if a selected interface is in use or
		// readonly.
		for (var i = 0; i < records.length; i++) {
			if (true == records[i].get("_used")) {
				tbarBtnDisabled["edit"] = true;
				tbarBtnDisabled["delete"] = true;
			}
			if (true == records[i].get("_readonly")) {
				tbarBtnDisabled["delete"] = true;
			}
		}
		// Update the button controls.
		Ext.Object.each(tbarBtnDisabled, function(key, value) {
			this.setToolbarButtonDisabled(key, value);
		}, me);
	},

	onAddButton: function(type) {
		var me = this;
		var clsName, title;
		switch (type) {
		case "ethernet":
			clsName = "OMV.module.admin.system.network.interface.window.Ethernet";
			title = _("Add ethernet connection");
			break;
		case "bond":
			clsName = "OMV.module.admin.system.network.interface.window.Bond";
			title = _("Add bond connection");
			break;
		case "vlan":
			clsName = "OMV.module.admin.system.network.interface.window.Vlan";
			title = _("Add VLAN connection");
			break;
		case "wifi":
			clsName = "OMV.module.admin.system.network.interface.window.Wifi";
			title = _("Add Wi-Fi connection");
			break;
		case "bridge":
			clsName = "OMV.module.admin.system.network.interface.window.Bridge";
			title = _("Add bridge connection");
			break;
		default:
			OMV.MessageBox.error(null, _("Unknown network interface type."));
			break;
		}
		if (Ext.isEmpty(clsName))
			return;
		Ext.create(clsName, {
			title: title,
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function() {
					me.doReload();
				},
				scope: me
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var clsName, title;
		var record = me.getSelected();
		// Display a different dialog depending on the type of the selected
		// interface.
		switch (record.get("type")) {
		case "ethernet":
			clsName = "OMV.module.admin.system.network.interface.window.Ethernet";
			title = _("Edit ethernet connection");
			break;
		case "bond":
			clsName = "OMV.module.admin.system.network.interface.window.Bond";
			title = _("Edit bond connection");
			break;
		case "vlan":
			clsName = "OMV.module.admin.system.network.interface.window.Vlan";
			title = _("Edit VLAN connection");
			break;
		case "wifi":
			clsName = "OMV.module.admin.system.network.interface.window.Wifi";
			title = _("Edit Wi-Fi connection");
			break;
		case "bridge":
			clsName = "OMV.module.admin.system.network.interface.window.Bridge";
			title = _("Edit bridge connection");
			break;
		default:
			OMV.MessageBox.error(null, _("Unknown network interface type."));
			break;
		}
		if (Ext.isEmpty(clsName))
			return;
		Ext.create(clsName, {
			title: title,
			uuid: record.get("uuid"),
			readOnly: record.get("_readonly"),
			listeners: {
				submit: function() {
					me.doReload();
				},
				scope: me
			}
		}).show();
	},

	onIdentifyButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.system.network.interface.Identify", {
			devicename: record.get("devicename")
		}).show();
	},

	doDeletion: function(record) {
		var me = this;
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "Network",
				method: "deleteInterface",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "interfaces",
	path: "/system/network",
	text: _("Interfaces"),
	position: 20,
	className: "OMV.module.admin.system.network.interface.Interfaces"
});
