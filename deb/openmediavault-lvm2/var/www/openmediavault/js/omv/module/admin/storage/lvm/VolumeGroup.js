/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/field/CheckboxGrid.js")

// Append new vtypes used by LVM property dialogs.
// According to http://linux.die.net/man/8/lvm
Ext.apply(Ext.form.VTypes, {
	vgname: function(v) {
		return /^[a-zA-Z0-9+_\.\-]+$/.test(v);
	},
	vgnameText: _("Invalid name"),
	vgnameMask: /[a-zA-Z0-9+_\.\-]/,
});

/**
 * @class OMV.module.admin.storage.lvm.vg.Create
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.storage.lvm.vg.Create", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.form.field.CheckboxGrid"
	],

	rpcService: "LogicalVolumeMgmt",
	rpcSetMethod: "createVolumeGroup",
	hideResetButton: true,
	autoLoadData: false,
	title: _("Create volume group"),
	width: 500,

	getFormItems: function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "vgname"
		},{
			xtype: "checkboxgridfield",
			name: "devices",
			fieldLabel: _("Devices"),
			valueField: "devicefile",
			height: 110,
			minSelections: 1,
			useStringValue: true,
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "description", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getVolumeGroupCandidates"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			}),
			gridConfig: {
				stateful: true,
				stateId: "f5a7a042-ae59-4a5d-a1f9-276230cacab7",
				columns: [{
					text: _("Description"),
					sortable: true,
					dataIndex: "description",
					stateId: "description"
				}]
			}
		}];
	},

	doSubmit: function() {
		var me = this;
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to create the volume group?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if(answer === "no")
					return;
				me.superclass.doSubmit.call(me);
			},
			scope: me,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.module.admin.storage.lvm.vg.Edit
 * @derived OMV.workspace.window.Form
 * @param devicefile The devicefile of the volume group.
 * @param name The name of the volume group.
 */
Ext.define("OMV.module.admin.storage.lvm.vg.Edit", {
	extend: "OMV.workspace.window.Form",

	rpcService: "LogicalVolumeMgmt",
	rpcSetMethod: "renameVolumeGroup",
	title: _("Edit volume group"),
	autoLoadData: false,
	hideResetButton: false,
	width: 300,

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "vgname",
			value: me.name
		}];
	},

	getRpcSetParams: function() {
		var me = this;
		var params = me.callParent(arguments);
		return Ext.apply(params, {
			devicefile: me.devicefile
		});
	}
});

/**
 * @class OMV.module.admin.storage.lvm.vg.Extend
 * @derived OMV.workspace.window.Form
 * @param devicefile The devicefile of the volume group.
 * @param name The name of the volume group.
 */
Ext.define("OMV.module.admin.storage.lvm.vg.Extend", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.form.field.CheckboxGrid"
	],

	rpcService: "LogicalVolumeMgmt",
	rpcSetMethod: "extendVolumeGroup",
	title: _("Extend volume group"),
	autoLoadData: false,
	hideResetButton: true,
	width: 450,

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			readOnly: true,
			submitValue: false,
			vtype: "vgname",
			value: me.name
		},{
			xtype: "checkboxgridfield",
			name: "devices",
			fieldLabel: _("Devices"),
			valueField: "devicefile",
			height: 110,
			minSelections: 1,
			useStringValue: true,
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "description", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getVolumeGroupCandidates"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			}),
			gridConfig: {
				stateful: true,
				stateId: "713c4486-89cd-4e80-b21d-11921e5f717e",
				columns: [{
					text: _("Description"),
					sortable: true,
					dataIndex: "description",
					stateId: "description"
				}]
			}
		}];
	},

	getRpcSetParams: function() {
		var me = this;
		var params = me.callParent(arguments);
		return Ext.apply(params, {
			devicefile: me.devicefile
		});
	}
});

/**
 * @class OMV.module.admin.storage.lvm.vg.Reduce
 * @derived OMV.workspace.window.Form
 * @param devicefile The devicefile of the volume group.
 * @param name The name of the volume group.
 */
Ext.define("OMV.module.admin.storage.lvm.vg.Reduce", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.form.field.CheckboxGrid"
	],

	rpcService: "LogicalVolumeMgmt",
	rpcSetMethod: "reduceVolumeGroup",
	autoLoadData: false,
	hideResetButton: true,
	title: _("Reduce volume group"),
	width: 450,

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "hidden",
			name: "devicefile",
			value: me.devicefile
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			readOnly: true,
			submitValue: false,
			vtype: "vgname",
			value: me.name
		},{
			xtype: "checkboxgridfield",
			name: "devices",
			fieldLabel: _("Devices"),
			valueField: "devicefile",
			height: 110,
			minSelections: 1,
			useStringValue: true,
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "description", type: "string" },
						{ name: "_used", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					extraParams: {
						name: me.name
					},
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getVolumeGroupPhysicalVolumes"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}],
				listeners: {
					scope: me,
					datachanged: function(store) {
						// Do not display devices that are in usage and not
						// removable. Suspend the firing of all events,
						// otherwise 'datachanged' will be fired which will
						// result in a invinite loop.
						store.suspendEvents();
						store.filter("_used", false);
						store.resumeEvents();
					}
				}
			}),
			gridConfig: {
				stateful: true,
				stateId: "6060bab3-e7da-40a9-99cf-5f7570f16a68",
				columns: [{
					text: _("Description"),
					sortable: true,
					dataIndex: "description",
					stateId: "description"
				}]
			}
		}];
	}
});

