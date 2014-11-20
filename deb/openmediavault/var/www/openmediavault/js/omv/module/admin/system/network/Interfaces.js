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
// require("js/omv/util/Format.js")
// require("js/omv/window/Execute.js")
// require("js/omv/form/field/CheckboxGrid.js")
// require("js/omv/toolbar/Tip.js")

/**
 * @class OMV.module.admin.system.network.interface.Physical
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.system.network.interface.Physical", {
	extend: "OMV.workspace.window.Form",
	requires: [
	    "OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "Network",
	rpcGetMethod: "getIface",
	rpcSetMethod: "setIface",
	plugins: [{
		ptype: "configobject"
	}],
	height: 340,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 * @param devicename The name of the network interface device, e.g. eth0.
	 *   Required.
	 */

	getFormConfig: function() {
		return {
			plugins: [{
				ptype: "linkedfields",
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
				},{
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
			}]
		}
	},

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textfield",
			name: "devicename",
			fieldLabel: _("Name"),
			readOnly: true,
			allowBlank: true,
			value: me.devicename
		},{
			xtype: "fieldset",
			title: _("IPv4"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
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
			}]
		},{
			xtype: "fieldset",
			title: _("IPv6"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
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
			}]
		},{
			xtype: "numberfield",
			name: "mtu",
			fieldLabel: _("MTU"),
			allowBlank: true,
			allowDecimals: false,
			minValue: 0
		},{
			xtype: "textfield",
			name: "options",
			fieldLabel: _("Options"),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Additional device settings, e.g. 'autoneg off speed 100 duplex full'. See <a href='http://linux.die.net/man/8/ethtool' target='_blank'>manual pages</a> for more details.")
			}]
		},{
			xtype: "checkbox",
			name: "wol",
			fieldLabel: _("Wake-on-LAN"),
			checked: false
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		}];
	}
});

