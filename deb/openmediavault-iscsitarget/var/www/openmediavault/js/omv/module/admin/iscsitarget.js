/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
// require("js/omv/FormPanelExt.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/CfgObjectTabDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/Grid.js")
// require("js/omv/form/FormPanel.js")
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")
// require("js/omv/MessageBox.js")

Ext.ns("OMV.Module.Services.iSCSITgt");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "iscsitgt", {
	text: "iSCSI Target",
	icon: "images/iscsitarget.png"
});

/**
 * @class OMV.Module.Services.iSCSITgt.SettingsPanel
 */
OMV.Module.Services.iSCSITgt.SettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "iSCSITarget",
 		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.SettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.SettingsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "General settings",
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: "Extra options",
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "3",
					cols: "65"
				}
			}]
		},{
			xtype: "fieldset",
			title: "Discovery authentication",
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "iscsitgtauthusergrid",
				id: this.getId() + "-authentication",
				name: "authentication",
				fieldLabel: "Users",
				height: 150
			}]
		}];
	},

	/**
	 * Returns the fields in this form as an object with key/value pairs.
	 */
	getValues : function() {
		var values = OMV.Module.Services.iSCSITgt.SettingsPanel.superclass.
		  getValues.call(this, arguments);
		// Get the values from the 'Discovery authentication' grid
		var ctrl = this.findById(this.getId() + "-authentication");
		if (!Ext.isEmpty(ctrl)) {
			Ext.apply(values, {
				authentication: ctrl.getValues()
			});
		}
		return values;
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "iscsitgt", {
	cls: OMV.Module.Services.iSCSITgt.SettingsPanel,
	title: "Settings",
	position: 10
});

/**
 * @class OMV.Module.Services.iSCSITgt.TargetsGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.iSCSITgt.TargetsGridPanel = function(config) {
	var initialConfig = {
		stateId: "15e18b72-c1e9-11e0-a91c-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "IQN",
				sortable: true,
				dataIndex: "iqn",
				id: "iqn"
			},{
				header: "Alias",
				sortable: true,
				dataIndex: "alias",
				id: "alias"
			},{
				header: "Max. connections",
				sortable: true,
				dataIndex: "maxconnections",
				id: "maxconnections"
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.TargetsGridPanel.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.TargetsGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("iSCSITarget", "getTargetList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "iqn" },
					{ name: "alias" },
					{ name: "maxconnections" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Services.iSCSITgt.TargetsGridPanel.superclass.
		  initComponent.apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.iSCSITgt.TargetPropertyDialog({
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
		var wnd = new OMV.Module.Services.iSCSITgt.TargetPropertyDialog({
			uuid: record.get("uuid"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "iSCSITarget",
		  "deleteTarget", [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "iscsitgt", {
	cls: OMV.Module.Services.iSCSITgt.TargetsGridPanel,
	title: "Targets",
	position: 20
});

/**
 * @class OMV.Module.Services.iSCSITgt.TargetPropertyDialog
 */
OMV.Module.Services.iSCSITgt.TargetPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "iSCSITarget",
		rpcGetMethod: "getTarget",
		rpcSetMethod: "setTarget",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " target",
		width: 600,
		height: 450
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.TargetPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.TargetPropertyDialog,
  OMV.CfgObjectTabDialog, {
	getTabItems : function() {
		return [
			new OMV.Module.Services.iSCSITgt.TargetGeneralPanel,
			new OMV.Module.Services.iSCSITgt.TargetAuthUserGrid,
			new OMV.Module.Services.iSCSITgt.TargetLUNGrid
		];
	}
});

/**
 * @class OMV.Module.Services.iSCSITgt.TargetGeneralPanel
 */
