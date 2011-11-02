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
// require("js/omv/MessageBox.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/FormPanelDialog.js")
// require("js/omv/form/CheckboxGrid.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Storage");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("storage", "raidmanagement", {
	text: "RAID Management",
	icon: "images/raid.png",
	position: 20
});

/**
 * @class OMV.Module.Storage.RAIDGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.RAIDGridPanel = function(config) {
	var initialConfig = {
		autoReload: true,
		reloadInterval: 10000, // 10 seconds
		hideAdd: true,
		hideEdit: true,
		hideRefresh: true,
		hidePagingToolbar: false,
		addButtonText: "Create",
		stateId: "4ba1f909-9f57-4aab-87eb-638385c30f46",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: "Device",
				sortable: true,
				dataIndex: "devicefile",
				id: "devicefile",
				width: 50
			},{
				header: "State",
				sortable: true,
				dataIndex: "state",
				id: "state",
				renderer: OMV.util.Format.whitespaceRenderer()
			},{
				header: "Level",
				sortable: true,
				dataIndex: "level",
				id: "level",
				width: 30
			},{
				header: "Capacity",
				sortable: true,
				dataIndex: "capacity",
				id: "capacity",
				width: 50
			},{
				header: "Devices",
				sortable: true,
				dataIndex: "devices",
				id: "devices",
				renderer: function(val, cell, record, row, col, store) {
					var tpl = new Ext.XTemplate('<tpl for=".">{.}<br/></tpl>');
					return tpl.apply(record.get("devices"));
				}
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.RAIDGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.RAIDGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("RaidMgmt", "getList"),
			reader: new Ext.data.JsonReader({
				idProperty: "devicefile",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "_used" },
					{ name: "name" },
					{ name: "devicefile" },
					{ name: "capacity" },
					{ name: "level" },
					{ name: "devices" },
					{ name: "state" }
    			]
			})
		});
		OMV.Module.Storage.RAIDGridPanel.superclass.initComponent.
		  apply(this, arguments);
		// Register event handler
		this.on("rowdblclick", this.cbDetailBtnHdl, this);
		// Reselect previous selected rows after the store has been
		// reloaded, e.g. to make sure toolbar is updated depending on
		// the latest row record values.
		this.getSelectionModel().previousSelections = [];
		this.store.on("beforeload", function(store, options) {
			  var sm = this.getSelectionModel();
			  var records = sm.getSelections();
			  sm.previousSelections = [];
			  Ext.each(records, function(record, index) {
				  sm.previousSelections.push(record.get("devicefile"));
			  }, this);
		  }, this);
		this.store.on("load", function(store, records, options) {
			  var sm = this.getSelectionModel();
			  var rows = [];
			  if (Ext.isDefined(sm.previousSelections)) {
				  for (var i = 0; i < sm.previousSelections.length; i++) {
					  var index = store.findExact("devicefile",
						sm.previousSelections[i]);
					  if (index !== -1) {
						  rows.push(index);
					  }
				  }
			  }
			  if (rows.length > 0) {
				  sm.selectRows(rows);
			  }
		  }, this);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Storage.RAIDGridPanel.superclass.initToolbar.
		  apply(this);
		// Add 'Create' button to top toolbar
		tbar.insert(1, {
			id: this.getId() + "-create",
			xtype: "button",
			text: "Create",
			icon: "images/raid.png",
			handler: this.cbCreateBtnHdl,
			scope: this,
			disabled: false
		});
		// Add 'Grow' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-grow",
			xtype: "button",
			text: "Grow",
			icon: "images/raid-grow.png",
			handler: this.cbGrowBtnHdl,
			scope: this,
			disabled: true
		});
		// Add 'Detail' button to top toolbar
		tbar.insert(3, {
			id: this.getId() + "-detail",
			xtype: "button",
			text: "Detail",
			icon: "images/detail.png",
			handler: this.cbDetailBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Storage.RAIDGridPanel.superclass.cbSelectionChangeHdl.
		  apply(this, arguments);
		// Process additional buttons
		var tbarBtnName = [ "grow", "detail" ];
		var tbarBtnDisabled = {
			"grow": true,
			"detail": true
		};
		var records = model.getSelections();
		if (records.length <= 0) {
			tbarBtnDisabled["grow"] = true;
			tbarBtnDisabled["detail"] = true;
		} else if (records.length == 1) {
			tbarBtnDisabled["detail"] = false;
			// Only RAID level 1/4/5/6 are able to grow.
			var level = records[0].get("level");
			var state = records[0].get("state");
			tbarBtnDisabled["grow"] = !(([ "raid1", "stripe", "raid4", "raid5",
			  "raid6" ].indexOf(level) !== -1) &&
			  ([ "clean", "active" ].indexOf(state) !== -1));
		} else {
			tbarBtnDisabled["grow"] = true;
			tbarBtnDisabled["detail"] = true;
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

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "RaidMgmt",
		  "delete", [ record.get("devicefile") ]);
	},

	cbCreateBtnHdl : function() {
		var wnd = new OMV.Module.Storage.RAIDCreateDialog({
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbGrowBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.RAIDGrowDialog({
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			level: record.get("level"),
			listeners: {
				submit: function() {
					this.doReload();
					OMV.MessageBox.info(null, "After the RAID has been " +
					  "grown you can resize the containing filesystem.");
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbDetailBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.RAIDDetailDialog({
			rpcGetParams: [ record.get("devicefile") ]
		});
		wnd.show();
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "raidmanagement", {
	cls: OMV.Module.Storage.RAIDGridPanel
});

/**
 * @class OMV.Module.Storage.RAIDCreateDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Storage.RAIDCreateDialog = function(config) {
	var initialConfig = {
		rpcService: "RaidMgmt",
		rpcSetMethod: "create",
		title: "Create RAID device",
		autoHeight: true,
		hideReset: true,
		width: 550
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.RAIDCreateDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.RAIDCreateDialog, OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: "Name",
			vtype: "fslabel"
		},{
			xtype: "combo",
			name: "level",
			hiddenName: "level",
			fieldLabel: "Level",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "stripe","Stripe" ],
					[ "mirror","Mirror" ],
					[ "linear","Linear" ],
					[ "raid10","RAID 10" ],
					[ "raid5","RAID 5" ],
					[ "raid6","RAID 6" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "raid5"
		},{
			xtype: "checkboxgrid",
			name: "devices",
			hiddenName: "devices",
			fieldLabel: "Devices",
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy("RaidMgmt", "getCandidates"),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "description" }
					]
				})
			}),
			valueField: "devicefile",
			stateId: "efd6463f-db6a-4d9d-9e77-411fc02e4b22",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: "Description",
					sortable: true,
					dataIndex: "description",
					id: "description"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			height: 130,
			frame: true
		}];
	},

	isValid : function() {
		var valid = OMV.Module.Storage.RAIDCreateDialog.
		  superclass.isValid.apply(this, arguments);
		if (!valid) {
			return valid;
		}
		valid = false;
		var level = this.findFormField("level").getValue();
		var devices = this.findFormField("devices").getValue();
		if (devices.length > 0) {
			devices = devices.split(",");
		}
		var minNumDevices = -1;
		// Does the number of selected devices match the minimum requirements
		// of the selected RAID type?
		switch (level) {
		case "stripe":
		case "linear":
			minNumDevices = 2;
			valid = (devices.length >= minNumDevices);
			break;
		case "mirror":
			minNumDevices = 2;
			valid = (devices.length == minNumDevices);
			break;
		case "raid10":
			minNumDevices = 4;
			valid = (devices.length >= minNumDevices);
			break;
		case "raid5":
			minNumDevices = 3;
			valid = (devices.length >= minNumDevices);
			break;
		case "raid6":
			minNumDevices = 4;
			valid = (devices.length >= minNumDevices);
			break;
		default:
			valid = false;
			break;
		}
		if (!valid) {
			OMV.MessageBox.failure(null, String.format("Incorrect number " +
			  "of selected devices. At least {0} devices are required.",
			  minNumDevices));
		}
		return valid;
	},

	doSubmit : function() {
		OMV.MessageBox.show({
			title: "Confirmation",
			msg: "Do you really want to create the RAID device?",
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if (answer === "no") {
					return;
				}
				OMV.Module.Storage.RAIDCreateDialog.superclass.doSubmit.
				  apply(this, arguments);
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.Module.Storage.RAIDGrowDialog
 * @derived OMV.FormPanelDialog
 * @config devicefile The devicefile of the RAID to grow.
 * @config name The name of the array.
 * @config level The level of the array.
 */
