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
// require("js/omv/workspace/window/TextArea.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/field/CheckboxGrid.js")
// require("js/omv/grid/column/BinaryUnit.js")
// require("js/omv/grid/column/Empty.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.module.admin.storage.mdadm");

OMV.module.admin.storage.mdadm.mapRaidLevels = [
	[ "stripe", _("Stripe") ],
	[ "raid0", _("Stripe") ],
	[ "mirror", _("Mirror") ],
	[ "raid1", _("Mirror") ],
	[ "linear", _("Linear") ],
	[ "raid2", _("RAID 2") ],
	[ "raid3", _("RAID 3") ],
	[ "raid4", _("RAID 4") ],
	[ "raid5", _("RAID 5") ],
	[ "raid6", _("RAID 6") ],
	[ "raid10", _("RAID 10") ]
];

/**
 * @class OMV.module.admin.storage.mdadm.device.Create
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.storage.mdadm.device.Create", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.form.field.CheckboxGrid",
		"OMV.grid.column.BinaryUnit",
		"OMV.grid.column.Empty"
	],

	rpcService: "RaidMgmt",
	rpcSetMethod: "create",
	title: _("Create RAID device"),
	okButtonText: _("Create"),
	autoLoadData: false,
	hideResetButton: true,
	width: 550,
	height: 390,

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
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			vtype: "fslabel"
		},{
			xtype: "combo",
			name: "level",
			fieldLabel: _("Level"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "stripe", _("Stripe") ],
					[ "mirror", _("Mirror") ],
					[ "linear", _("Linear") ],
					[ "raid10", _("RAID 10") ],
					[ "raid5", _("RAID 5") ],
					[ "raid6", _("RAID 6") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "raid5",
			listeners: {
				scope: me,
				change: function(combo, value) {
					// See http://en.wikipedia.org/wiki/Standard_RAID_levels
					var devicesField = this.findField("devices");
					switch (value) {
					case "stripe":
					case "linear":
					case "mirror":
						devicesField.minSelections = 2;
						break;
					case "raid5":
						devicesField.minSelections = 3;
						break;
					case "raid6":
					case "raid10":
						devicesField.minSelections = 4;
						break;
					default:
						devicesField.minSelections = 2;
						break;
					}
					devicesField.validate();
				}
			}
		},{
			xtype: "checkboxgridfield",
			name: "devices",
			fieldLabel: _("Devices"),
			valueField: "devicefile",
			minSelections: 3, // Min. number of devices for RAID5
			minHeight: 170,
			// flex: 1, // Hides the field info due render error
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "size", type: "string" }, // This is a string
						{ name: "vendor", type: "string" },
						{ name: "serialnumber", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "RaidMgmt",
						method: "getCandidates"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			}),
			gridConfig: {
				stateful: true,
				stateId: "efd6463f-db6a-4d9d-9e77-411fc02e4b22",
				columns: [{
					xtype: "emptycolumn",
					text: _("Device"),
					sortable: true,
					dataIndex: "devicefile",
					stateId: "devicefile",
					flex: 1
				},{
					xtype: "binaryunitcolumn",
					text: _("Capacity"),
					sortable: true,
					dataIndex: "size",
					stateId: "size",
					width: 50,
					flex: 1
				},{
					xtype: "emptycolumn",
					text: _("Vendor"),
					sortable: true,
					dataIndex: "vendor",
					stateId: "vendor",
					flex: 1
				},{
					xtype: "emptycolumn",
					text: _("Serial Number"),
					sortable: true,
					dataIndex: "serialnumber",
					stateId: "serialnumber",
					flex: 1
				}]
			},
			plugins: [{
				ptype: "fieldinfo",
				text: _("Select the devices that will be used to create the RAID device. Devices connected via USB will not be listed (too unreliable).")
			}]
		}];
	},

	doSubmit: function() {
		var me = this;
		OMV.MessageBox.confirm(null,
			_("Do you really want to create the RAID device?"),
			function(answer) {
				if (answer === "no")
					return;
				me.superclass.doSubmit.call(me);
			}, me);
	}
});

/**
 * @class OMV.module.admin.storage.mdadm.device.Add
 * @derived OMV.workspace.window.Form
 * @param rpcSetMethod The name of the RPC method to be executed. Defaults
 *  to 'add'.
 * @param devicefile The devicefile of the RAID to grow.
 * @param name The name of the array.
 * @param level The level of the array.
 * @param title The dialog title.
 */