/**
 * @class OMV.module.admin.system.network.interface.Bond
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.system.network.interface.Bond", {
	extend: "OMV.workspace.window.Form",
	uses: [
		"OMV.form.field.CheckboxGrid",
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "Network",
	rpcGetMethod: "getBondIface",
	rpcSetMethod: "setBondIface",
	plugins: [{
		ptype: "configobject"
	}],
	height: 400,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormConfig: function() {
		return {
			plugins: [{
				ptype: "linkedfields",
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
				},{
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
			}]
		}
	},

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textfield",
			name: "devicename",
			fieldLabel: _("Name"),
			readOnly: true,
			allowBlank: true,
			value: ""
		},{
			xtype: "checkboxgridfield",
			name: "slaves",
			fieldLabel: _("Slaves"),
			height: 105,
			minSelections: 1,
			allowBlank: false,
			valueField: "devicename",
			useStringValue: true,
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
					text: _("Device"),
					sortable: true,
					dataIndex: "devicename",
					stateId: "devicename",
					flex: 1
				},{
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
					var field = this.findField("bondprimary");
					var fieldValue = field.getValue();
					// Clear selected value and the whole store.
					field.clearValue();
					field.store.removeAll();
					// Add all checked interfaces to the 'Primary'
					// combobox.
					Ext.Array.each(selected, function(record) {
						field.store.add(record);
					});
					// Reselect the old value if it is still in the list.
					if(field.findRecordByValue(fieldValue))
						field.setValue(fieldValue);
				}
			}
		},{
			xtype: "fieldset",
			title: _("IPv4"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
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
				value: "dhcp"
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
			}]
		},{
			xtype: "fieldset",
			title: _("IPv6"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
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
			}]
		},{
			xtype: "numberfield",
			name: "mtu",
			fieldLabel: _("MTU"),
			allowBlank: true,
			allowDecimals: false,
			minValue: 0
		},{
			xtype: "textfield",
			name: "options",
			fieldLabel: _("Options"),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Additional device settings, e.g. 'autoneg off speed 100 duplex full'. See <a href='http://linux.die.net/man/8/ethtool' target='_blank'>manual pages</a> for more details.")
			}]
		},{
			xtype: "checkbox",
			name: "wol",
			fieldLabel: _("Wake-on-LAN"),
			checked: false
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		},{
			xtype: "fieldset",
			title: _("Bond options"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
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
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies one of the bonding policies.")
				}]
			},{
				xtype: "combo",
				name: "bondprimary",
				fieldLabel: _("Primary"),
				emptyText: _("Select a primary device ..."),
				queryMode: "local",
				store: Ext.create("OMV.data.Store", {
					autoLoad: true,
					model: OMV.data.Model.createImplicit({
						idProperty: "devicename",
						fields: [
							{ name: "devicename", type: "string" }
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
							unused: false
						},
						appendSortParams: false
					},
					sorters: [{
						direction: "ASC",
						property: "devicename"
					}]
				}),
				displayField: "devicename",
				valueField: "devicename",
				allowBlank: false,
				editable: false,
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
			}]
		}];
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

	getFormConfig: function() {
		return {
			dockedItems: [{
				xtype: "tiptoolbar",
				dock: "bottom",
				ui: "footer",
				icon: OMV.toolbar.Tip.WARNING,
				text: _("Please note that no communication with the system is possible during this test.")
			}]
		};
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
				scope: me
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
		"OMV.module.admin.system.network.interface.Physical",
		"OMV.module.admin.system.network.interface.Bond"
	],
	uses: [
		"Ext.XTemplate"
	],

	autoReload: true,
	rememberSelected: true,
	hidePagingToolbar: false,
	stateful: true,
	stateId: "85093f5d-9f9f-45bf-a46f-ead6bc36884a",
	columns: [{
		text: _("Name"),
		sortable: true,
		dataIndex: "devicename",
		stateId: "devicename",
		width: 45
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
						  static: _("Static")
					  };
					  return Ext.util.Format.defaultValue(methods[value], "-");
				  }
			  });
			return tpl.apply(record.data);
		},
		width: 45
	},{
		text: _("Address"),
		sortable: true,
		stateId: "address",
		renderer: function(value, metaData, record) {
			var tpl = Ext.create("Ext.XTemplate",
			  _("IPv4"),': {[Ext.util.Format.defaultValue(values.address, "-")]}<br/>',
			  _("IPv6"),': {[Ext.util.Format.defaultValue(values.address6, "-")]}');
			return tpl.apply(record.data);
		}
	},{
		text: _("Netmask"),
		sortable: true,
		stateId: "netmask",
		renderer: function(value, metaData, record) {
			var tpl = Ext.create("Ext.XTemplate",
			  _("IPv4"),': {[Ext.util.Format.defaultValue(values.netmask, "-")]}<br/>',
			  _("IPv6"),': {[Ext.util.Format.defaultValue((values.netmask6 < 0) ? "" : values.netmask6, "-")]}');
			return tpl.apply(record.data);
		}
	},{
		text: _("Gateway"),
		sortable: true,
		stateId: "gateway",
		renderer: function(value, metaData, record) {
			var tpl = Ext.create("Ext.XTemplate",
			  _("IPv4"),': {[Ext.util.Format.defaultValue(values.gateway, "-")]}<br/>',
			  _("IPv6"),': {[Ext.util.Format.defaultValue(values.gateway6, "-")]}');
			return tpl.apply(record.data);
		}
	},{
		text: _("MAC address"),
		sortable: true,
		dataIndex: "ether",
		stateId: "ether"
	},{
		text: _("MTU"),
		sortable: true,
		dataIndex: "mtu",
		stateId: "mtu",
		width: 45
	},{
		text: _("Link"),
		sortable: true,
		dataIndex: "link",
		stateId: "link",
		align: "center",
		width: 80,
		resizable: false,
		renderer: function(value) {
			switch(value) {
			case true:
				img = "iflinkyes.png";
				break;
			default:
				img = "iflinkno.png";
				break;
			}
			return "<img border='0' src='images/" + img +
			  "' alt='" + value + "'>";
		}
	},{
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
						{ name: "devicename", type: "string" },
						{ name: "method", type: "string" },
						{ name: "address", type: "string" },
						{ name: "netmask", type: "string" },
						{ name: "gateway", type: "string" },
						{ name: "method6", type: "string" },
						{ name: "address6", type: "string" },
						{ name: "netmask6", type: "int" },
						{ name: "gateway6", type: "string" },
						{ name: "ether", type: "string" },
						{ name: "mtu", type: "string" },
						{ name: "state", type: "string" },
						{ name: "link", type: "boolean" },
						{ name: "type", type: "string" },
						{ name: "comment", type: "string" },
						{ name: "_used", type: "boolean" },
						{ name: "_readonly", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Network",
						method: "enumerateDevicesList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		// Override 'Add' button in top toolbar.
		Ext.apply(items[0], {
			icon: "images/add.png"
		});
		// Override 'Edit' button in top toolbar.
		Ext.apply(items[1], {
			icon: "images/edit.png"
		});
		// Add 'Identify' button to top toolbar.
		Ext.Array.insert(items, 2, [{
			id: me.getId() + "-identify",
			xtype: "button",
			text: _("Identify"),
			icon: "images/search.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
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
		if(records.length <= 0) {
			// Nothing to do here.
		} else if(records.length == 1) {
			tbarBtnDisabled["edit"] = false;
			if(records[0].get("type") == "physical") {
				tbarBtnDisabled["identify"] = false;
			}
			if(records[0].get("uuid") !== OMV.UUID_UNDEFINED) {
				tbarBtnDisabled["delete"] = false;
			}
		} else {
			// Nothing to do here.
		}
		// Disable 'Delete' button if a selected interface is in use or
		// readonly.
		for(var i = 0; i < records.length; i++) {
			if(true == records[i].get("_used")) {
				tbarBtnDisabled["edit"] = true;
				tbarBtnDisabled["delete"] = true;
			}
			if(true == records[i].get("_readonly")) {
				tbarBtnDisabled["delete"] = true;
			}
		}
		// Update the button controls.
		Ext.Object.each(tbarBtnDisabled, function(key, value) {
			this.setToolbarButtonDisabled(key, value);
		}, me);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.system.network.interface.Bond", {
			title: _("Add bonded interface"),
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
		var className, title;
		var record = me.getSelected();
		// Display a different dialog depending on the type of the selected
		// interface.
		switch(record.get("type")) {
		case "physical":
			className = "OMV.module.admin.system.network.interface.Physical";
			title = _("Edit physical interface");
			break;
		case "bond":
			className = "OMV.module.admin.system.network.interface.Bond";
			title = _("Edit bonded interface");
			break;
		default:
			OMV.MessageBox.error(null, _("Unknown network interface type."));
			break;
		}
		if(Ext.isEmpty(className))
			return;
		Ext.create(className, {
			title: title,
			uuid: record.get("uuid"),
			devicename: record.get("devicename"),
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
		var rpcMethod;
		switch(record.get("type")) {
		case "physical":
			rpcMethod = "deleteIface";
			break;
		case "bond":
			rpcMethod = "deleteBondIface";
			break;
		default:
			OMV.MessageBox.error(null, _("Unknown network interface type."));
			break;
		}
		if(Ext.isEmpty(rpcMethod))
			return;
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "Network",
				method: rpcMethod,
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