OMV.Module.Services.iSCSITgt.TargetGeneralPanel = function(config) {
	var initialConfig = {
		title: "General",
		autoScroll: true,
		trackResetOnLoad: true,
		defaults: {
			anchor: "-" + Ext.getScrollBarWidth()
		}
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.TargetGeneralPanel.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.TargetGeneralPanel,
  OMV.form.FormPanel, {
	initComponent : function() {
		this.items = [{
			xtype: "textfield",
			name: "identifier",
			fieldLabel: "Identifier",
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The identifier of the target."
		},{
			xtype: "textfield",
			name: "alias",
			fieldLabel: "Alias",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The optional alias of the target."
		},{
			xtype: "combo",
			name: "headerdigest",
			hiddenName: "headerdigest",
			fieldLabel: "Header digest",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "None","None" ],
					[ "CRC32C","CRC32C" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If set to 'CRC32C' and the initiator is configured accordingly, the integrity of an iSCSI PDU's header segments will be protected by a CRC32C checksum.",
			value: "None"
		},{
			xtype: "combo",
			name: "datadigest",
			hiddenName: "datadigest",
			fieldLabel: "Data digest",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "None","None" ],
					[ "CRC32C","CRC32C" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If set to 'CRC32C' and the initiator is configured accordingly, the integrity of an iSCSI PDU's data segment will be protected by a CRC32C checksum.",
			value: "None"
		},{
			xtype: "numberfield",
			name: "maxconnections",
			fieldLabel: "Max. connections",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The number of connections within a session.",
			value: 1
		},{
			xtype: "numberfield",
			name: "maxsessions",
			fieldLabel: "Max. sessions",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The maximum number of sessions for this target.",
			value: 0
		},{
			xtype: "checkbox",
			name: "initialr2t",
			fieldLabel: "Initial R2T",
			checked: true,
			inputValue: 1,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If enabled, the initiator has to wait for the target to solicit SCSI data before sending it. Disabling it allows the initiator to send a burst of N bytes unsolicited right after and/or together with the command. Thus disabling it may improve performance."
		},{
			xtype: "checkbox",
			name: "immediatedata",
			fieldLabel: "Immediate data",
			checked: false,
			inputValue: 1,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This allows the initiator to append unsolicited data to a command. To achieve better performance, this should be enabled."
		},{
			xtype: "numberfield",
			name: "maxrecvdatasegmentlength",
			fieldLabel: "Max. receive data segment length",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Sets the maximum data segment length that can be received.",
			value: 8192
		},{
			xtype: "numberfield",
			name: "maxxmitdatasegmentlength",
			fieldLabel: "Max. transmit data segment length",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Sets the maximum data segment length that can be sent.",
			value: 8192
		},{
			xtype: "numberfield",
			name: "maxburstlength",
			fieldLabel: "Max. burst length",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Sets the maximum amount of either unsolicited or solicited data the initiator may send in a single burst.",
			value: 262144
		},{
			xtype: "numberfield",
			name: "firstburstlength",
			fieldLabel: "First burst length",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Sets the amount of unsolicited data the initiator may transmit in the first burst of a transfer either with and/or right after the command.",
			value: 65536
		},{
			xtype: "numberfield",
			name: "maxoutstandingr2t",
			fieldLabel: "Max. outstanding R2T",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Controls the maximum number of data transfers the target may request at once.",
			value: 1
		},{
			xtype: "checkbox",
			name: "datapduinorder",
			fieldLabel: "Data PDU in order",
			checked: true,
			inputValue: 1
		},{
			xtype: "checkbox",
			name: "datasequenceinorder",
			fieldLabel: "Data sequence in order",
			checked: true,
			inputValue: 1
		},{
			xtype: "numberfield",
			name: "errorrecoverylevel",
			fieldLabel: "Error recovery level",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			value: 0
		},{
			xtype: "numberfield",
			name: "nopinterval",
			fieldLabel: "NOP interval",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If value is non-zero, the initiator will be 'ping'ed during phases of inactivity (i.e. no data transfers) every N seconds to verify the connection is still alive.",
			value: 0
		},{
			xtype: "numberfield",
			name: "noptimeout",
			fieldLabel: "NOP timeout",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If a non-zero 'NOP interval' is used to periodically 'ping' the initiator during phases of inactivity (i.e. no data transfers), the initiator must respond within N seconds, otherwise the connection will be closed.",
			value: 0
		},{
			xtype: "numberfield",
			name: "wthreads",
			fieldLabel: "IO threads",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The number of threads to perform block I/O to the device.",
			value: 8
		},{
			xtype: "numberfield",
			name: "queuedcommands",
			fieldLabel: "Queued commands",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The number of commands an initiator may send and that will be buffered by the target.",
			value: 32
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: "Extra options",
			allowBlank: true,
			autoCreate: {
				tag: "textarea",
				autocomplete: "off",
				rows: "3",
				cols: "65"
			}
		},{
			xtype: "hidden",
			name: "activation"
		}];
		OMV.Module.Services.iSCSITgt.TargetGeneralPanel.superclass.
		  initComponent.call(this);
	}
});

/**
 * @class OMV.Module.Services.iSCSITgt.TargetLUNGrid
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.iSCSITgt.TargetLUNGrid = function(config) {
	var initialConfig = {
		title: "LUN",
		mode: "local",
		stateId: "3107db90-c1e9-11e0-90c8-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Id",
				sortable: true,
				dataIndex: "id",
				id: "id"
			},{
				header: "Device",
				sortable: true,
				dataIndex: "devicefile",
				id: "devicefile"
			},{
				header: "SCSI Id.",
				sortable: true,
				dataIndex: "scsiid",
				id: "scsiid"
			},{
				header: "SCSI serial no.",
				sortable: true,
				dataIndex: "scsisn",
				id: "scsisn"
			},{
				header: "R/W mode",
				sortable: true,
				dataIndex: "iomode",
				id: "iomode",
				renderer: OMV.util.Format.arrayRenderer([
					[ "wt","Write-through" ],
					[ "wb","Write-back" ],
					[ "ro","Read-only" ]
				])
			},{
				header: "Transfer mode",
				sortable: true,
				dataIndex: "type",
				id: "type",
				renderer: OMV.util.Format.arrayRenderer([
					[ "fileio","File IO" ],
					[ "blockio","Block IO" ]
				])
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.TargetLUNGrid.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.TargetLUNGrid,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			remoteSort: false,
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				fields: [
					{ name: "uuid" },
					{ name: "id" },
					{ name: "devicefile" },
					{ name: "iomode" },
					{ name: "type" },
					{ name: "scsiid" },
					{ name: "scsisn" }
    			]
			})
		});
		OMV.Module.Services.iSCSITgt.TargetLUNGrid.superclass.initComponent.
		  apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.iSCSITgt.TargetLUNPropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function(dlg, data) {
					var lastRowNum = this.store.getCount();
					// Update the row number
					data.id = lastRowNum;
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
		var wnd = new OMV.Module.Services.iSCSITgt.TargetLUNPropertyDialog({
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

	/**
	 * Renumber all configured LUNs
	 */
	afterDeletion : function() {
		for (var i = 0; i < this.store.getTotalCount() - 1; i++) {
			var record = this.store.getAt(i);
			record.beginEdit();
			record.set("id", i);
			record.endEdit();
		}
	},

	/**
	 * Validates the field value. To be valid there must be at least
	 * one configured LUN.
	 */
	isValid : function() {
		var valid = (this.store.getCount() > 0);
		return valid;
	},

	/**
	 * @method setValues
	 * Load values into the grids store.
	 * @param values The values to load into the grids store.
	 */
	setValues : function(values) {
		OMV.Module.Services.iSCSITgt.TargetLUNGrid.superclass.setValues.
		  call(this, values.luns.lun);
	},

	/**
	 * @method getValues
	 * Returns the records of the grids store as object with key/value pairs.
	 * @return The records of the grids store as key/value pairs.
	 */
	getValues : function() {
		var luns = OMV.Module.Services.iSCSITgt.TargetLUNGrid.superclass.
		  getValues.call(this);
		var values = {
			luns: luns
		};
		return values;
	}
});