Ext.define("OMV.module.admin.storage.mdadm.device.Add", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.proxy.Rpc",
		"OMV.form.field.CheckboxGrid"
	],

	rpcService: "RaidMgmt",
	rpcSetMethod: "add",
	title: _("Add devices to RAID device"),
	okButtonText: _("OK"),
	autoLoadData: false,
	hideResetButton: true,
	width: 550,
	height: 390,

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
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			readOnly: true,
			submitValue: false,
			value: me.name
		},{
			xtype: "combo",
			name: "level",
			fieldLabel: _("Level"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: OMV.module.admin.storage.mdadm.mapRaidLevels
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			readOnly: true,
			submitValue: false,
			value: me.level
		},{
			xtype: "checkboxgridfield",
			name: "devices",
			fieldLabel: _("Devices"),
			valueField: "devicefile",
			minSelections: 1,
			height: 170,
			// flex: 1, // Hides the field info due render error
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "size", type: "string" }, // This is a string
						{ name: "vendor", type: "string" },
						{ name: "serialnumber", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "RaidMgmt",
						method: "getCandidates"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}],
				listeners: {
					scope: me,
					load: function(store, records, successful, operation, eOpts) {
						// Remove the RAID device file.
						var record = store.findExactRecord("devicefile",
						  me.devicefile);
						if (Ext.isObject(record) && record.isModel)
							  store.remove(record);
					}
				}
			}),
			gridConfig: {
				stateful: true,
				stateId: "11333089-7a71-4b49-931d-6ddf4bad77ed",
				columns: [{
					xtype: "emptycolumn",
					text: _("Device"),
					sortable: true,
					dataIndex: "devicefile",
					stateId: "devicefile",
					flex: 1
				},{
					xtype: "binaryunitcolumn",
					text: _("Capacity"),
					sortable: true,
					dataIndex: "size",
					stateId: "size",
					flex: 1
				},{
					xtype: "emptycolumn",
					text: _("Vendor"),
					sortable: true,
					dataIndex: "vendor",
					stateId: "vendor",
					flex: 1
				},{
					xtype: "emptycolumn",
					text: _("Serial Number"),
					sortable: true,
					dataIndex: "serialnumber",
					stateId: "serialnumber",
					flex: 1
				}]
			},
			plugins: [{
				ptype: "fieldinfo",
				text: _("Select devices to be added to the RAID device")
			}]
		},{
			xtype: "hidden",
			name: "devicefile",
			value: me.devicefile
		}];
	}
});

