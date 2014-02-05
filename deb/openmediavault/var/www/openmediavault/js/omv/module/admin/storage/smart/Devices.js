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
// require("js/omv/workspace/window/Tab.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.storage.smart.device.Information
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.storage.smart.device.Information", {
	extend: "OMV.workspace.window.Form",

	title: _("Device identity information"),
	hideOkButton: true,
	hideCancelButton: true,
	hideCloseButton: false,
	hideResetButton: true,
	rpcService: "Smart",
	rpcGetMethod: "getIdentityInfo",

	getFormItems: function() {
		return [{
			xtype: "textfield",
			name: "devicemodel",
			fieldLabel: _("Device model"),
			emptyText: _("n/a"),
			readOnly: true
		},{
			xtype: "textfield",
			name: "serialnumber",
			fieldLabel: _("Serial number"),
			emptyText: _("n/a"),
			readOnly: true
		},{
			xtype: "textfield",
			name: "firmwareversion",
			fieldLabel: _("Firmware version"),
			emptyText: _("n/a"),
			readOnly: true
		}];
	}
});

/**
 * @class OMV.module.admin.storage.smart.device.Attributes
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.smart.device.Attributes", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],

	title: _("Attributes"),
	hideTopToolbar: true,
	hidePagingToolbar: true,
	stateful: true,
	stateId: "70556b35-44c5-49d6-8d2e-29c045a57f9c",
	columns: [{
		text: _("ID"),
		dataIndex: "id",
		stateId: "id",
		width: 40,
		resizable: false,
		align: "right"
	},{
		text: _("Attribute name"),
		dataIndex: "attrname",
		stateId: "attrname"
	},{
		text: _("Flag"),
		dataIndex: "flag",
		stateId: "flag",
		width: 60,
		resizable: false,
		align: "center"
	},{
		text: _("Value"),
		dataIndex: "value",
		stateId: "value",
		width: 55,
		resizable: false,
		align: "center"
	},{
		text: _("Worst"),
		dataIndex: "worst",
		stateId: "worst",
		width: 55,
		resizable: false,
		align: "center"
	},{
		text: _("Treshold"),
		dataIndex: "treshold",
		stateId: "treshold",
		width: 55,
		resizable: false,
		align: "center"
	},{
		text: _("Type"),
		dataIndex: "type",
		stateId: "type",
		align: "center"
	},{
		text: _("Updated"),
		dataIndex: "updated",
		stateId: "updated",
		width: 60,
		resizable: false,
		align: "center"
	},{
		text: _("When failed"),
		dataIndex: "whenfailed",
		stateId: "whenfailed",
		align: "center"
	},{
		text: _("Raw value"),
		dataIndex: "rawvalue",
		stateId: "rawvalue"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
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
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Smart",
						method: "getAttributes"
					},
					appendSortParams: false,
					extraParams: {
						devicefile: me.devicefile
					}
				}
			})
		});
		me.callParent(arguments);
	}
});

/**
 * @class OMV.module.admin.storage.smart.device.SelfTestLogs
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.smart.device.SelfTestLogs", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],

	title: _("Self-test logs"),
	hideTopToolbar: true,
	hidePagingToolbar: true,
	stateful: true,
	stateId: "ac2859c4-fb88-4757-870c-794e5919c221",
	columns: [{
		text: _("Num"),
		dataIndex: "num",
		stateId: "num",
		width: 40,
		resizable: false,
		align: "right"
	},{
		text: _("Test description"),
		dataIndex: "description",
		stateId: "description",
	},{
		text: _("Status"),
		dataIndex: "status",
		stateId: "status"
	},{
		text: _("Remaining"),
		dataIndex: "remaining",
		stateId: "remaining",
		width: 60,
		resizable: false,
		align: "center",
		renderer: function(value) {
			return value + "%";
		}
	},{
		text: _("Lifetime"),
		dataIndex: "lifetime",
		stateId: "lifetime",
		width: 55,
		resizable: false,
		align: "center"
	},{
		text: _("LBA of first error"),
		dataIndex: "lbaoffirsterror",
		stateId: "lbaoffirsterror"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "num",
					fields: [
						{ name: "num" },
						{ name: "description" },
						{ name: "status" },
						{ name: "remaining" },
						{ name: "lifetime" },
						{ name: "lbaoffirsterror" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Smart",
						method: "getSelfTestLogs"
					},
					appendSortParams: false,
					extraParams: {
						devicefile: me.devicefile
					}
				}
			})
		});
		me.callParent(arguments);
	}
});

/**
 * @class OMV.module.admin.storage.smart.device.Stats
 * @derived OMV.workspace.window.Tab
 * Display S.M.A.R.T. statistic information from the given device.
 * @param devicefile The device file
 */
