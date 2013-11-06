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
// require("js/omv/form/CompositeField.js")

// Append new vtypes used by LVM property dialogs.
// According to http://linux.die.net/man/8/lvm
Ext.apply(Ext.form.VTypes, {
	lvname: function(v) {
		return /^[a-zA-Z0-9+_\.\-]+$/.test(v);
	},
	lvnameText: _("Invalid name"),
	lvnameMask: /[a-zA-Z0-9+_\.\-]/
});

/**
 * @class OMV.module.admin.storage.lvm.lv.Create
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.storage.lvm.lv.Create", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.form.CompositeField"
	],

	rpcService: "LogicalVolumeMgmt",
	rpcSetMethod: "createLogicalVolume",
	title: _("Create logical volume"),
	hideResetButton: true,
	autoLoadData: false,
	width: 500,
	lvFree: 0,

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "lvname"
		},{
			xtype: "combo",
			name: "vgname",
			fieldLabel: _("Volume group"),
			emptyText: _("Select a volume group ..."),
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "description", type: "string" },
						{ name: "free", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getLogicalVolumeCandidates"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			}),
			displayField: "description",
			valueField: "devicefile",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			listeners: {
				scope: me,
				select: function(combo, records) {
					var disabled = false;
					var record = records[0];
					var free = parseInt(record.get("free"));
					// Update the 'Size' slider control.
					field = this.findField("sizeslider");
					if(0 >= free) {
						disabled = true;
						OMV.MessageBox.info(null, _("No free space available to create a logical volume."));
					} else {
						field.on("change", function(slider, newValue, thumb) {
							// Update the hidden field storing the number of
							// bytes to use.
							var field = this.findField("size");
							field.setValue(newValue);
							// Display value in highest possible binary unit
							// in the textfield right beside the slider
							// control.
							field = this.findField("sizetext");
							field.setValue(newValue.binaryFormat());
						}, this);
					}
					field.setMaxValue(free);
					field.setValue(free);
					field.setDisabled(disabled);
				}
			}
		},{
			xtype: "compositefield",
			fieldLabel: _("Size"),
			combineErrors: false,
			items: [{
				xtype: "hidden",
				name: "size"
			},{
				xtype: "sliderfield",
				name: "sizeslider",
				minValue: 1,
				decimalPrecision: 0,
				useTips: false,
				disabled: true,
				flex: 1,
				submitValue: false
			},{
				xtype: "textfield",
				name: "sizetext",
				width: 90,
				readOnly: true,
				submitValue: false
			}]
		}];
	},

	doSubmit: function() {
		var me = this;
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to create the logical volume?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if(answer === "no")
					return;
				me.superclass.doSubmit.apply(this, arguments);
			},
			scope: me,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.module.admin.storage.lvm.lv.Edit
 * @derived OMV.workspace.window.Form
 * @param devicefile The devicefile of the volume group.
 * @param name The name of the volume group.
 */