/**
 * @class OMV.Module.Services.iSCSITgt.TargetLUNPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.iSCSITgt.TargetLUNPropertyDialog = function(config) {
	var initialConfig = {
		mode: "local",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " LUN",
		width: 500,
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.TargetLUNPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.TargetLUNPropertyDialog,
  OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		}
	},

	getFormItems : function() {
		return [{
			xtype: "hidden",
			name: "id"
		},{
			xtype: "combo",
			name: "type",
			hiddenName: "type",
			fieldLabel: "Transfer mode",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
//					[ "fileio","File IO" ],
					[ "blockio","Block IO" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			readOnly: (this.uuid !== OMV.UUID_UNDEFINED),
			triggerAction: "all",
			value: "blockio"
		},{
			xtype: "combo",
			name: "devicefile",
			hiddenName: "devicefile",
			fieldLabel: "Device",
			emptyText: "Select an device ...",
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy("FileSystemMgmt",
				  "getCandidates"),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "description" }
					]
				})
			}),
			displayField: "description",
			valueField: "devicefile",
			allowBlank: false,
			editable: false,
			readOnly: (this.uuid !== OMV.UUID_UNDEFINED),
			triggerAction: "all"
		},{
			xtype: "textfield",
			name: "scsiid",
			fieldLabel: "SCSI Id.",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Assign a unique identifier to the iSCSI volume optionally."
		},{
			xtype: "textfield",
			name: "scsisn",
			fieldLabel: "SCSI serial no.",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Assign a unique serial number to the iSCSI volume optionally."
		},{
			xtype: "combo",
			name: "iomode",
			hiddenName: "iomode",
			fieldLabel: "R/W mode",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "wt","Write-through" ],
					[ "wb","Write-back" ],
					[ "ro","Read-only" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "wt"
		}];
	}
});

/**
 * @class OMV.Module.Services.iSCSITgt.TargetAuthUserGrid
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.iSCSITgt.TargetAuthUserGrid = function(config) {
	var initialConfig = {
		title: "Authentication",
		mode: "local",
		stateId: "53f13d90-c1e9-11e0-a6eb-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Type",
				sortable: true,
				dataIndex: "type",
				id: "type",
				renderer: OMV.util.Format.arrayRenderer([
					[ "incoming","Incoming" ],
					[ "outgoing","Outgoing" ]
				])
			},{
				header: "Username",
				sortable: true,
				dataIndex: "username",
				id: "username"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.TargetAuthUserGrid.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.TargetAuthUserGrid,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			remoteSort: false,
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				fields: [
					{ name: "uuid" },
					{ name: "type" },
					{ name: "username" },
					{ name: "password" }
    			]
			})
		});
		OMV.Module.Services.iSCSITgt.TargetAuthUserGrid.superclass.
		  initComponent.apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.iSCSITgt.AuthUserPropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function(dlg, data) {
					// Do some checks before adding the new user
					if (data.type === "outgoing") {
						var collection = this.store.query("type", "outgoing");
						if (collection.getCount() > 0) {
							OMV.MessageBox.failure(null, "Only one outgoing " +
							  "user per target is supported.");
							return;
						}
					}
					var record = new this.store.recordType(data);
					record.markDirty();
					this.store.add(record);
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Services.iSCSITgt.AuthUserPropertyDialog({
			uuid: record.get("uuid"),
			listeners: {
				submit: function(dlg, data) {
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

	setValues : function(values) {
		OMV.Module.Services.iSCSITgt.TargetAuthUserGrid.superclass.setValues.
		  call(this, values.authentication.user);
	},

	/**
	 * @method getValues
	 * Returns the records of the grids store as object with key/value pairs.
	 * @return The records of the grids store as key/value pairs.
	 */
	getValues : function() {
		var authentication = OMV.Module.Services.iSCSITgt.TargetAuthUserGrid.
		  superclass.getValues.call(this);
		var values = {
			authentication: authentication
		};
		return values;
	}
});

