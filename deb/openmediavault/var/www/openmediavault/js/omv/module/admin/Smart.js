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
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/FormPanelDialog.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/module/admin/Logs.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Storage");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("storage", "smart", {
	text: _("S.M.A.R.T."),
	icon: "images/harddisk-smart.png",
	position: 40
});

/**
 * @class OMV.Module.Storage.SMARTDevicesPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.SMARTDevicesPanel = function(config) {
	var initialConfig = {
		hideAdd: true,
		hideEdit: true,
		hideDelete: true,
		hidePagingToolbar: false,
		stateId: "103a18fd-df1c-4934-b5fd-e90b3e08fa91",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Device"),
				sortable: true,
				dataIndex: "devicefile",
				id: "devicefile",
				width: 50
			},{
				header: _("Model"),
				sortable: true,
				dataIndex: "model",
				id: "model"
			},{
				header: _("Vendor"),
				sortable: true,
				dataIndex: "vendor",
				id: "vendor",
				width: 30
			},{
				header: _("Serial Number"),
				sortable: true,
				dataIndex: "serialnumber",
				id: "serialnumber"
			},{
				header: _("Capacity"),
				sortable: true,
				dataIndex: "size",
				id: "size",
				width: 50,
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				header: _("Temperature"),
				sortable: true,
				dataIndex: "temperature",
				id: "temperature",
				width: 45
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.SMARTDevicesPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.SMARTDevicesPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "Smart",
				"method": "getList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "devicefile",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "devicefile" },
					{ name: "model" },
					{ name: "vendor" },
					{ name: "serialnumber" },
					{ name: "size" },
					{ name: "temperature" }
    			]
			})
		});
		OMV.Module.Storage.SMARTDevicesPanel.superclass.initComponent.apply(
		  this, arguments);
		this.on("rowdblclick", this.cbDetailBtnHdl, this);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Storage.SMARTDevicesPanel.superclass.
		  initToolbar.apply(this);
		tbar.add({
			id: this.getId() + "-detail",
			xtype: "button",
			text: _("Detail"),
			icon: "images/detail.png",
			handler: this.cbDetailBtnHdl,
			scope: this,
			disabled: true
		},{
			id: this.getId() + "-information",
			xtype: "button",
			text: _("Information"),
			icon: "images/info.png",
			handler: this.cbInformationBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		var records = model.getSelections();
		var tbarDetailCtrl = this.getTopToolbar().findById(
			this.getId() + "-detail");
		var tbarInformationCtrl = this.getTopToolbar().findById(
			this.getId() + "-information");
		if (records.length <= 0) {
			tbarDetailCtrl.disable();
			tbarInformationCtrl.disable();
		} else if (records.length == 1) {
			tbarDetailCtrl.enable();
			tbarInformationCtrl.enable();
		} else {
			tbarDetailCtrl.disable();
			tbarInformationCtrl.disable();
		}
	},

	cbDetailBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.SMARTIdentPropertyDialog({
			rpcGetParams: { "devicefile": record.get("devicefile") }
		});
		wnd.show();
	},

	cbInformationBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.SMARTInformationWindow({
			devicefile: record.get("devicefile")
		});
		wnd.show();
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "smart", {
	cls: OMV.Module.Storage.SMARTDevicesPanel,
	title: _("Devices"),
	position: 20
});

/**
 * @class OMV.Module.Storage.SMARTInformationWindow
 * Display S.M.A.R.T. informations from the given device.
 * @config devicefile The device file
 */
