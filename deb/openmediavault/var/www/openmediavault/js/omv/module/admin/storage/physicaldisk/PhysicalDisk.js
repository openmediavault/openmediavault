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
// require("js/omv/window/Execute.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.storage.physicaldisk.Settings
 * @derived OMV.workspace.window.Form
 * @param uuid The UUID of the configuration object.
 * @param devicefile The device file, e.g. /dev/sda.
 */
Ext.define("OMV.module.admin.storage.physicaldisk.Settings", {
	extend: "OMV.workspace.window.Form",
	requires: [
	    "OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "DiskMgmt",
	rpcGetMethod: "getHdParm",
	rpcSetMethod: "setHdParm",
	title: _("Physical disk properties"),
	plugins: [{
		ptype: "configobject"
	}],
	width: 450,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormItems: function() {
		return [{
			xtype: "combo",
			name: "apm",
			fieldLabel: _("Advanced Power Management"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ 0, _("Disabled") ],
					[ 1, _("1 - Minimum power usage with standby (spindown)") ],
					[ 64, _("64 - Intermediate power usage with standby") ],
					[ 127, _("127 - Intermediate power usage with standby") ],
					[ 128, _("128 - Minimum power usage without standby (no spindown)") ],
					[ 192, _("192 - Intermediate power usage without standby") ],
					[ 254, _("254 - Maximum performance, maximum power usage") ],
					[ 255, _("255 - Disabled") ]
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
			name: "aam",
			fieldLabel: _("Automatic Acoustic Management"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ 0, _("Disabled") ],
					[ 128, _("Minimum performance, Minimum acoustic output") ],
					[ 254, _("Maximum performance, maximum acoustic output") ]
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
			fieldLabel: _("Spindown time"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ 0, _("Disabled") ],
					[ 60, _("5 minutes") ],
					[ 120, _("10 minutes") ],
					[ 240, _("20 minutes") ],
					[ 241, _("30 minutes") ],
					[ 242, _("60 minutes") ],
					[ 244, _("120 minutes") ],
					[ 246, _("180 minutes") ],
					[ 248, _("240 minutes") ],
					[ 250, _("300 minutes") ],
					[ 252, _("360 minutes") ]
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
			name: "writecache",
			fieldLabel: _("Write cache"),
			checked: false,
			boxLabel: _("Enable write-cache."),
			plugins: [{
				ptype: "fieldinfo",
				text: _("This function is effective only if the hard drive supports it.")
			}]
		}];
	},

	getRpcSetParams: function() {
		var me = this;
		var params = me.callParent(arguments);
		// Append the given devicefile.
		return Ext.apply(params || {}, {
			devicefile: me.devicefile
		});
	}
});

/**
 * @class OMV.module.admin.storage.physicaldisk.Devices
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.physicaldisk.Devices", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.storage.physicaldisk.Settings",
		"OMV.window.MessageBox"
	],

	hideAddButton: true,
	hideDeleteButton: true,
	hideRefreshButton: true,
	hidePagingToolbar: false,
	stateful: true,
	stateId: "5e5cc148-c1e7-11e0-99e1-00221568ca88",
	columns: [{
		xtype: "emptycolumn",
		text: _("Device"),
		sortable: true,
		dataIndex: "devicefile",
		stateId: "device",
	},{
		xtype: "emptycolumn",
		text: _("Model"),
		sortable: true,
		dataIndex: "model",
		stateId: "model"
	},{
		xtype: "emptycolumn",
		text: _("Serial Number"),
		sortable: true,
		dataIndex: "serialnumber",
		stateId: "serialnumber"
	},{
		xtype: "emptycolumn",
		text: _("Vendor"),
		sortable: true,
		dataIndex: "vendor",
		stateId: "vendor"
	},{
		xtype: "binaryunitcolumn",
		text: _("Capacity"),
		sortable: true,
		dataIndex: "size",
		stateId: "size"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "devicefilebyid", type: "string" },
						{ name: "model", type: "string" },
						{ name: "vendor", type: "string" },
						{ name: "serialnumber", type: "string" },
						{ name: "size", type: "string" },
						{ name: "hdparm", type: "object" },
						{ name: "israid", type: "boolean" },
						{ name: "isroot", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "DiskMgmt",
						method: "getList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		Ext.Array.push(items, [{
			id: me.getId() + "-wipe",
			xtype: "button",
			text: _("Wipe"),
			icon: "images/erase.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: me.onWipebutton,
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-scan",
			xtype: "button",
			text: _("Scan"),
			icon: "images/search.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: me.onScanButton,
			scope: me
		}]);
		return items;
	},

	onSelectionChange: function(model, records) {
		var me = this;
		me.callParent(arguments);
		// Process additional buttons.
		var tbarBtnDisabled = {
			"edit": true,
			"wipe": true
		};
		// Enable/disable buttons depending on the number of selected rows.
		if (records.length == 1) {
			// If the selected device is a (hardware) RAID, then disable
			// the 'Edit' button because such devices do not support
			// this additional customizations.
			tbarBtnDisabled["edit"] = records[0].get("israid");
			// Disable the 'Wipe' button if the selected device contains the
			// operating system.
			tbarBtnDisabled["wipe"] = records[0].get("isroot");
		}
		// Update the button controls.
		Ext.Object.each(tbarBtnDisabled, function(key, value) {
			var tbarBtnCtrl = this.queryById(this.getId() + "-" + key);
			if (Ext.isObject(tbarBtnCtrl) && tbarBtnCtrl.isButton) {
				if (true == value)
					tbarBtnCtrl.disable();
				else
					tbarBtnCtrl.enable();
			}
		}, me);
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		var hdparm = record.get("hdparm");
		Ext.create("OMV.module.admin.storage.physicaldisk.Settings", {
			uuid: Ext.isObject(hdparm) ? hdparm.uuid : OMV.UUID_UNDEFINED,
			devicefile: record.get("devicefilebyid"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onWipebutton: function() {
		var me = this;
		var record = me.getSelected();
		OMV.MessageBox.show({
			title: _("Confirmation"),
			icon: Ext.Msg.QUESTION,
			msg: _("Do you really want to wipe the selected device?"),
			buttons: Ext.Msg.YESNO,
			scope: me,
			fn: function(answer) {
				if(answer !== "yes")
					return;
				OMV.MessageBox.show({
					title: _("Wiping device ..."),
					icon: Ext.Msg.QUESTION,
					msg: _("Please choose the method to wipe the device or 'Cancel' to abort."),
					buttonText: {
						yes: _("Secure"),
						no: _("Quick"),
						cancel: _("Cancel")
					},
					scope: this,
					fn: function(answer) {
						if(answer == "cancel")
							return;
						var wnd = Ext.create("OMV.window.Execute", {
							title: _("Wiping device ..."),
							rpcService: "DiskMgmt",
							rpcMethod: "wipe",
							rpcParams: {
								devicefile: record.get("devicefile"),
								secure: (answer === "yes") ? true : false
							},
							hideStartButton: true,
							hideStopButton: false,
							killCmdBeforeDestroy: true,
							listeners: {
								scope: this,
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
								}
							}
						});
						wnd.show();
						wnd.start();
					}
				});
			}
		});
	},

	onScanButton: function() {
		var me = this;
		OMV.MessageBox.wait(null, _("Scanning for new devices ..."));
		// Force a rescan of the SCSI bus.
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				// Delay some time ... Note, it is difficult to choose a
				// correct delay time because every system is different,
				// thus we can only guess a proper value that hopefully
				// works on most systems.
				Ext.Function.defer(function() {
					OMV.MessageBox.hide();
					me.doReload();
				}, 5000, me); // 5 seconds
			},
			relayErrors: false,
			rpcData: {
				service: "DiskMgmt",
				method: "rescan"
			}
		});
	}
});

OMV.WorkspaceManager.registerNode({
	id: "physicaldisk",
	path: "/storage",
	text: _("Physical Disks"),
	icon16: "images/hdd.png",
	iconSvg: "images/hdd.svg",
	position: 10
});

OMV.WorkspaceManager.registerPanel({
	id: "devices",
	path: "/storage/physicaldisk",
	text: _("Devices"),
	position: 10,
	className: "OMV.module.admin.storage.physicaldisk.Devices"
});