/**
 * @class OMV.Module.Services.iSCSITgt.AuthUserPropertyDialog
 */
OMV.Module.Services.iSCSITgt.AuthUserPropertyDialog = function(config) {
	var initialConfig = {
		mode: "local",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " user",
		width: 500,
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.AuthUserPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.AuthUserPropertyDialog,
	OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		}
	},

	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "type",
			hiddenName: "type",
			fieldLabel: "Transfer mode",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "incoming","Incoming" ],
					[ "outgoing","Outgoing" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			readOnly: (this.uuid !== OMV.UUID_UNDEFINED),
			value: "incoming"
		},{
			xtype: "textfield",
			name: "username",
			fieldLabel: "Username",
			allowBlank: false,
			vtype: "username",
			readOnly: (this.uuid !== OMV.UUID_UNDEFINED)
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: "Password",
			allowBlank: true
		}];
	}
});

/**
 * @class OMV.Module.Services.iSCSITgt.AuthUserGrid
 * @derived OMV.form.Grid
 */
OMV.Module.Services.iSCSITgt.AuthUserGrid = function(config) {
	var initialConfig = {
		stateId: "54af0f59-ef4e-4c1f-a3c3-1efaa1ea02fd",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Type",
				sortable: true,
				dataIndex: "type",
				id: "type",
				renderer: OMV.util.Format.arrayRenderer([
					[ "incoming","Incoming" ],
					[ "outgoing","Outgoing" ]
				])
			},{
				header: "Username",
				sortable: true,
				dataIndex: "username",
				id: "username"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.iSCSITgt.AuthUserGrid.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.iSCSITgt.AuthUserGrid, OMV.form.Grid, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			remoteSort: false,
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				fields: [
					{ name: "uuid" },
					{ name: "type" },
					{ name: "username" },
					{ name: "password" }
    			]
			})
		});
		OMV.Module.Services.iSCSITgt.AuthUserGrid.superclass.initComponent.
		  apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.iSCSITgt.AuthUserPropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function(dlg, data) {
					// Do some checks before adding the new user
					if (data.type === "outgoing") {
						var collection = this.store.query("type", "outgoing");
						if (collection.getCount() > 0) {
							OMV.MessageBox.failure(null, "Only one outgoing " +
							  "user per target is supported.");
							return;
						}
					}
					var record = new this.store.recordType(data);
					record.markDirty();
					this.store.add(record);
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Services.iSCSITgt.AuthUserPropertyDialog({
			uuid: record.get("uuid"),
			listeners: {
				submit: function(dlg, data) {
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
	}
});
Ext.reg('iscsitgtauthusergrid', OMV.Module.Services.iSCSITgt.AuthUserGrid);