/**
* @class OMV.module.admin.storage.mdadm.device.Remove
* @derived OMV.workspace.window.Form
* @param devicefile The devicefile of the RAID.
* @param name The name of the array.
* @param level The level of the array.
*/
Ext.define("OMV.module.admin.storage.mdadm.device.Remove", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.proxy.Rpc",
		"OMV.form.field.CheckboxGrid"
	],

	rpcService: "RaidMgmt",
	rpcSetMethod: "remove",
	title: _("Remove devices from RAID device"),
	okButtonText: _("OK"),
	autoLoadData: false,
	hideResetButton: true,
	width: 550,
	height: 390,

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
		// Set the max. number of devices that can be removed. See for fault
		// tolerance at http://en.wikipedia.org/wiki/Standard_RAID_levels
		var maxSelections = 1;
		switch (me.level) {
		case "mirror":
		case "raid1":
		case "raid2":
		case "raid3":
		case "raid4":
		case "raid5":
			maxSelections = 1;
			break;
		case "raid6":
			maxSelections = 2;
			break;
		case "raid10":
			maxSelections = 1;
			break;
		}
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			readOnly: true,
			value: me.name
		},{
			xtype: "combo",
			name: "level",
			fieldLabel: _("Level"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: OMV.module.admin.storage.mdadm.mapRaidLevels
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			readOnly: true,
			value: me.level
		},{
			xtype: "checkboxgridfield",
			name: "devices",
			fieldLabel: _("Devices"),
			valueField: "devicefile",
			minSelections: 1,
			maxSelections: maxSelections,
			height: 170,
			// flex: 1, // Hides the field info due render error
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "size", type: "string" }, // This is a string
						{ name: "vendor", type: "string" },
						{ name: "serialnumber", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					extraParams: {
						devicefile: me.devicefile
					},
					rpcData: {
						service: "RaidMgmt",
						method: "getSlaves"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			}),
			gridConfig: {
				stateful: true,
				stateId: "2b1120a4-9a92-11e4-aee9-00221568ca88",
				columns: [{
					xtype: "emptycolumn",
					text: _("Device"),
					sortable: true,
					dataIndex: "devicefile",
					stateId: "devicefile",
					flex: 1
				},{
					xtype: "binaryunitcolumn",
					text: _("Capacity"),
					sortable: true,
					dataIndex: "size",
					stateId: "size",
					flex: 1
				},{
					xtype: "emptycolumn",
					text: _("Vendor"),
					sortable: true,
					dataIndex: "vendor",
					stateId: "vendor",
					flex: 1
				},{
					xtype: "emptycolumn",
					text: _("Serial Number"),
					sortable: true,
					dataIndex: "serialnumber",
					stateId: "serialnumber",
					flex: 1
				}]
			},
			plugins: [{
				ptype: "fieldinfo",
				text: _("Select devices to be removed from the RAID device.")
			}]
		}];
	},

	getValues: function() {
		var me = this;
		var values = me.callParent(arguments);;
		return {
			"devicefile": me.devicefile,
			"devices": values.devices
		};
	}
});

/**
 * @class OMV.module.admin.storage.mdadm.device.Detail
 * @derived OMV.workspace.window.TextArea
 */
Ext.define("OMV.module.admin.storage.mdadm.device.Detail", {
	extend: "OMV.workspace.window.TextArea",

	rpcService: "RaidMgmt",
	rpcGetMethod: "getDetail",
	title: _("Array details"),
	width: 600,
	height: 400
});

