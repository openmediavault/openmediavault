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
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/panel/Textarea.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/Tab.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.storage.smart.device.Settings
 * @derived OMV.workspace.window.Form
 * @param uuid The UUID of the configuration object.
 * @param devicefile The device file, e.g. /dev/sda.
 */
Ext.define("OMV.module.admin.storage.smart.device.Settings", {
	extend: "OMV.workspace.window.Form",

	rpcService: "Smart",
	rpcGetMethod: "getDeviceSettings",
	rpcSetMethod: "setDeviceSettings",
	title: _("Device properties"),
	width: 450,

	initComponent: function() {
		var me = this;
		// Do not auto-load configuration data if 'uuid' is undefined
		// (e.g. no configuration has been set for the given device until
		// now).
		Ext.apply(me, {
			autoLoadData: (me.uuid !== OMV.UUID_UNDEFINED)
		});
		me.callParent(arguments);
	},

	getFormItems: function() {
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Monitor"),
			checked: false,
			boxLabel: _("Activate S.M.A.R.T. monitoring.")
		}];
	},

	getRpcGetParams: function() {
		var me = this;
		return Ext.apply({}, {
			devicefile: me.devicefile
		});
	},

	getRpcSetParams: function() {
		var me = this;
		var params = me.callParent(arguments);
		// Append the given devicefile.
		return Ext.apply(params || {}, {
			uuid: me.uuid,
			devicefile: me.devicefile
		});
	}
});

/**
 * @class OMV.module.admin.storage.smart.device.information.Information
 * @derived OMV.workspace.form.Panel
 * @param devicefile The device file, e.g. /dev/sda.
 */
Ext.define("OMV.module.admin.storage.smart.device.information.Information", {
	extend: "OMV.workspace.form.Panel",

	title: _("Device information"),
	hideTopToolbar: true,
	rpcService: "Smart",
	rpcGetMethod: "getInformation",

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			defaults: {
				readOnly: true
			},
			rpcGetParams: {
				devicefile: me.devicefile
			}
		});
		me.callParent(arguments);
	},

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textfield",
			name: "devicefile",
			fieldLabel: _("Device"),
			emptyText: _("n/a"),
			value: me.devicefile
		},{
			xtype: "textfield",
			name: "devicemodel",
			fieldLabel: _("Device model"),
			emptyText: _("n/a")
		},{
			xtype: "textfield",
			name: "serialnumber",
			fieldLabel: _("Serial number"),
			emptyText: _("n/a")
		},{
			xtype: "textfield",
			name: "firmwareversion",
			fieldLabel: _("Firmware version"),
			emptyText: _("n/a")
		}];
	}
});

/**
 * @class OMV.module.admin.storage.smart.device.information.Attributes
 * @derived OMV.workspace.grid.Panel
 * @param devicefile The device file, e.g. /dev/sda.
 */
Ext.define("OMV.module.admin.storage.smart.device.information.Attributes", {
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
		stateId: "attrname",
		flex: 1
	},{
		text: _("Flags"),
		dataIndex: "flags",
		stateId: "flags",
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
						{ name: "id", type: "string" },
						{ name: "attrname", type: "string" },
						{ name: "flags", type: "string" },
						{ name: "value", type: "string" },
						{ name: "worst", type: "string" },
						{ name: "treshold", type: "string" },
						{ name: "whenfailed", type: "string" },
						{ name: "rawvalue", type: "string" }
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
 * @class OMV.module.admin.storage.smart.device.information.SelfTestLogs
 * @derived OMV.workspace.grid.Panel
 * @param devicefile The device file, e.g. /dev/sda.
 */
Ext.define("OMV.module.admin.storage.smart.device.information.SelfTestLogs", {
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
		width: 45,
		resizable: false,
		align: "right"
	},{
		text: _("Test description"),
		dataIndex: "description",
		stateId: "description",
		flex: 1
	},{
		text: _("Status"),
		dataIndex: "status",
		stateId: "status",
		flex: 1
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
 * @class OMV.module.admin.storage.smart.device.information.ExtendedInformation
 * @derived OMV.workspace.panel.Textarea
 * @param devicefile The device file, e.g. /dev/sda.
 */
Ext.define("OMV.module.admin.storage.smart.device.information.ExtendedInformation", {
	extend: "OMV.workspace.panel.Textarea",

	title: _("Extended information"),
	hideTopToolbar: true,
	rpcService: "Smart",
	rpcMethod: "getExtendedInformation",

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			rpcParams: {
				devicefile: me.devicefile
			}
		});
		me.callParent(arguments);
	}
});

/**
 * @class OMV.module.admin.storage.smart.device.Information
 * @derived OMV.workspace.window.Tab
 * Display S.M.A.R.T. information about the given device.
 * @param devicefile The device file.
 */
Ext.define("OMV.module.admin.storage.smart.device.Information", {
	extend: "OMV.workspace.window.Tab",
	uses: [
		"OMV.module.admin.storage.smart.device.information.Information",
		"OMV.module.admin.storage.smart.device.information.Attributes",
		"OMV.module.admin.storage.smart.device.information.SelfTestLogs",
		"OMV.module.admin.storage.smart.device.information.ExtendedInformation"
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
			Ext.create("OMV.module.admin.storage.smart.device.information.Information", {
				devicefile: me.devicefile
			}),
			Ext.create("OMV.module.admin.storage.smart.device.information.Attributes", {
				devicefile: me.devicefile
			}),
			Ext.create("OMV.module.admin.storage.smart.device.information.SelfTestLogs", {
				devicefile: me.devicefile
			}),
			Ext.create("OMV.module.admin.storage.smart.device.information.ExtendedInformation", {
				devicefile: me.devicefile
			})
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
		"OMV.module.admin.storage.smart.device.Settings",
		"OMV.module.admin.storage.smart.device.Information"
	],

	hideAddButton: true,
	hideDeleteButton: true,
	hidePagingToolbar: false,
	stateful: true,
	stateId: "103a18fd-df1c-4934-b5fd-e90b3e08fa91",
	columns: [{
		xtype: "booleaniconcolumn",
		text: _("Monitor"),
		sortable: true,
		dataIndex: "monitor",
		stateId: "monitor",
		align: "center",
		width: 80,
		resizable: false,
		trueIcon: "switch_on.png",
		falseIcon: "switch_off.png"
	},{
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
						{ name: "uuid", type: "string" },
						{ name: "devicefile", type: "string" },
						{ name: "devicefilebyid", type: "string" },
						{ name: "model", type: "string" },
						{ name: "vendor", type: "string" },
						{ name: "serialnumber", type: "string" },
						{ name: "size", type: "string" },
						{ name: "temperature", type: "string" },
						{ name: "monitor", type: "boolean" }
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
			id: me.getId() + "-information",
			xtype: "button",
			text: _("Information"),
			icon: "images/details.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onInformationButton, me, [ me ]),
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1
			}
		}]);
		return items;
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.smart.device.Settings", {
			uuid: record.get("uuid"),
			devicefile: record.get("devicefilebyid"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onInformationButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.smart.device.Information", {
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