Ext.define("OMV.module.admin.storage.smart.device.Stats", {
	extend: "OMV.workspace.window.Tab",
	uses: [
		"OMV.module.admin.storage.smart.device.Attributes",
		"OMV.module.admin.storage.smart.device.SelfTestLogs"
	],

	title: _("S.M.A.R.T. information"),
	width: 700,
	height: 350,
	hideOkButton: true,
	hideCancelButton: true,
	hideCloseButton: false,
	hideResetButton: true,

	getTabItems: function() {
		var me = this;
		return [
			Ext.create("OMV.module.admin.storage.smart.device.Attributes", {
				devicefile: me.devicefile
			}),
			Ext.create("OMV.module.admin.storage.smart.device.SelfTestLogs", {
				devicefile: me.devicefile
			}),
		];
	}
});

/**
 * @class OMV.module.admin.storage.smart.device.Devices
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.smart.device.Devices", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
	],
	uses: [
		"OMV.module.admin.storage.smart.device.Information",
		"OMV.module.admin.storage.smart.device.Stats"
	],

	hideAddButton: true,
	hideEditButton: true,
	hideDeleteButton: true,
	hidePagingToolbar: false,
	stateful: true,
	stateId: "103a18fd-df1c-4934-b5fd-e90b3e08fa91",
	columns: [{
		text: _("Device"),
		sortable: true,
		dataIndex: "devicefile",
		stateId: "devicefile"
	},{
		xtype: "emptycolumn",
		text: _("Model"),
		sortable: true,
		dataIndex: "model",
		stateId: "model"
	},{
		xtype: "emptycolumn",
		text: _("Vendor"),
		sortable: true,
		dataIndex: "vendor",
		stateId: "vendor"
	},{
		xtype: "emptycolumn",
		text: _("Serial Number"),
		sortable: true,
		dataIndex: "serialnumber",
		stateId: "serialnumber"
	},{
		xtype: "binaryunitcolumn",
		text: _("Capacity"),
		sortable: true,
		dataIndex: "size",
		stateId: "size"
	},{
		xtype: "emptycolumn",
		text: _("Temperature"),
		sortable: true,
		dataIndex: "temperature",
		stateId: "temperature"
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
						{ name: "model", type: "string" },
						{ name: "vendor", type: "string" },
						{ name: "serialnumber", type: "string" },
						{ name: "size", type: "string" },
						{ name: "temperature", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Smart",
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

	getTopToolbarItems : function() {
		var me = this;
		var items = me.callParent(arguments);
		Ext.Array.push(items, [{
			id: me.getId() + "-detail",
			xtype: "button",
			text: _("Detail"),
			icon: "images/details.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onDetailButton, me, [ me ]),
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-information",
			xtype: "button",
			text: _("Information"),
			icon: "images/info.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onInformationButton, me, [ me ]),
			scope: me,
			disabled: true
		}]);
		return items;
	},

	onItemDblClick: function() {
		this.onDetailButton();
	},

	onSelectionChange: function(model, records) {
		var me = this;
		me.callParent(arguments);
		var tbarDetailCtrl = me.queryById(me.getId() + "-detail");
		var tbarInformationCtrl = me.queryById(me.getId() + "-information");
		if(records.length <= 0) {
			tbarDetailCtrl.disable();
			tbarInformationCtrl.disable();
		} else if(records.length == 1) {
			tbarDetailCtrl.enable();
			tbarInformationCtrl.enable();
		} else {
			tbarDetailCtrl.disable();
			tbarInformationCtrl.disable();
		}
	},

	onDetailButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.smart.device.Information", {
			rpcGetParams: {
				devicefile: record.get("devicefile")
			}
		}).show();
	},

	onInformationButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.smart.device.Stats", {
			devicefile: record.get("devicefile")
		}).show();
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "devices",
	path: "/storage/smart",
	text: _("Devices"),
	position: 20,
	className: "OMV.module.admin.storage.smart.device.Devices"
});
