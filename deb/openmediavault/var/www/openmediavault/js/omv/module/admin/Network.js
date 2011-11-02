/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
// require("js/omv/NavigationPanel.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/util/Format.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/FormPanelDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/CheckboxGrid.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.System.Network");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "network", {
	text: "Network",
	icon: "images/network.png",
	position: 30
});

/**
 * @class OMV.Module.System.Network.General
 * @derived OMV.FormPanelExt
 */
OMV.Module.System.Network.General = function(config) {
	var initialConfig = {
		rpcService: "Network",
 		rpcGetMethod: "getGeneralSettings",
		rpcSetMethod: "setGeneralSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.General.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.General, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "General settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "hostname",
				fieldLabel: "Server name",
				vtype: "hostname",
				allowBlank: false
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "network", {
	cls: OMV.Module.System.Network.General,
	position: 10,
	title: "General"
});

/**
 * @class OMV.Module.System.Network.IfaceGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.System.Network.IfaceGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "85093f5d-9f9f-45bf-a46f-ead6bc36884a",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Name",
				sortable: true,
				dataIndex: "devicename",
				id: "devicename",
				width: 45
			},{
				header: "Method",
				sortable: true,
				dataIndex: "method",
				id: "method",
				renderer: OMV.util.Format.arrayRenderer([
					[ "dhcp","DHCP" ],
					[ "static","Static" ]
				]),
				width: 45
			},{
				header: "Address",
				sortable: true,
				dataIndex: "address",
				id: "address"
			},{
				header: "Netmask",
				sortable: true,
				dataIndex: "netmask",
				id: "netmask"
			},{
				header: "Gateway",
				sortable: true,
				dataIndex: "gateway",
				id: "gateway"
			},{
				header: "MAC address",
				sortable: true,
				dataIndex: "ether",
				id: "ether"
			},{
				header: "MTU",
				sortable: true,
				dataIndex: "mtu",
				id: "mtu",
				width: 45
			},{
				header: "Link",
				sortable: true,
				dataIndex: "link",
				id: "link",
				renderer: function(val, cell, record, row, col, store) {
					switch (val) {
					case true:
						img = "iflinkyes.png";
						break;
					default:
						img = "iflinkno.png";
						break;
					}
					return "<img border='0' src='images/" + img +
					  "' alt='" + val + "'>";
				},
				scope: this,
				align: "center",
				width: 45
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.IfaceGridPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.System.Network.IfaceGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("Network", "enumerateDevicesList"),
			reader: new Ext.data.JsonReader({
				idProperty: "devicename",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "devicename" },
					{ name: "method" },
					{ name: "address" },
					{ name: "netmask" },
					{ name: "gateway" },
					{ name: "ether" },
					{ name: "mtu" },
					{ name: "state" },
					{ name: "link" },
					{ name: "type" },
					{ name: "_used" },
					{ name: "_readOnly" }
				]
			})
		});
		OMV.Module.System.Network.IfaceGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.System.Network.IfaceGridPanel.superclass.
		  initToolbar.apply(this);
		// Override 'Add' button in top toolbar
		var item = tbar.get(0);
		item.setIcon("images/nic-add.png");
		// Override 'Edit' button in top toolbar
		item = tbar.get(1);
		item.setIcon("images/nic-edit.png");
		// Add 'Identify' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-identify",
			xtype: "button",
			text: "Identify",
			icon: "images/nic-identify.png",
			handler: this.cbIdentifyBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.System.Network.IfaceGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		var tbarBtnName = [ "edit", "delete", "identify" ];
		var tbarBtnDisabled = {
			"edit": true,
			"delete": true,
			"identify": true
		};
		var records = model.getSelections();
		// Disable 'Delete' button if the selected interface has a
		// configuration (uuid !== OMV.UUID_UNDEFINED).
		if (records.length <= 0) {
			// Nothing to do here.
		} else if (records.length == 1) {
			tbarBtnDisabled["edit"] = false;
			if (records[0].get("type") == "physical") {
				tbarBtnDisabled["identify"] = false;
			}
			if (records[0].get("uuid") !== OMV.UUID_UNDEFINED) {
				tbarBtnDisabled["delete"] = false;
			}
		} else {
			// Nothing to do here.
		}
		// Disable 'Delete' button if a selected interface is in usage or
		// readonly.
		for (var i = 0; i < records.length; i++) {
			if (true == records[i].get("_used")) {
				tbarBtnDisabled["edit"] = true;
				tbarBtnDisabled["delete"] = true;
			}
			if (true == records[i].get("_readOnly")) {
				tbarBtnDisabled["delete"] = true;
			}
		}
		for (var i = 0; i < tbarBtnName.length; i++) {
			var tbarBtnCtrl = this.getTopToolbar().findById(this.getId() +
			  "-" + tbarBtnName[i]);
			if (!Ext.isEmpty(tbarBtnCtrl)) {
				if (true == tbarBtnDisabled[tbarBtnName[i]]) {
					tbarBtnCtrl.disable();
				} else {
					tbarBtnCtrl.enable();
				}
			}
		}
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.System.Network.BondIfacePropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var dlgClass = null;
		// Display a different dialog depending on the type of the selected
		// interface.
		switch (record.get("type")) {
		case "physical":
			dlgClass = OMV.Module.System.Network.PhyIfacePropertyDialog;
			break;
		case "bond":
			dlgClass = OMV.Module.System.Network.BondIfacePropertyDialog;
			break;
		}
		var wnd = new dlgClass({
			uuid: record.get("uuid"),
			devicename: record.get("devicename"),
			readOnly: record.get("_readOnly"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbIdentifyBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.System.Network.PhyIfaceIdentifyDialog({
			devicename: record.get("devicename")
		});
		wnd.show();
	},

	doDeletion : function(record) {
		var rpcName = null;
		switch (record.get("type")) {
		case "physical":
			rpcName = "deleteIface";
			break;
		case "bond":
			rpcName = "deleteBondIface";
			break;
		}
		OMV.Ajax.request(this.cbDeletionHdl, this, "Network", rpcName,
		  [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "network", {
	cls: OMV.Module.System.Network.IfaceGridPanel,
	position: 20,
	title: "Interfaces"
});

/**
 * @class OMV.Module.System.Network.PhyIfacePropertyDialog
 * @derived OMV.CfgObjectDialog
 * @config uuid The UUID of the network interface device configuration object.
 * @config devicename The name of the network interface device, e.g. eth0.
 */
OMV.Module.System.Network.PhyIfacePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Network",
		rpcGetMethod: "getIface",
		rpcSetMethod: "setIface",
		title: "Edit physical interface",
		height: 255
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.PhyIfacePropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.PhyIfacePropertyDialog,
  OMV.CfgObjectDialog, {
	initComponent : function() {
		OMV.Module.System.Network.PhyIfacePropertyDialog.superclass.
		  initComponent.apply(this, arguments);
		this.on("load", this._updateFormFields, this);
		this.methodCtrl.on("select", this._updateFormFields, this);
		this.form.on("render", this._updateFormFields, this);
	},

	getFormItems : function() {
		this.methodCtrl = new Ext.form.ComboBox({
			name: "method",
			hiddenName: "method",
			fieldLabel: "Method",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "dhcp","DHCP" ],
					[ "static","Static" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "dhcp"
		});
		return [{
			xtype: "textfield",
			name: "devicename",
			fieldLabel: "Name",
			readOnly: true,
			allowBlank: true,
			value: this.devicename
		}, this.methodCtrl, {
			xtype: "textfield",
			name: "address",
			fieldLabel: "Address",
			vtype: "IPv4",
			readOnly: true,
			allowBlank: true
		},{
			xtype: "textfield",
			name: "netmask",
			fieldLabel: "Netmask",
			vtype: "netmask",
			readOnly: true,
			allowBlank: true
		},{
			xtype: "textfield",
			name: "gateway",
			fieldLabel: "Gateway",
			vtype: "IPv4",
			readOnly: true,
			allowBlank: true
		},{
			xtype: "numberfield",
			name: "mtu",
			fieldLabel: "MTU",
			allowBlank: true,
			allowDecimals: false,
			allowNegative: false
		},{
			xtype: "checkbox",
			name: "wol",
			fieldLabel: "Wake-on-LAN",
			checked: false,
			inputValue: 1
		}];
	},

	/**
	 * Private function to update the states of various form fields
	 * depending on the selection of the 'method' field.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("devicename");
		if ((this.uuid !== OMV.UUID_UNDEFINED) && Ext.isDefined(field)) {
			field.setReadOnly(true);
		}
		// Disable/enable the following fields depending on the selected
		// 'method', e.g. 'dhcp' or 'static'. Also update the 'allowBlank'
		// flag.
		var fields = [ "address", "netmask", "gateway" ];
		var readOnly = ("dhcp" === this.methodCtrl.getValue());
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (Ext.isDefined(field)) {
				field.setReadOnly(readOnly);
				// Modify the 'allowBlank' flag for all fields except the
				// field 'gateway'. This is optional in 'static' mode.
				if(fields[i] !== "gateway") {
					field.allowBlank = readOnly;
				}
			}
		}
	}
});

/**
 * @class OMV.Module.System.Network.PhyIfaceIdentifyDialog
 * @derived OMV.FormPanelDialog
 * @config devicename The name of the network interface device, e.g. eth0.
 */
OMV.Module.System.Network.PhyIfaceIdentifyDialog = function(config) {
	var initialConfig = {
		title: "Identify network interface device",
		autoHeight: true,
		hideReset: true,
		mode: "local",
		width: 260
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.PhyIfaceIdentifyDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.PhyIfaceIdentifyDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "hidden",
			name: "devicename",
			value: this.devicename
		},{
			xtype: "numberfield",
			name: "seconds",
			fieldLabel: "Seconds",
			minValue: 1,
			maxValue: 30,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			value: 10,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Length of time in seconds to blink one or more LEDs " +
			  "on the specific ethernet port."
		},{
			xtype: "displayfield",
			hideLabel: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "<img border='0' src='images/warn.png'> Please note " +
			  "that no communication with the system is possible during " +
			  "this test."
		}];
	},

	cbOkBtnHdl : function() {
		var values = this.getValues();
		this.close();
		var dlg = new OMV.ExecCmdDialog({
			title: this.title,
			width: 350,
			rpcService: "Network",
			rpcMethod: "identify",
			rpcArgs: values,
			hideStart: true,
			hideStop: true,
			hideClose: true,
			progress: true,
			listeners: {
				start: function(c) {
					c.show();
				},
				finish: function(c) {
					var value = c.getValue();
					c.close();
					if (value.length > 0) {
						OMV.MessageBox.error(null, value);
					}
				},
				exception: function(c, error) {
					c.close();
					OMV.MessageBox.error(null, error);
				},
				scope: this
			}
		});
		dlg.start();
	}
});