/**
 * @class OMV.module.admin.storage.lvm.VolumeGroups
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.lvm.VolumeGroups", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.storage.lvm.vg.Create",
		"OMV.module.admin.storage.lvm.vg.Edit",
		"OMV.module.admin.storage.lvm.vg.Extend",
		"OMV.module.admin.storage.lvm.vg.Reduce"
	],

	hideAddButton: false,
	hideEditButton: false,
	hideDeleteButton: false,
	hideRefreshButton: true,
	hidePagingToolbar: false,
	stateful: true,
	stateId: "af0712c4-9f60-493a-9be4-c9658f958f99",
	columns: [{
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name"
	},{
		xtype: "binaryunitcolumn",
		text: _("Available"),
		sortable: true,
		dataIndex: "size",
		stateId: "size"
	},{
		xtype: "binaryunitcolumn",
		text: _("Free"),
		sortable: true,
		dataIndex: "free",
		stateId: "free"
	},{
		text: _("Physical volumes"),
		sortable: true,
		dataIndex: "pvname",
		stateId: "pvname",
		renderer: function(value, metaData, record) {
			var tpl = new Ext.XTemplate('<tpl for=".">{.}<br/></tpl>');
			return tpl.apply(record.get("pvname"));
		}
	},{
		text: _("Logical volumes"),
		sortable: true,
		dataIndex: "lvname",
		stateId: "lvname",
		renderer: function(value, metaData, record) {
			var tpl = new Ext.XTemplate('<tpl for=".">{.}<br/></tpl>');
			return tpl.apply(record.get("lvname"));
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
						{ name: "devicefile", type: "string" },
						{ name: "name", type: "string" },
						{ name: "free", type: "string" },
						{ name: "size", type: "string" },
						{ name: "pvname", type: "array" },
						{ name: "lvname", type: "array" },
						{ name: "_used", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getVolumeGroupsList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		// Add additional buttons.
		Ext.Array.insert(items, 2, [{
			id: me.getId() + "-extend",
			xtype: "button",
			text: _("Extend"),
			icon: "images/expand.png",
			handler: Ext.Function.bind(me.onExtendButton, me, [ me ]),
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-reduce",
			xtype: "button",
			text: _("Reduce"),
			icon: "images/shrink.png",
			handler: Ext.Function.bind(me.onReduceButton, me, [ me ]),
			scope: me,
			disabled: true
		}]);
		return items;
	},

	onSelectionChange: function(model, records) {
		var me = this;
		me.callParent(arguments);
		// Process additional buttons.
		var tbarBtnName = [ "extend","reduce" ];
		var tbarBtnDisabled = {
			"extend": true,
			"reduce": true
		};
		if(records.length <= 0) {
			tbarBtnDisabled["extend"] = true;
			tbarBtnDisabled["reduce"] = true;
		} else if(records.length == 1) {
			tbarBtnDisabled["extend"] = false;
			tbarBtnDisabled["reduce"] = false;
		} else {
			tbarBtnDisabled["extend"] = true;
			tbarBtnDisabled["reduce"] = true;
		}
		for(var i = 0; i < tbarBtnName.length; i++) {
			var tbarBtnCtrl = me.queryById(me.getId() + "-" + tbarBtnName[i]);
			if(!Ext.isEmpty(tbarBtnCtrl)) {
				if(true == tbarBtnDisabled[tbarBtnName[i]])
					tbarBtnCtrl.disable();
				else
					tbarBtnCtrl.enable();
			}
		}
	},

	onAddButton : function() {
		var me = this;
		Ext.create("OMV.module.admin.storage.lvm.vg.Create", {
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.lvm.vg.Edit", {
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onExtendButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.lvm.vg.Extend", {
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onReduceButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.lvm.vg.Reduce", {
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	doDeletion: function(record) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "LogicalVolumeMgmt",
				method: "deleteVolumeGroup",
				params: {
					name: record.get("name")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "volumegroups",
	path: "/storage/lvm",
	text: _("Volume groups"),
	position: 20,
	className: "OMV.module.admin.storage.lvm.VolumeGroups"
});