OMV.Module.Storage.RAIDGrowDialog = function(config) {
	var initialConfig = {
		rpcService: "RaidMgmt",
		rpcSetMethod: "grow",
		title: "Grow RAID device",
		autoHeight: true,
		hideReset: true,
		width: 550
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.RAIDGrowDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.RAIDGrowDialog, OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: "Name",
			readOnly: true,
			value: this.name
		},{
			xtype: "combo",
			name: "level",
			hiddenName: "level",
			fieldLabel: "Level",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "stripe","Stripe" ],
					[ "raid0","Stripe" ],
					[ "mirror","Mirror" ],
					[ "raid1","Mirror" ],
					[ "linear","Linear" ],
					[ "raid5","RAID 5" ],
					[ "raid6","RAID 6" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			readOnly: true,
			value: this.level
		},{
			xtype: "checkboxgrid",
			name: "devices",
			hiddenName: "devices",
			fieldLabel: "Devices",
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy("RaidMgmt", "getCandidates"),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "capacity" },
						{ name: "vendor" },
						{ name: "serialnumber" }
					]
				})
			}),
			valueField: "devicefile",
			stateId: "11333089-7a71-4b49-931d-6ddf4bad77ed",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: "Device",
					sortable: true,
					dataIndex: "devicefile",
					id: "devicefile",
					width: 40
				},{
					header: "Capacity",
					sortable: true,
					dataIndex: "capacity",
					id: "capacity",
					width: 50
				},{
					header: "Vendor",
					sortable: true,
					dataIndex: "vendor",
					id: "vendor",
					width: 30
				},{
					header: "Serial Number",
					sortable: true,
					dataIndex: "serialnumber",
					id: "serialnumber"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			height: 130,
			frame: true
		}];
	},

	getValues : function() {
		var values = OMV.Module.Storage.RAIDGrowDialog.superclass.
		  getValues.call(this, arguments);
		return {
			"devicefile": this.devicefile,
			"devices": values.devices
		};
	}
});

/**
 * @class OMV.Module.Storage.RAIDDetailDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Storage.RAIDDetailDialog = function(config) {
	var initialConfig = {
		hideOk: true,
		hideCancel: true,
		hideClose: false,
		hideReset: true,
		rpcService: "RaidMgmt",
		rpcGetMethod: "getDetail",
		title: "Array details",
		width: 600,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.RAIDDetailDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.RAIDDetailDialog, OMV.FormPanelDialog, {
	initForm : function() {
		return new Ext.Panel({
			layout: "fit",
			border: false,
			items: [{
				id: this.getId() + "-detail",
				xtype: "textarea",
				name: "detail",
				readOnly: true,
				cls: "x-form-textarea-monospaced",
				disabledClass: ""
			}]
		});
	},

	cbOkBtnHdl : function() {
		this.close();
	},

	cbLoadHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			var cmp = Ext.getCmp(this.getId() + "-detail");
			if (!Ext.isEmpty(cmp)) {
				cmp.setValue(response);
			}
		} else {
			OMV.MessageBox.error(null, error);
		}
	}
});