/**
 * @class OMV.Module.System.Network.BondIfacePropertyDialog
 * @derived OMV.CfgObjectDialog
 * @config uuid The UUID of the network interface device configuration object.
 */
OMV.Module.System.Network.BondIfacePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Network",
		rpcGetMethod: "getBondIface",
		rpcSetMethod: "setBondIface",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " bonded interface",
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.BondIfacePropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.BondIfacePropertyDialog,
  OMV.CfgObjectDialog, {
	initComponent : function() {
		OMV.Module.System.Network.BondIfacePropertyDialog.superclass.
		  initComponent.apply(this, arguments);
		this.on("load", this._updateFormFields, this);
		this.methodCtrl.on("select", this._updateFormFields, this);
		this.form.on("render", this._updateFormFields, this);
	},

	getFormConfig : function() {
		return {
			autoScroll: true,
			defaults: {
				anchor: "-" + Ext.getScrollBarWidth()
			}
		};
	},

	getFormItems : function() {
		this.methodCtrl = new Ext.form.ComboBox({
			name: "method",
			hiddenName: "method",
			fieldLabel: "Method",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "dhcp","DHCP" ],
					[ "static","Static" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "dhcp"
		});
		this.bondPrimaryCtrl = new Ext.form.ComboBox({
			name: "bondprimary",
			hiddenName: "bondprimary",
			fieldLabel: "Primary",
			emptyText: "Select a primary device ...",
			mode: "local",
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy("Network", "enumerateBondSlaves",
				  [ this.uuid, false ], false),
				reader: new Ext.data.JsonReader({
					idProperty: "devicename",
					fields: [
						{ name: "devicename" }
					]
				}),
				sortInfo: {
					field: "devicename",
					direction: "ASC"
				}
			}),
			displayField: "devicename",
			valueField: "devicename",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Specifies which slave is the primary device."
		});
		this.bondslavesCtrl = new OMV.form.CheckboxGrid({
			xtype: "checkboxgrid",
			name: "slaves",
			hiddenName: "slaves",
			fieldLabel: "Slaves",
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy("Network", "enumerateBondSlaves",
				  [ this.uuid ], false),
				reader: new Ext.data.JsonReader({
					idProperty: "devicename",
					fields: [
						{ name: "devicename" },
						{ name: "ether" }
					]
				}),
				sortInfo: {
					field: "devicename",
					direction: "ASC"
				}
			}),
			valueField: "devicename",
			stateId: "c4d556df-e7cf-4291-a1f3-2a815c46acf6",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: "Device",
					sortable: true,
					dataIndex: "devicename",
					id: "devicename"
				},{
					header: "MAC address",
					sortable: true,
					dataIndex: "ether",
					id: "ether"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			height: 130,
			frame: true,
			listeners: {
				selectionchange: function(comp, value) {
					var slaves = value.split(comp.separator);
					var store = this.bondPrimaryCtrl.store;
					var oldValue = this.bondPrimaryCtrl.getValue();
					var found = false;
					this.bondPrimaryCtrl.clearValue();
					this.bondPrimaryCtrl.store.removeAll();
					slaves.each(function(slave) {
						  if (!Ext.isEmpty(slave)) {
							  var index = comp.store.findExact(comp.valueField,
								slave);
							  if (-1 !== index) {
								  var record = comp.store.getAt(index);
								  store.add(new store.recordType(record.data))
							  }
							  if (slave === oldValue) {
								  found = true;
							  }
						  }
					  }, this);
					store.sort(store.sortInfo.field, store.sortInfo.direction);
					if (found === true) {
						this.bondPrimaryCtrl.setValue(oldValue);
					}
				},
				scope: this
			}
		});
		return [{
			xtype: "textfield",
			name: "devicename",
			fieldLabel: "Name",
			readOnly: true,
			allowBlank: true,
			hidden: (this.uuid == OMV.UUID_UNDEFINED),
			value: ""
		},
			this.bondslavesCtrl,
			this.methodCtrl,
		{
			xtype: "textfield",
			name: "address",
			fieldLabel: "Address",
			vtype: "IPv4",
			readOnly: true,
			allowBlank: true
		},{
			xtype: "textfield",
			name: "netmask",
			fieldLabel: "Netmask",
			vtype: "netmask",
			readOnly: true,
			allowBlank: true
		},{
			xtype: "textfield",
			name: "gateway",
			fieldLabel: "Gateway",
			vtype: "IPv4",
			readOnly: true,
			allowBlank: true
		},{
			xtype: "numberfield",
			name: "mtu",
			fieldLabel: "MTU",
			allowBlank: true,
			allowDecimals: false,
			allowNegative: false
		},{
			xtype: "checkbox",
			name: "wol",
			fieldLabel: "Wake-on-LAN",
			checked: false,
			inputValue: 1
		},{
			xtype: "fieldset",
			title: "Bond options",
			defaults: {
				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "combo",
				name: "bondmode",
				hiddenName: "bondmode",
				fieldLabel: "Mode",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,"balance-rr" ],
						[ 1,"active-backup" ],
						[ 2,"balance-xor" ],
						[ 3,"broadcast" ],
						[ 4,"802.3ad" ],
						[ 5,"balance-tlb" ],
						[ 6,"balance-alb" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Specifies one of the bonding policies."
			},
				this.bondPrimaryCtrl
			,{
				xtype: "numberfield",
				name: "bondmiimon",
				fieldLabel: "MII monitoring frequency",
				allowBlank: true,
				allowDecimals: false,
				allowNegative: false,
				minValue: 0,
				value: 100,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Specifies the MII link monitoring frequency in milliseconds."
			},{
				xtype: "numberfield",
				name: "bonddowndelay",
				fieldLabel: "Down delay",
				allowBlank: true,
				allowDecimals: false,
				allowNegative: false,
				minValue: 0,
				value: 200,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Specifies the time, in milliseconds, to wait before disabling a slave after a link failure has been detected."
			},{
				xtype: "numberfield",
				name: "bondupdelay",
				fieldLabel: "Up delay",
				allowBlank: true,
				allowDecimals: false,
				allowNegative: false,
				minValue: 0,
				value: 200,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Specifies the time, in milliseconds, to wait before enabling a slave after a link recovery has been detected."
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields
	 * depending on the selection of the 'method' field.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("devicename");
		if ((this.uuid !== OMV.UUID_UNDEFINED) && Ext.isDefined(field)) {
			field.setReadOnly(true);
		}
		// Disable/enable the following fields depending on the selected
		// 'method', e.g. 'dhcp' or 'static'. Also update the 'allowBlank'
		// flag.
		var fields = [ "address", "netmask", "gateway" ];
		var readOnly = ("dhcp" === this.methodCtrl.getValue());
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (Ext.isDefined(field)) {
				field.allowBlank = readOnly;
				field.setReadOnly(readOnly);
			}
		}
	},

	isValid : function() {
		var valid = OMV.Module.System.Network.BondIfacePropertyDialog.
		  superclass.isValid.apply(this, arguments);
		if (valid) {
			var bondslaves = this.findFormField("slaves").getValue();
			if (Ext.isEmpty(bondslaves) || bondslaves.split(/,/).length < 1) {
				valid = false;
				OMV.MessageBox.failure(null, "Incorrect number of selected " +
				  "physical interfaces. You need to select at least one " +
				  "interfaces.");
			}
		}
		return valid;
	}
});