Ext.define("OMV.module.admin.storage.lvm.lv.Edit", {
	extend: "OMV.workspace.window.Form",

	rpcService: "LogicalVolumeMgmt",
	rpcSetMethod: "renameLogicalVolume",
	title: _("Edit logical volume"),
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
			vtype: "lvname",
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
 * @class OMV.module.admin.storage.lvm.lv.Extend
 * @derived OMV.workspace.window.Form
 * @param devicefile The devicefile of the logical volume.
 * @param name The name of the logical volume.
 * @param size The current size of the logical volume.
 * @param vgname The name of the volume group.
 */
Ext.define("OMV.module.admin.storage.lvm.lv.Extend", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.form.CompositeField"
	],

	title: _("Extend logical volume"),
	rpcService: "LogicalVolumeMgmt",
	rpcSetMethod: "modifyLogicalVolume",
	autoLoadData: false,
	hideResetButton: true,
	width: 500,

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			readOnly: true,
			submitValue: false,
			vtype: "lvname",
			value: me.name
		},{
			xtype: "combo",
			name: "vgname",
			submitValue: false,
			fieldLabel: _("Volume group"),
			emptyText: _("Select a volume group ..."),
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "name",
					fields: [
						{ name: "name", type: "string" },
						{ name: "description", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					extraParams: {
						name: me.vgname
					},
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getVolumeGroup"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "name"
				}]
			}),
			displayField: "description",
			valueField: "name",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			readOnly: true,
			submitValue: false,
			value: me.vgname
		},{
			xtype: "compositefield",
			fieldLabel: _("Size"),
			combineErrors: false,
			items: [{
				xtype: "hidden",
				name: "size"
			},{
				xtype: "sliderfield",
				name: "sizeslider",
				minValue: 0,
				decimalPrecision: 0,
				useTips: false,
				flex: 1,
				submitValue: false
			},{
				xtype: "textfield",
				name: "sizetext",
				width: 90,
				readOnly: true,
				submitValue: false
			}]
		}];
	},

	getRpcSetParams: function() {
		var me = this;
		var params = me.callParent(arguments);
		return Ext.apply(params, {
			devicefile: me.devicefile
		});
	},

	show: function() {
		var me = this;
		// Before the dialog can be displayed it is necessary to request
		// some details about the underlaying volume group.
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				if(success) {
					var currSize = this.size;
					var free = parseInt(response.free);
					// Display a info message if no free space is available
					// and close the dialog.
					if(0 >= free) {
						OMV.MessageBox.info(null, _("No free space available to extend the logical volume."));
						this.close();
					} else {
						// Finally show the dialog.
						me.superclass.show.call(me);
						// Update the 'Size' slider control.
						var field = this.findField("sizeslider");
						field.on("change", function(slider, newValue, thumb) {
							var newSize = currSize + newValue;
							// Update the hidden field storing the number
							// of bytes to use.
							var field = this.findField("size");
							field.setValue(newSize);
							// Display value in highest possible binary
							// unit in the textfield right beside the slider
							// control.
							field = this.findField("sizetext");
							field.setValue(newSize.binaryFormat());
						}, this);
						field.setMaxValue(free);
						field.setValue(free);
					}
				} else {
					OMV.MessageBox.error(null, response);
					this.close();
				}
			},
			rpcData: {
				service: "LogicalVolumeMgmt",
				method: "getVolumeGroup",
				params: {
					name: me.vgname
				}
			}
		});
	}
});

/**
 * @class OMV.module.admin.storage.lvm.LogicalVolumes
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.lvm.LogicalVolumes", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.storage.lvm.lv.Create",
		"OMV.module.admin.storage.lvm.lv.Edit",
		"OMV.module.admin.storage.lvm.lv.Extend"
	],

	hideAddButton: false,
	hideEditButton: false,
	hideDeleteButton: false,
	hideRefreshButton: true,
	hidePagingToolbar: false,
	reloadOnActivate: true,
	stateful: true,
	stateId: "87081dac-a91b-4a5e-901e-e69290b533ee",
	columns: [{
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name"
	},{
		xtype: "binaryunitcolumn",
		text: _("Capacity"),
		sortable: true,
		dataIndex: "size",
		stateId: "size"
	},{
		text: _("Volume group"),
		sortable: true,
		dataIndex: "vgname",
		stateId: "vgname"
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
						{ name: "uuid", type: "string" },
						{ name: "name", type: "string" },
						{ name: "size", type: "string" },
						{ name: "vgname", type: "string" },
						{ name: "_used", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getLogicalVolumesList"
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
			disabled: true,
			hidden: true // Not supported at the moment
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
		Ext.create("OMV.module.admin.storage.lvm.lv.Create", {
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
		Ext.create("OMV.module.admin.storage.lvm.lv.Edit", {
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
		Ext.create("OMV.module.admin.storage.lvm.lv.Extend", {
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			size: parseInt(record.get("size")),
			vgname: record.get("vgname"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
					OMV.MessageBox.info(null, _("After the logical volume has been extended you can resize the containing filesystem."));
				}
			}
		}).show();
	},

	onReduceButton: function() {
		// Not supported at the moment.
	},

	doDeletion: function(record) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "LogicalVolumeMgmt",
				method: "deleteLogicalVolume",
				params: {
					devicefile: record.get("devicefile")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "logicalvolumes",
	path: "/storage/lvm",
	text: _("Logical volumes"),
	position: 30,
	className: "OMV.module.admin.storage.lvm.LogicalVolumes"
});
