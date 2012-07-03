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
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/ExecCmdDialog.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Storage");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("storage", "physicaldisks", {
	text: _("Physical Disks"),
	icon: "images/harddisk.png",
	position: 10
});

/**
 * @class OMV.Module.Storage.PhysicalDiskPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.PhysicalDiskPanel = function(config) {
	var initialConfig = {
		hideAdd: true,
		hideDelete: true,
		hideRefresh: true,
		hidePagingToolbar: false,
		stateId: "5e5cc148-c1e7-11e0-99e1-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Device"),
				sortable: true,
				dataIndex: "devicefile",
				id: "device",
				width: 50
			},{
				header: _("Model"),
				sortable: true,
				dataIndex: "model",
				id: "model"
			},{
				header: _("Serial Number"),
				sortable: true,
				dataIndex: "serialnumber",
				id: "serialnumber"
			},{
				header: _("Vendor"),
				sortable: true,
				dataIndex: "vendor",
				id: "vendor",
				width: 30
			},{
				header: _("Capacity"),
				sortable: true,
				dataIndex: "size",
				id: "size",
				width: 50,
				renderer: OMV.util.Format.binaryUnitRenderer()
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.PhysicalDiskPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.PhysicalDiskPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "DiskMgmt",
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
					{ name: "hdparm" },
					{ name: "israid" },
					{ name: "isrootdevice" }
    			]
			})
		});
		OMV.Module.Storage.PhysicalDiskPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Storage.PhysicalDiskPanel.superclass.
		  initToolbar.apply(this);
		// Add 'Wipe' button to top toolbar
		tbar.add({
			id: this.getId() + "-wipe",
			xtype: "button",
			text: _("Wipe"),
			icon: "images/erase.png",
			handler: this.cbWipeBtnHdl,
			scope: this,
			disabled: true
		});
		// Add 'Scan' button to top toolbar
		tbar.add({
			id: this.getId() + "-scan",
			xtype: "button",
			text: _("Scan"),
			icon: "images/search.png",
			handler: this.cbScanBtnHdl,
			scope: this
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Storage.PhysicalDiskPanel.superclass.cbSelectionChangeHdl.
		  apply(this, arguments);
		// Process additional buttons
		var tbarBtnName = [ "edit", "wipe" ];
		var tbarBtnDisabled = {
			"edit": true,
			"wipe": true
		};
		var records = model.getSelections();
		// Enable/disable buttons depending on the number of selected rows.
		if (records.length == 1) {
			// If the selected device is a (hardware) RAID, then disable
			// the 'Edit' button because such devices do not support
			// this additional customizations.
			tbarBtnDisabled["edit"] = records[0].get("israid");
			// Disable the 'Wipe' button if the selected device contains the
			// operating system.
			tbarBtnDisabled["wipe"] = records[0].get("isrootdevice");
		}
		// Update the button controls.
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

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var hdparm = record.get("hdparm");
		var wnd = new OMV.Module.Storage.HdParmPropertyDialog({
			uuid: Ext.isObject(hdparm) ? hdparm.uuid : OMV.UUID_UNDEFINED,
			devicefile: record.get("devicefile"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbWipeBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to wipe the selected device?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if (answer == "no")
					return;
				OMV.MessageBox.show({
					title: _("Wiping device ..."),
					msg: _("Do you want to secure wipe the device? Please note that this may take a long time. Press 'No' to fast wipe the device or 'Cancel' to abort."),
					buttons: Ext.Msg.YESNOCANCEL,
					fn: function(answer) {
						if (answer == "cancel")
							return;
						var wnd = new OMV.ExecCmdDialog({
							title: _("Wiping device ..."),
							rpcService: "DiskMgmt",
							rpcMethod: "wipe",
							rpcArgs: {
								"devicefile": record.get("devicefile"),
								"secure": (answer === "yes") ? true : false
							},
							hideStart: true,
							hideStop: false,
							killCmdBeforeDestroy: true,
							listeners: {
								finish: function(wnd, response) {
									wnd.appendValue("\n" + _("Done ..."));
									wnd.setButtonVisible("stop", false);
									wnd.setButtonDisabled("close", false);
								},
								exception: function(wnd, error) {
									OMV.MessageBox.error(null, error);
									wnd.setButtonDisabled("close", false);
								},
								close: function() {
									this.doReload();
								},
								scope: this
							}
						});
						wnd.show();
						wnd.start();
					},
					scope: this,
					icon: Ext.Msg.QUESTION
				});
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	},

	cbScanBtnHdl : function() {
		OMV.MessageBox.wait(null, _("Scanning for new devices ..."));
		// Force a rescan of the SCSI bus.
		OMV.Ajax.request(function(id, response, error) {
			  if (error === null) {
				  // Delay some time ... Note, it is difficult to choose a
				  // correct delay time because every system is different,
				  // thus we can only guess a proper value that hopefully
				  // works on most systems.
				  (function() {
					  OMV.MessageBox.hide();
					  this.doReload();
				  }).defer(10000); // 10 seconds
			  } else {
				  OMV.MessageBox.error(null, error);
			  }
		  }, this, "DiskMgmt", "rescan", null);
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "physicaldisks", {
	cls: OMV.Module.Storage.PhysicalDiskPanel
});

/**
 * @class OMV.Module.Storage.HdParmPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Storage.HdParmPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "DiskMgmt",
		rpcGetMethod: "getHdParm",
		rpcSetMethod: "setHdParm",
		title: _("Edit physical disk properties"),
		autoHeight: true,
		width: 450
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.HdParmPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.HdParmPropertyDialog, OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "apm",
			hiddenName: "apm",
			fieldLabel: _("Advanced Power Management"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ 255,_("Disabled") ],
					[ 1,_("1 - Minimum power usage with standby (spindown)") ],
					[ 64,_("64 - Intermediate power usage with standby") ],
					[ 127,_("127 - Intermediate power usage with standby") ],
					[ 128,_("128 - Minimum power usage without standby (no spindown)") ],
					[ 192,_("192 - Intermediate power usage without standby") ],
					[ 254,_("254 - Maximum performance, maximum power usage") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: 255
		},{
			xtype: "combo",
			name: "aam",
			hiddenName: "aam",
			fieldLabel: _("Automatic Acoustic Management"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ 0,_("Disabled") ],
					[ 128,_("Minimum performance, Minimum acoustic output") ],
					[ 254,_("Maximum performance, maximum acoustic output") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: 0
		},{
			xtype: "combo",
			name: "spindowntime",
			hiddenName: "spindowntime",
			fieldLabel: _("Spindown time"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ 0,_("Disabled") ],
					[ 60,_("5 minutes") ],
					[ 120,_("10 minutes") ],
					[ 240,_("20 minutes") ],
					[ 241,_("30 minutes") ],
					[ 242,_("60 minutes") ],
					[ 244,_("120 minutes") ],
					[ 246,_("180 minutes") ],
					[ 248,_("240 minutes") ],
					[ 250,_("300 minutes") ],
					[ 252,_("360 minutes") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: 0
		},{
			xtype: "checkbox",
			name: "smart",
			fieldLabel: _("S.M.A.R.T."),
			checked: false,
			inputValue: 1,
			boxLabel: _("Activate S.M.A.R.T. monitoring.")
		},{
			xtype: "checkbox",
			name: "writecache",
			fieldLabel: _("Write cache"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Enable write-cache."),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("This function is effective only if the hard drive supports it.")
		}];
	},

	getValues : function() {
		var values = OMV.Module.Storage.HdParmPropertyDialog.superclass.
		  getValues.call(this);
		// Append the devicefile
		values.devicefile = this.devicefile;
		return values;
	}
});