OMV.Module.Storage.SMARTInformationWindow = function(config) {
	var initialConfig = {
		title: _("S.M.A.R.T. informations"),
		width: 700,
		height: 350,
		layout: "fit",
		modal: true,
		border: false,
		buttonAlign: "center",
		buttons: [{
			text: _("Close"),
			handler: this.close,
			scope: this
		}]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.SMARTInformationWindow.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.SMARTInformationWindow, Ext.Window, {
	initComponent : function() {
		Ext.apply(this, {
			items: new Ext.TabPanel({
				activeTab: 0,
				layoutOnTabChange: true,
				enableTabScroll: true,
				items: [
					new OMV.Module.Storage.SMARTAttributesPanel({
						devicefile: this.devicefile
					}),
					new OMV.Module.Storage.SMARTSelfTestLogsPanel({
						devicefile: this.devicefile
					})
				]
			})
		});
		OMV.Module.Storage.SMARTInformationWindow.superclass.initComponent.
		  apply(this, arguments);
	}
});

/**
 * @class OMV.Module.Storage.SMARTAttributesPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.SMARTAttributesPanel = function(config) {
	var initialConfig = {
		title: _("Attributes"),
		hideToolbar: true,
		hidePagingToolbar: true,
		stateId: "70556b35-44c5-49d6-8d2e-29c045a57f9c",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("ID"),
				dataIndex: "id",
				id: "id",
				width: 30,
				fixed: true,
				align: "right"
			},{
				header: _("Attribute name"),
				dataIndex: "attrname",
				id: "attrname",
				width: 150
			},{
				header: _("Flag"),
				dataIndex: "flag",
				id: "flag",
				width: 60,
				fixed: true,
				align: "center"
			},{
				header: _("Value"),
				dataIndex: "value",
				id: "value",
				width: 55,
				fixed: true,
				align: "center"
			},{
				header: _("Worst"),
				dataIndex: "worst",
				id: "worst",
				width: 55,
				fixed: true,
				align: "center"
			},{
				header: _("Treshold"),
				dataIndex: "treshold",
				id: "treshold",
				width: 55,
				fixed: true,
				align: "center"
			},{
				header: _("Type"),
				dataIndex: "type",
				id: "type",
				align: "center"
			},{
				header: _("Updated"),
				dataIndex: "updated",
				id: "updated",
				width: 60,
				fixed: true,
				align: "center"
			},{
				header: _("When failed"),
				dataIndex: "whenfailed",
				id: "whenfailed",
				align: "center"
			},{
				header: _("Raw value"),
				dataIndex: "rawvalue",
				id: "rawvalue",
				width: 150
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.SMARTAttributesPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.SMARTAttributesPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "Smart",
				"method": "getAttributes",
				"extraParams": { "devicefile": this.devicefile },
				"appendPagingParams": false
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "id",
				fields: [
					{ name: "id" },
					{ name: "attrname" },
					{ name: "flag" },
					{ name: "value" },
					{ name: "worst" },
					{ name: "treshold" },
					{ name: "type" },
					{ name: "updated" },
					{ name: "whenfailed" },
					{ name: "rawvalue" }
				]
			})
		});
		OMV.Module.Storage.SMARTAttributesPanel.superclass.initComponent.
		  apply(this, arguments);
	}
});

/**
 * @class OMV.Module.Storage.SMARTSelfTestLogsPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.SMARTSelfTestLogsPanel = function(config) {
	var initialConfig = {
		title: _("Self-test logs"),
		hideToolbar: true,
		hidePagingToolbar: true,
		stateId: "ac2859c4-fb88-4757-870c-794e5919c221",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Num"),
				dataIndex: "num",
				id: "num",
				width: 35,
				fixed: true,
				align: "right"
			},{
				header: _("Test description"),
				dataIndex: "description",
				id: "description",
				width: 100,
				fixed: true
			},{
				header: _("Status"),
				dataIndex: "status",
				id: "status",
				width: 200
			},{
				header: _("Remaining"),
				dataIndex: "remaining",
				id: "remaining",
				width: 60,
				fixed: true,
				align: "center",
				renderer: function(val, cell, record, row, col, store) {
					return val + "%";
				}
			},{
				header: _("Lifetime"),
				dataIndex: "lifetime",
				id: "lifetime",
				width: 55,
				fixed: true,
				align: "center"
			},{
				header: _("LBA of first error"),
				dataIndex: "lbaoffirsterror",
				id: "lbaoffirsterror",
				width: 100,
				fixed: true
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.SMARTSelfTestLogsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.SMARTSelfTestLogsPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "Smart",
				"method": "getSelfTestLogs",
				"extraParams": { "devicefile": this.devicefile },
				"appendPagingParams": false
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "id",
				fields: [
					{ name: "num" },
					{ name: "description" },
					{ name: "status" },
					{ name: "remaining" },
					{ name: "lifetime" },
					{ name: "lbaoffirsterror" }
				]
			})
		});
		OMV.Module.Storage.SMARTSelfTestLogsPanel.superclass.initComponent.
		  apply(this, arguments);
	}
});

/**
 * @class OMV.Module.Storage.SMARTIdentPropertyDialog
 */
OMV.Module.Storage.SMARTIdentPropertyDialog = function(config) {
	var initialConfig = {
		hideOk: true,
		hideCancel: true,
		hideClose: false,
		hideReset: true,
		rpcService: "Smart",
		rpcGetMethod: "getIdentityInfo",
		title: _("Device identity information"),
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.SMARTIdentPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.SMARTIdentPropertyDialog, OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "devicemodel",
			fieldLabel: _("Device model"),
			readOnly: true
		},{
			xtype: "textfield",
			name: "serialnumber",
			fieldLabel: _("Serial number"),
			readOnly: true
		},{
			xtype: "textfield",
			name: "firmwareversion",
			fieldLabel: _("Firmware version"),
			readOnly: true
		}];
	},

	cbOkBtnHdl : function() {
		this.close();
	},

	cbLoadHdl : function(id, response, error) {
		OMV.Module.Storage.SMARTIdentPropertyDialog.superclass.
		  cbLoadHdl.call(this, id, response, error);
		if (error !== null) {
			// Close the property dialog on error
			this.close();
		}
	}
});

