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
// require("js/omv/data/proxy/RpcBg.js")

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
				editable: false,
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
			emptyText: _("n/a")
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
		xtype: "textcolumn",
		text: _("ID"),
		dataIndex: "id",
		stateId: "id",
		width: 60,
		align: "right"
	},{
		xtype: "tooltipcolumn",
		text: _("Name"),
		dataIndex: "attrname",
		stateId: "attrname",
		flex: 1,
		minWidth: 100,
		getTooltipText: function(value, record) {
			return record.get("description");
		}
	},{
		text: _("Flags"),
		dataIndex: "flags",
		stateId: "flags",
		width: 80,
		align: "center"
	},{
		text: _("Value"),
		dataIndex: "value",
		stateId: "value",
		width: 55,
		align: "center"
	},{
		text: _("Worst"),
		dataIndex: "worst",
		stateId: "worst",
		width: 55,
		align: "center"
	},{
		text: _("Threshold"),
		dataIndex: "threshold",
		stateId: "threshold",
		width: 55,
		align: "center"
	},{
		text: _("When failed"),
		dataIndex: "whenfailed",
		stateId: "whenfailed",
		align: "center"
	},{
		text: _("Raw value"),
		dataIndex: "rawvalue",
		stateId: "rawvalue",
	},{
		xtype: "booleantextcolumn",
		text: _("Type"),
		dataIndex: "prefailure",
		stateId: "prefailure",
		trueText: _("prefail"),
		falseText: _("old-age")
	},{
		xtype: "fonticoncolumn",
		text: _("Status"),
		sortable: true,
		dataIndex: "assessment",
		stateId: "assessment",
		align: "center",
		resizable: false,
		width: 80,
		getFontIconCls: function(value, metaData, record) {
			var colorCls = Ext.baseCSSPrefix + "color-gray";
			if (true === record.get("prefailure")) {
				switch (value) {
				case "GOOD":
					colorCls = Ext.baseCSSPrefix + "color-good";
					break;
				case "BAD_SECTOR":
				case "BAD_ATTRIBUTE_NOW":
				case "BAD_ATTRIBUTE_IN_THE_PAST":
					colorCls = Ext.baseCSSPrefix + "color-error";
					break;
				}
			}
			return ["mdi mdi-checkbox-blank-circle", colorCls];
		}
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "id",
					fields: [
						{ name: "id", type: "int" },
						{ name: "attrname", type: "string" },
						{ name: "flags", type: "string" },
						{ name: "value", type: "int" },
						{ name: "worst", type: "int" },
						{ name: "threshold", type: "int" },
						{ name: "whenfailed", type: "string" },
						{ name: "rawvalue", type: "string" },
						{ name: "description", type: "string" },
						{ name: "prefailure", type: "boolean" },
						{ name: "assessment", type: "string" }
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
		xtype: "textcolumn",
		text: _("Num"),
		dataIndex: "num",
		stateId: "num",
		width: 60,
		align: "right"
	},{
		xtype: "textcolumn",
		text: _("Test description"),
		dataIndex: "description",
		stateId: "description",
		flex: 1
	},{
		xtype: "textcolumn",
		text: _("Status"),
		dataIndex: "status",
		stateId: "status",
		flex: 1
	},{
		text: _("Remaining"),
		dataIndex: "remaining",
		stateId: "remaining",
		width: 60,
		align: "center",
		renderer: function(value) {
			return Ext.String.htmlEncode(value) + "%";
		}
	},{
		xtype: "textcolumn",
		text: _("Lifetime"),
		dataIndex: "lifetime",
		stateId: "lifetime",
		width: 60,
		align: "center"
	},{
		xtype: "textcolumn",
		text: _("LBA of first error"),
		dataIndex: "lbaoffirsterror",
		stateId: "lbaoffirsterror",
		width: 100
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
		"OMV.data.proxy.RpcBg",
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
		xtype: "enabledcolumn",
		text: _("Monitor"),
		sortable: true,
		dataIndex: "monitor",
		stateId: "monitor"
	},{
		text: _("Device"),
		sortable: true,
		dataIndex: "canonicaldevicefile",
		stateId: "devicefile"
	},{
		xtype: "devicefilescolumn",
		text: _("Device Symlinks"),
		sortable: false,
		hidden: true,
		dataIndex: "devicelinks",
		stateId: "devicelinks"
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
	},{
		xtype: "fonticoncolumn",
		text: _("Status"),
		sortable: true,
		dataIndex: "overallstatus",
		stateId: "overallstatus",
		align: "center",
		resizable: false,
		width: 80,
		getFontIconCls: function(value, metaData) {
			var text = _("Unknown");
			var colorCls = Ext.baseCSSPrefix + "color-gray";
			switch (value) {
			case "GOOD":
				text = _("Good");
				colorCls = Ext.baseCSSPrefix + "color-good";
				break;
			case "BAD_ATTRIBUTE_NOW":
				text = _("Device is being used outside design parameters");
				colorCls = Ext.baseCSSPrefix + "color-error";
				break;
			case "BAD_ATTRIBUTE_IN_THE_PAST":
				text = _("Device was used outside of design parameters in the past");
				colorCls = Ext.baseCSSPrefix + "color-error";
				break;
			case "BAD_SECTOR":
				text = _("Device has a few bad sectors");
				colorCls = Ext.baseCSSPrefix + "color-error";
				break;
			case "BAD_SECTOR_MANY":
				text = _("Device has many bad sectors");
				colorCls = Ext.baseCSSPrefix + "color-error";
				break;
			}
			metaData.tdAttr = Ext.String.format("data-qtip='{0}'",
			  Ext.String.htmlEncode(text));
			return ["mdi mdi-checkbox-blank-circle", colorCls];
		}
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
						{ name: "canonicaldevicefile", type: "string" },
						{ name: "devicefile", type: "string" },
						{ name: "devicelinks", type: "array" },
						{ name: "model", type: "string" },
						{ name: "vendor", type: "string" },
						{ name: "serialnumber", type: "string" },
						{ name: "size", type: "string" },
						{ name: "temperature", type: "string" },
						{ name: "monitor", type: "boolean" },
						{ name: "overallstatus", type: "string" }
					]
				}),
				proxy: {
					type: "rpcbg",
					rpcData: {
						service: "Smart",
						method: "getListBg"
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
			iconCls: "x-fa fa-info",
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
			devicefile: record.get("devicefile"),
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