/**
 * @class OMV.Module.System.Network.HostsPanel
 */
OMV.Module.System.Network.HostsPanel = function(config) {
	var initialConfig = {
		rpcService: "Network",
 		rpcGetMethod: "getHostAccessControl",
		rpcSetMethod: "setHostAccessControl"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.HostsPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.System.Network.HostsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "Host access control",
			defaults: {
				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "textarea",
				name: "hostacallow",
				fieldLabel: "Allow",
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "8"
				}
			},{
				xtype: "textarea",
				name: "hostacdeny",
				fieldLabel: "Deny",
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "8"
				}
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "network", {
	cls: OMV.Module.System.Network.HostsPanel,
	position: 50,
	title: "Hosts"
});

/**
 * @class OMV.Module.System.Network.DNSNameServerPanel
 */
OMV.Module.System.Network.DNSNameServerPanel = function(config) {
	var initialConfig = {
		rpcService: "Network",
 		rpcGetMethod: "getDNSNameServers",
		rpcSetMethod: "setDNSNameServers"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.DNSNameServerPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.DNSNameServerPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "DNS server",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "primarydns",
				fieldLabel: "Primary",
				vtype: "IPv4",
				allowBlank: true
			},{
				xtype: "textfield",
				name: "secondarydns",
				fieldLabel: "Secondary",
				vtype: "IPv4",
				allowBlank: true
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "network", {
	cls: OMV.Module.System.Network.DNSNameServerPanel,
	position: 30,
	title: "DNS Server"
});

/**
 * @class OMV.Module.System.Network.FirewallGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.System.Network.FirewallGridPanel = function(config) {
	var initialConfig = {
		hideUp: false,
		hideDown: false,
		hidePagingToolbar: true,
		enableDragDrop: true,
		ddGroup: "firewallgridpanel-dd",
		stateId: "edb8c917-abd1-4b59-a67f-fc4ef3ab8a5f",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Direction",
				sortable: false,
				dataIndex: "chain",
				id: "chain"
			},{
				header: "Action",
				sortable: false,
				dataIndex: "action",
				id: "action"
			},{
				header: "Source",
				sortable: false,
				dataIndex: "source",
				id: "source",
				renderer: function(val, cell, record, row, col, store) {
					if (val == "")
						return "-";
					return val;
				}
			},{
				header: "Port",
				sortable: false,
				dataIndex: "sport",
				id: "sport",
				renderer: function(val, cell, record, row, col, store) {
					if (val == "")
						return "-";
					return val;
				}
			},{
				header: "Destination",
				sortable: false,
				dataIndex: "destination",
				id: "destination",
				renderer: function(val, cell, record, row, col, store) {
					if (val == "")
						return "-";
					return val;
				}
			},{
				header: "Port",
				sortable: false,
				dataIndex: "dport",
				id: "dport",
				renderer: function(val, cell, record, row, col, store) {
					if (val == "")
						return "-";
					return val;
				}
			},{
				header: "Protocol",
				sortable: false,
				dataIndex: "protocol",
				id: "protocol",
				renderer: OMV.util.Format.arrayRenderer([
					[ "tcp","TCP" ],
					[ "udp","UDP" ],
					[ "icmp","ICMP" ],
					[ "all","All" ],
					[ "!tcp","Not TCP" ],
					[ "!udp","Not UDP" ],
					[ "!icmp","Not ICMP" ]
				])
			},{
				header: "Comment",
				sortable: false,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.FirewallGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.FirewallGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			sortInfo: {
				field: "rulenum",
				direction: "ASC"
			},
			proxy: new OMV.data.DataProxy("Iptables", "getRules"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
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
			listeners: {
				load: function(store, records, options) {
					var tbarBtnCtrl = this.getTopToolbar().findById(
					  this.getId() + "-commit");
					tbarBtnCtrl.disable();
				},
				add: function(store, records, index) {
					var tbarBtnCtrl = this.getTopToolbar().findById(
					  this.getId() + "-commit");
					tbarBtnCtrl.enable();
				},
				update: function(store, record, operation) {
					var tbarBtnCtrl = this.getTopToolbar().findById(
					  this.getId() + "-commit");
					tbarBtnCtrl.enable();
				},
				remove: function(store, record, index) {
					var tbarBtnCtrl = this.getTopToolbar().findById(
					  this.getId() + "-commit");
					tbarBtnCtrl.enable();
				},
				scope: this
			}
		});
		OMV.Module.System.Network.FirewallGridPanel.superclass.
		  initComponent.apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.System.Network.FirewallGridPanel.superclass.
		  initToolbar.call(this);
		// Add 'Commit' button to top toolbar
		tbar.insert(5, {
			id: this.getId() + "-commit",
			xtype: "button",
			text: "Commit",
			icon: "images/commit.png",
			handler: this.cbCommitBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	onRender : function() {
		OMV.Module.System.Network.FirewallGridPanel.superclass.onRender.
		  apply(this, arguments);
		// Create new drop target
		this.dropTarget = new Ext.dd.DropTarget(this.container, {
			ddGroup: this.ddGroup,
			copy: false,
			notifyDrop: function(dd, e, data){
				var sm = dd.grid.getSelectionModel();
				var records = sm.getSelections();
				var index = dd.getDragData(e).rowIndex;
				dd.grid.doMoveRows(records, index);
			}
		});
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.System.Network.FirewallPropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
			ruleNum: this.store.getTotalCount() + 1,
			listeners: {
				submit: function(dlg, data) {
					var lastRowNum = this.store.getCount();
					// Update the row number
					data.rulenum = lastRowNum;
					// Insert new record
					var record = new this.store.recordType(data);
					record.markDirty();
					this.store.insert(lastRowNum, record);
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.System.Network.FirewallPropertyDialog({
			uuid: record.get("uuid"),
			listeners: {
				submit: function(dlg, data) {
					// Update the selected record
					record.beginEdit();
					for (var prop in data) {
						record.set(prop, data[prop]);
					}
					record.endEdit();
				},
				scope: this
			}
		});
		wnd.setValues(record.data);
		wnd.show();
	},

	doDeletion : function(record) {
		this.store.remove(record);
		this.cbDeletionHdl.call(this, null, null, null);
	},

	afterDeletion : function() {
		// Do not reload the store but update the 'rulenum' fields
		this._updateRuleNums();
	},

	afterMoveRows : function(records, index) {
		OMV.Module.System.Network.FirewallGridPanel.superclass.afterMoveRows.
		  apply(this, arguments);
		// Update the 'rulenum' fields
		this._updateRuleNums();
	},

	cbCommitBtnHdl : function() {
		var records = this.store.getRange();
		var values = [];
		for (var i = 0; i < records.length; i++) {
			values.push(records[i].data);
		}
		OMV.Ajax.request(function(id, response, error) {
			  if (error === null) {
				  OMV.MessageBox.success();
				  this.store.reload();
			  } else {
				  OMV.MessageBox.error(null, error);
			  }
		  }, this, "Iptables", "setRules", [ values ]);
	},

	_updateRuleNums : function() {
		for (var i = 0; i < this.store.getTotalCount() - 1; i++) {
			var record = this.store.getAt(i);
			record.beginEdit();
			record.set("rulenum", i);
			record.endEdit();
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "network", {
	cls: OMV.Module.System.Network.FirewallGridPanel,
	position: 60,
	title: "Firewall"
});

/**
 * @class OMV.Module.System.Network.FirewallPropertyDialog
 * @derived OMV.CfgObjectDialog
 * @param uuid The uuid of the firewall rule
 */
OMV.Module.System.Network.FirewallPropertyDialog = function(config) {
	var initialConfig = {
		mode: "local",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " firewall rule",
		width: 500,
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.FirewallPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.FirewallPropertyDialog,
  OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		}
	},

	getFormItems : function() {
		return [{
			xtype: "hidden",
			name: "rulenum",
			value: -1 // Default
		},{
			xtype: "combo",
			name: "chain",
			hiddenName: "chain",
			fieldLabel: "Direction",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "INPUT","INPUT" ],
					[ "OUTPUT","OUTPUT" ]
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
			hiddenName: "action",
			fieldLabel: "Action",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "ACCEPT","ACCEPT" ],
					[ "REJECT","REJECT" ],
					[ "DROP","DROP" ],
					[ "LOG","LOG" ]
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
			fieldLabel: "Source",
			vtype: "IPv4Fw",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Source address can be either a network IP address (with /mask), a IP range or a plain IP address. A '!' argument before the address specification inverts the sense of the address."
		},{
			xtype: "textfield",
			name: "sport",
			fieldLabel: "Source port",
			vtype: "portFw",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Match if the source port is one of the given ports. E.g. 21 or !443 or 1024-65535."
		},{
			xtype: "textfield",
			name: "destination",
			fieldLabel: "Destination",
			vtype: "IPv4Fw",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Destination address can be either a network IP address (with /mask), a IP range or a plain IP address. A '!' argument before the address specification inverts the sense of the address."
		},{
			xtype: "textfield",
			name: "dport",
			fieldLabel: "Destination port",
			vtype: "portFw",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Match if the destination port is one of the given ports. E.g. 21 or !443 or 1024-65535."
		},{
			xtype: "combo",
			name: "protocol",
			hiddenName: "protocol",
			fieldLabel: "Protocol",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "tcp","TCP" ],
					[ "udp","UDP" ],
					[ "icmp","ICMP" ],
					[ "all","All" ],
					[ "!tcp","Not TCP" ],
					[ "!udp","Not UDP" ],
					[ "!icmp","Not ICMP" ]
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
			fieldLabel: "Extra options",
			allowBlank: true
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true
		}];
	},

	isValid : function() {
		var valid = OMV.Module.System.Network.FirewallPropertyDialog.
		  superclass.isValid.call(this);
		if (!valid) {
			return valid;
		}
		// Do additional checks
		var values = this.getValues();
		if (!Ext.isEmpty(values.sport) && (values.protocol == "all")) {
			this.markInvalid([
				{ id: "protocol", msg: "'All' is not allowed" }
			]);
			valid = false;
		}
		if (!Ext.isEmpty(values.dport) && (values.protocol == "all")) {
			this.markInvalid([
				{ id: "protocol", msg: "'All' is not allowed" }
			]);
			valid = false;
		}
		return valid;
	}
});

/**
 * @class OMV.Module.System.Network.DNSServiceDiscoveryPanel
 */
OMV.Module.System.Network.DNSServiceDiscoveryPanel = function(config) {
	var initialConfig = {
		rpcService: "Network",
 		rpcGetMethod: "getDNSServiceDiscovery",
		rpcSetMethod: "setDNSServiceDiscovery"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.DNSNameServerPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.DNSServiceDiscoveryPanel,
  OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "Zeroconf",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: "Enable",
				checked: true,
				inputValue: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Enable advertising of services via mDNS/DNS-SD."
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "network", {
	cls: OMV.Module.System.Network.DNSServiceDiscoveryPanel,
	position: 40,
	title: "DNS Service Discovery"
});