/**
 * @class OMV.Module.Storage.SMARTSettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Storage.SMARTSettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "Smart",
		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.SMARTSettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.SMARTSettingsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("General settings"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1
			},{
				xtype: "numberfield",
				name: "interval",
				fieldLabel: _("Check interval"),
				minValue: 10,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 1800,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Sets the interval between disk checks to N seconds.")
			},{
				xtype: "combo",
				name: "powermode",
				hiddenName: "powermode",
				fieldLabel: _("Power mode"),
				mode: "local",
				store: [
					[ "never",_("Never") ],
					[ "sleep",_("Sleep") ],
					[ "standby",_("Standby") ],
					[ "idle",_("Idle") ]
				],
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "standby",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Prevent a disk from being spun-up when it is periodically polled.<br/><br/><ul><li>Never - Poll (check) the device regardless of its power mode. This may cause a disk which is spun-down to be spun-up when it is checked.</li><li>Sleep - Check the device unless it is in SLEEP mode.</li><li>Standby - Check the device unless it is in SLEEP or STANDBY mode. In these modes most disks are not spinning, so if you want to prevent a disk from spinning up each poll, this is probably what you want.</li><li>Idle - Check the device unless it is in SLEEP, STANDBY or IDLE mode. In the IDLE state, most disks are still spinning, so this is probably not what you want.</li></ul>")
			}]
		},{
			xtype: "fieldset",
			title: _("Temperature monitoring"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "numberfield",
				name: "tempdiff",
				fieldLabel: _("Difference"),
				width: 40,
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 0,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Report if the temperature had changed by at least N degrees Celsius since last report. Set to 0 to disable this report.")
			},{
				xtype: "numberfield",
				name: "tempinfo",
				fieldLabel: _("Informal"),
				width: 40,
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 0,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Report if the temperature is greater or equal than N degrees Celsius. Set to 0 to disable this report.")
			},{
				xtype: "numberfield",
				name: "tempcrit",
				fieldLabel: _("Critical"),
				width: 40,
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 0,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Report if the temperature is greater or equal than N degrees Celsius. Set to 0 to disable this report.")
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "smart", {
	cls: OMV.Module.Storage.SMARTSettingsPanel,
	title: _("Settings"),
	position: 10
});

/**
 * @class OMV.Module.Storage.SMARTScheduledTestsPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.SMARTScheduledTestsPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "ca86feba-53c1-42b4-8eea-5119f0244fb5",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Device"),
				id: "device",
				width: 200,
				sortable: true,
				renderer: function(val, cell, record, row, col, store) {
					var tpl = new Ext.XTemplate(
					  '<b>Model:</b> {volumemodel}<br/>',
					  '<b>Device:</b> {volumedevicefile}<br/>',
					  '<b>Capacity:</b> {[OMV.util.Format.binaryUnit(values.volumesize)]}');
					return tpl.apply(record.data);
				}
			},{
				header: _("Type"),
				sortable: true,
				dataIndex: "type",
				id: "type",
				renderer: OMV.util.Format.arrayRenderer([
					[ "S",_("Short self-test") ],
					[ "L",_("Long self-test") ],
					[ "C",_("Conveyance self-test") ],
					[ "O",_("Offline immediate test") ]
				])
			},{
				header: _("Hour"),
				sortable: true,
				dataIndex: "hour",
				id: "hour"
			},{
				header: _("Day of month"),
				sortable: true,
				dataIndex: "dayofmonth",
				id: "dayofmonth",
				renderer: OMV.util.Format.arrayRenderer(
				  Date.mapDayOfMonth2Digits)
			},{
				header: _("Month"),
				sortable: true,
				dataIndex: "month",
				id: "month",
				renderer: OMV.util.Format.arrayRenderer(
				  Date.mapMonth2Digits)
			},{
				header: _("Day of week"),
				sortable: true,
				dataIndex: "dayofweek",
				id: "dayofweek",
				renderer: OMV.util.Format.arrayRenderer(Date.mapDayOfWeek)
			},{
				header: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.SMARTScheduledTestsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.SMARTScheduledTestsPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "Smart",
				"method": "getScheduleList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "volumedevicefile", mapping: "volume.devicefile" },
					{ name: "volumemodel", mapping: "volume.model" },
					{ name: "volumesize", mapping: "volume.size" },
					{ name: "type" },
					{ name: "hour" },
					{ name: "dayofmonth" },
					{ name: "month" },
					{ name: "dayofweek" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Storage.SMARTScheduledTestsPanel.superclass.
		  initComponent.apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Storage.SMARTScheduledTestPropertyDialog({
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
		var wnd = new OMV.Module.Storage.SMARTScheduledTestPropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "Smart",
		  "deleteScheduledTest", { "uuid": record.get("uuid") });
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "smart", {
	cls: OMV.Module.Storage.SMARTScheduledTestsPanel,
	title: _("Scheduled tests"),
	position: 30
});

/**
 * @class OMV.Module.Storage.SMARTScheduledTestPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Storage.SMARTScheduledTestPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Smart",
		rpcGetMethod: "getScheduledTest",
		rpcSetMethod: "setScheduledTest",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add scheduled test") : _("Edit scheduled test"),
		height: 330
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.SMARTScheduledTestPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.SMARTScheduledTestPropertyDialog,
  OMV.CfgObjectDialog, {
	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "devicefile",
			hiddenName: "devicefile",
			fieldLabel: _("Device"),
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "Smart",
					"method": "enumerateDevices",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefilebyid",
					fields: [
						{ name: "devicefilebyid" },
						{ name: "description" }
					]
				})
			}),
			emptyText: _("Select a device ..."),
			valueField: "devicefilebyid",
			displayField: "description",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("S.M.A.R.T. monitoring must be activated for the selected device.")
		},{
			xtype: "combo",
			name: "type",
			hiddenName: "type",
			fieldLabel: _("Type"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "S",_("Short self-test") ],
					[ "L",_("Long self-test") ],
					[ "C",_("Conveyance self-test") ],
					[ "O",_("Offline immediate test") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "S"
		},{
			xtype: "combo",
			name: "hour",
			hiddenName: "hour",
			fieldLabel: _("Hour"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapHour2Digits
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: String(new Date().format("H"))
		},{
			xtype: "combo",
			name: "dayofmonth",
			hiddenName: "dayofmonth",
			fieldLabel: _("Day of month"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapDayOfMonth2Digits
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "combo",
			name: "month",
			hiddenName: "month",
			fieldLabel: _("Month"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapMonth2Digits
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "combo",
			name: "dayofweek",
			hiddenName: "dayofweek",
			fieldLabel: _("Day of week"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapDayOfWeek
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		}];
	}
});

/**
 * @class OMV.Module.Diagnostics.LogPlugin.SMART
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'S.M.A.R.T.' logfile diagnostics plugin
 */
OMV.Module.Diagnostics.LogPlugin.SMART = function(config) {
	var initialConfig = {
		title: _("S.M.A.R.T."),
		stateId: "67659b68-3cb2-4434-a92e-f541236e12d0",
		columns: [{
			header: _("Date & Time"),
			sortable: true,
			dataIndex: "rownum",
			id: "date",
			width: 35,
			renderer: function(val, cell, record, row, col, store) {
				return record.get("date");
			}
		},{
			header: _("Event"),
			sortable: true,
			dataIndex: "event",
			id: "event"
		}],
		rpcArgs: { "id": "smartd" },
		rpcFields: [
			{ name: "rownum" },
			{ name: "ts" },
			{ name: "date" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.SMART.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.SMART,
  OMV.Module.Diagnostics.LogPlugin, {
});
OMV.preg("log", "smartd", OMV.Module.Diagnostics.LogPlugin.SMART);