/**
 * @class OMV.module.admin.storage.mdadm.Devices
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.mdadm.Devices", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.storage.mdadm.device.Create",
		"OMV.module.admin.storage.mdadm.device.Add",
		"OMV.module.admin.storage.mdadm.device.Remove",
		"OMV.module.admin.storage.mdadm.device.Detail"
	],

	autoReload: true,
	rememberSelected: true,
	disableLoadMaskOnLoad: true,
	hideAddButton: true,
	hideEditButton: true,
	hideRefreshButton: true,
	hidePagingToolbar: false,
	addButtonText: "Create",
	stateful: true,
	stateId: "4ba1f909-9f57-4aab-87eb-638385c30f46",
	columns: [{
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name"
	},{
		text: _("Device"),
		sortable: true,
		dataIndex: "devicefile",
		stateId: "devicefile"
	},{
		xtype: "whitespacecolumn",
		text: _("State"),
		sortable: true,
		dataIndex: "state",
		stateId: "state"
	},{
		xtype: "mapcolumn",
		text: _("Level"),
		sortable: true,
		dataIndex: "level",
		stateId: "level",
		mapItems: OMV.module.admin.storage.mdadm.mapRaidLevels
	},{
		xtype: "binaryunitcolumn",
		text: _("Capacity"),
		sortable: true,
		dataIndex: "size",
		stateId: "size"
	},{
		xtype: "templatecolumn",
		text: _("Devices"),
		sortable: true,
		dataIndex: "devices",
		stateId: "devices",
		tpl: '<tpl for="devices" between="&lt;br/&gt;">{.}</tpl>'
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "_used", type: "boolean" },
						{ name: "name", type: "string" },
						{ name: "devicefile", type: "string" },
						{ name: "size", type: "string" },
						{ name: "level", type: "string" },
						{ name: "devices", type: "array" },
						{ name: "state", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "RaidMgmt",
						method: "getList",
						options: {
							updatelastaccess: false
						}
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "name"
				}]
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		Ext.Array.insert(items, 1, [{
			id: me.getId() + "-create",
			xtype: "button",
			text: _("Create"),
			iconCls: "x-fa fa-plus",
			handler: me.onCreateButton,
			scope: me,
			disabled: false
		},{
			id: me.getId() + "-grow",
			xtype: "button",
			text: _("Grow"),
			iconCls: "x-fa fa-expand",
			handler: me.onGrowButton,
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1,
				enabledFn: function(c, records) {
					// Only RAID level 1/4/5/6 are able to grow.
					var level = records[0].get("level");
					var state = records[0].get("state");
					return (([ "raid1", "stripe", "raid4", "raid5",
					  "raid6" ].indexOf(level) !== -1) &&
					  ([ "clean", "active" ].indexOf(state) !== -1));
				}
			}
		},{
			id: me.getId() + "-remove",
			xtype: "button",
			text: _("Remove"),
			iconCls: "x-fa fa-minus",
			handler: me.onRemoveButton,
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1,
				enabledFn: function(c, records) {
					var level = records[0].get("level");
					var state = records[0].get("state");
					// Only RAID level 1/2/3/4/5/6/10 are tolerant enough
					// to be able to remove slave/component devices without
					// loosing the whole array.
					return (([ "raid1", "mirror", "raid2", "raid3", "raid4",
					  "raid5", "raid6", "raid10" ].indexOf(level) !== -1) &&
					  ([ "clean", "active" ].indexOf(state) !== -1));
				}
			}
		},{
			id: me.getId() + "-recover",
			xtype: "button",
			text: _("Recover"),
			iconCls: "x-fa fa-medkit",
			handler: me.onRecoverButton,
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1
			}
		},{
			id: me.getId() + "-detail",
			xtype: "button",
			text: _("Detail"),
			iconCls: "x-fa fa-info",
			handler: me.onDetailButton,
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1
			}
		}]);
		return items;
	},

	onItemDblClick: function() {
		var me = this;
		me.onDetailButton(me);
	},

	doDeletion: function(record) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "RaidMgmt",
				method: "delete",
				params: {
					devicefile: record.get("devicefile")
				}
			}
		});
	},

	onCreateButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.storage.mdadm.device.Create", {
			scope: me,
			listeners: {
				submit: function() {
					me.doReload();
					OMV.MessageBox.info(null, _("Please wait until the RAID has been initialized before creating a file system."));
				}
			}
		}).show();
	},

	onGrowButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.mdadm.device.Add", {
			title: _("Grow RAID device"),
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			level: record.get("level"),
			rpcSetMethod: "grow",
			listeners: {
				scope: me,
				submit: function() {
					me.doReload();
					OMV.MessageBox.info(null, _("After the RAID has been grown you can resize the containing file system."));
				}
			}
		}).show();
	},

	onRecoverButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.mdadm.device.Add", {
			title: _("Add hot spares / recover RAID device"),
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			level: record.get("level"),
			listeners: {
				scope: me,
				submit: function() {
					me.doReload();
				}
			}
		}).show();
	},

	onRemoveButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.mdadm.device.Remove", {
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			level: record.get("level"),
			listeners: {
				scope: me,
				submit: function() {
					me.doReload();
				}
			}
		}).show();
	},

	onDetailButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.mdadm.device.Detail", {
			rpcGetParams: {
				devicefile: record.get("devicefile")
			}
		}).show();
	}
});

OMV.WorkspaceManager.registerNode({
	id: "mdadm",
	path: "/storage",
	text: _("RAID Management"),
	iconCls: "x-fa fa-database",
	position: 30
});

OMV.WorkspaceManager.registerPanel({
	id: "devices",
	path: "/storage/mdadm",
	text: _("Devices"),
	position: 10,
	className: "OMV.module.admin.storage.mdadm.Devices"
});
