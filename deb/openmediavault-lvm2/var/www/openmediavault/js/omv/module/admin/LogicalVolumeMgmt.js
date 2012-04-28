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
// require("js/omv/data/Connection.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/FormPanelDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/CheckboxGrid.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Storage.LVM");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("storage", "lvm", {
	text: _("Logical Volume Management"),
	icon: "images/lvm.png",
	position: 25
});

/**
 * @class OMV.Module.Storage.LVM.PhysicalVolumeGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.LVM.PhysicalVolumeGridPanel = function(config) {
	var initialConfig = {
		hideAdd: false,
		hideEdit: true,
		hideDelete: false,
		hideRefresh: true,
		hidePagingToolbar: false,
		stateId: "4f7be743-777a-4547-9277-21a30ce0856b",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Device"),
				sortable: true,
				dataIndex: "devicefile",
				id: "devicefile"
			},{
				header: _("Available"),
				sortable: true,
				dataIndex: "size",
				id: "size",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				header: _("Used"),
				sortable: true,
				dataIndex: "used",
				id: "used",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				header: _("Volume group"),
				sortable: true,
				dataIndex: "vgname",
				id: "vgName",
				renderer: function(val, cell, record, row, col, store) {
					if (val.length == 0) {
						val = "-";
					}
					return val;
				}
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.PhysicalVolumeGridPanel.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.PhysicalVolumeGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "LogicalVolumeMgmt",
				"method": "getPhysicalVolumesList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "devicefile",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "devicefile" },
					{ name: "used" },
					{ name: "size" },
					{ name: "vgname" },
					{ name: "_used" }
    			]
			})
		});
		OMV.Module.Storage.LVM.PhysicalVolumeGridPanel.superclass.
		  initComponent.apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Storage.LVM.PhysicalVolumeGridPanel.superclass.
		  initToolbar.apply(this);
		// Override 'Add' button in top toolbar
		var item = tbar.get(0);
		item.setText(_("Create"));
		item.setIcon("images/lvm-create.png");
		return tbar;
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Storage.LVM.CreatePhysicalVolumeDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "LogicalVolumeMgmt",
		  "deletePhysicalVolume", { "devicefile": record.get("devicefile") });
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "lvm", {
	cls: OMV.Module.Storage.LVM.PhysicalVolumeGridPanel,
	title: _("Physical volumes"),
	position: 10
});

/**
 * @class OMV.Module.Storage.LVM.CreatePhysicalVolumeDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Storage.LVM.CreatePhysicalVolumeDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "createPhysicalVolume",
		title: _("Create physical volume"),
		autoHeight: true,
		hideReset: true,
		width: 500
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.CreatePhysicalVolumeDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.CreatePhysicalVolumeDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "devicefile",
			hiddenName: "devicefile",
			fieldLabel: _("Device"),
			emptyText: _("Select an device ..."),
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "LogicalVolumeMgmt",
					"method": "getPhysicalVolumeCandidates",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "description" }
					]
				})
			}),
			displayField: "description",
			valueField: "devicefile",
			allowBlank: false,
			editable: false,
			triggerAction: "all"
		}];
	},

	doSubmit : function() {
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to create the physical volume?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if (answer === "no") {
					return;
				}
				OMV.Module.Storage.LVM.CreatePhysicalVolumeDialog.superclass.
				  doSubmit.apply(this, arguments);
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.Module.Storage.LVM.VolumeGroupGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.LVM.VolumeGroupGridPanel = function(config) {
	var initialConfig = {
		hideAdd: false,
		hideEdit: false,
		hideDelete: false,
		hideRefresh: true,
		hidePagingToolbar: false,
		stateId: "af0712c4-9f60-493a-9be4-c9658f958f99",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Name"),
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: _("Available"),
				sortable: true,
				dataIndex: "size",
				id: "size",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				header: _("Free"),
				sortable: true,
				dataIndex: "free",
				id: "free",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				header: _("Physical volumes"),
				sortable: true,
				dataIndex: "pvName",
				id: "pvName",
				renderer: function(val, cell, record, row, col, store) {
					var tpl = new Ext.XTemplate('<tpl for=".">{.}<br/></tpl>');
					return tpl.apply(record.get("pvName"));
				}
			},{
				header: _("Logical volumes"),
				sortable: true,
				dataIndex: "lvName",
				id: "lvName",
				renderer: function(val, cell, record, row, col, store) {
					var tpl = new Ext.XTemplate('<tpl for=".">{.}<br/></tpl>');
					return tpl.apply(record.get("lvName"));
				}
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.VolumeGroupGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.VolumeGroupGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "LogicalVolumeMgmt",
				"method": "getVolumeGroupsList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "devicefile",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "devicefile" },
					{ name: "name" },
					{ name: "free" },
					{ name: "size" },
					{ name: "pvName" },
					{ name: "lvName" },
					{ name: "_used" }
    			]
			})
		});
		OMV.Module.Storage.LVM.VolumeGroupGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Storage.LVM.VolumeGroupGridPanel.superclass.
		  initToolbar.apply(this);
		// Override 'Add' button in top toolbar
		var item = tbar.get(0);
		item.setText(_("Create"));
		item.setIcon("images/lvm-create.png");
		// Override 'Edit' button in top toolbar
		item = tbar.get(1);
		item.setIcon("images/lvm-edit.png");
		// Add 'Extend' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-extend",
			xtype: "button",
			text: _("Extend"),
			icon: "images/lvm-extend.png",
			handler: this.cbExtendBtnHdl,
			scope: this,
			disabled: true
		});
		// Add 'Reduce' button to top toolbar
		tbar.insert(3, {
			id: this.getId() + "-reduce",
			xtype: "button",
			text: _("Reduce"),
			icon: "images/lvm-reduce.png",
			handler: this.cbReduceBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Storage.LVM.VolumeGroupGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons
		var tbarBtnName = [ "extend","reduce" ];
		var tbarBtnDisabled = {
			"extend": true,
			"reduce": true
		};
		var records = model.getSelections();
		if (records.length <= 0) {
			tbarBtnDisabled["extend"] = true;
			tbarBtnDisabled["reduce"] = true;
		} else if (records.length == 1) {
			tbarBtnDisabled["extend"] = false;
			tbarBtnDisabled["reduce"] = false;
		} else {
			tbarBtnDisabled["extend"] = true;
			tbarBtnDisabled["reduce"] = true;
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

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Storage.LVM.CreateVolumeGroupDialog({
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
		var wnd = new OMV.Module.Storage.LVM.EditVolumeGroupDialog({
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbExtendBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.LVM.ExtendVolumeGroupDialog({
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbReduceBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.LVM.ReduceVolumeGroupDialog({
			devicefile: record.get("devicefile"),
			name: record.get("name"),
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "LogicalVolumeMgmt",
		  "deleteVolumeGroup", { "name": record.get("name") });
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "lvm", {
	cls: OMV.Module.Storage.LVM.VolumeGroupGridPanel,
	title: _("Volume groups"),
	position: 20
});

/**
 * @class OMV.Module.Storage.LVM.CreateVolumeGroupDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Storage.LVM.CreateVolumeGroupDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "createVolumeGroup",
		title: _("Create volume group"),
		autoHeight: true,
		hideReset: true,
		width: 500
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.CreateVolumeGroupDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.CreateVolumeGroupDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "devname"
		},{
			xtype: "checkboxgrid",
			name: "devices",
			hiddenName: "devices",
			fieldLabel: _("Devices"),
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "LogicalVolumeMgmt",
					"method": "getVolumeGroupCandidates",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "description" }
					]
				})
			}),
			valueField: "devicefile",
			stateId: "f5a7a042-ae59-4a5d-a1f9-276230cacab7",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: _("Description"),
					sortable: true,
					dataIndex: "description",
					id: "description"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			height: 110,
			minSelections: 1
		}];
	},

	doSubmit : function() {
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to create the volume group?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if (answer === "no") {
					return;
				}
				OMV.Module.Storage.LVM.CreateVolumeGroupDialog.superclass.
				  doSubmit.apply(this, arguments);
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.Module.Storage.LVM.EditVolumeGroupDialog
 * @derived OMV.FormPanelDialog
 * @config devicefile The devicefile of the volume group.
 * @config name The name of the volume group.
 */
OMV.Module.Storage.LVM.EditVolumeGroupDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "renameVolumeGroup",
		title: _("Edit volume group"),
		autoHeight: true,
		hideReset: false,
		width: 300
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.EditVolumeGroupDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.EditVolumeGroupDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "hidden",
			name: "devicefile",
			value: this.devicefile
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "devname",
			value: this.name
		}];
	}
});

/**
 * @class OMV.Module.Storage.LVM.ExtendVolumeGroupDialog
 * @derived OMV.FormPanelDialog
 * @config devicefile The devicefile of the volume group.
 * @config name The name of the volume group.
 */
OMV.Module.Storage.LVM.ExtendVolumeGroupDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "extendVolumeGroup",
		title: _("Extend volume group"),
		autoHeight: true,
		hideReset: true,
		width: 450
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.ExtendVolumeGroupDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.ExtendVolumeGroupDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "hidden",
			name: "devicefile",
			value: this.devicefile
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			readOnly: true,
			submitValue: false,
			vtype: "devname",
			value: this.name
		},{
			xtype: "checkboxgrid",
			name: "devices",
			hiddenName: "devices",
			fieldLabel: _("Devices"),
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "LogicalVolumeMgmt",
					"method": "getVolumeGroupCandidates",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "description" }
					]
				})
			}),
			valueField: "devicefile",
			stateId: "713c4486-89cd-4e80-b21d-11921e5f717e",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: _("Description"),
					sortable: true,
					dataIndex: "description",
					id: "description"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			height: 110,
			minSelections: 1
		}];
	}
});

/**
 * @class OMV.Module.Storage.LVM.ReduceVolumeGroupDialog
 * @derived OMV.FormPanelDialog
 * @config devicefile The devicefile of the volume group.
 * @config name The name of the volume group.
 */
OMV.Module.Storage.LVM.ReduceVolumeGroupDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "reduceVolumeGroup",
		title: _("Reduce volume group"),
		autoHeight: true,
		hideReset: true,
		width: 450
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.ReduceVolumeGroupDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.ReduceVolumeGroupDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "hidden",
			name: "devicefile",
			value: this.devicefile
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			readOnly: true,
			submitValue: false,
			vtype: "devname",
			value: this.name
		},{
			xtype: "checkboxgrid",
			name: "devices",
			hiddenName: "devices",
			fieldLabel: _("Devices"),
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "LogicalVolumeMgmt",
					"method": "getVolumeGroupPhysicalVolumes",
					"extraParams": { "name": this.name },
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "description" },
						{ name: "_used" }
					]
				}),
				listeners: {
					datachanged: function(store) {
						// Do not display devices that are in usage and not
						// removable. Suspend the firing of all events,
						// otherwise 'datachanged' will be fired which will
						// result in a invinite loop.
						store.suspendEvents();
						store.filter("_used", false);
						store.resumeEvents();
					},
					scope: this
				}
			}),
			valueField: "devicefile",
			stateId: "6060bab3-e7da-40a9-99cf-5f7570f16a68",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: _("Description"),
					sortable: true,
					dataIndex: "description",
					id: "description"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			height: 110,
			minSelections: 1
		}];
	}
});

/**
 * @class OMV.Module.Storage.LVM.LogicalVolumeGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.LVM.LogicalVolumeGridPanel = function(config) {
	var initialConfig = {
		hideAdd: false,
		hideEdit: false,
		hideDelete: false,
		hideRefresh: true,
		hidePagingToolbar: false,
		stateId: "87081dac-a91b-4a5e-901e-e69290b533ee",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Name"),
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: _("Capacity"),
				sortable: true,
				dataIndex: "size",
				id: "size",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				header: _("Volume group"),
				sortable: true,
				dataIndex: "vgname",
				id: "vgname"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.LogicalVolumeGridPanel.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.LogicalVolumeGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "LogicalVolumeMgmt",
				"method": "getLogicalVolumesList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "devicefile",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "devicefile" },
					{ name: "uuid" },
					{ name: "name" },
					{ name: "size" },
					{ name: "vgname" },
					{ name: "_used" }
    			]
			})
		});
		OMV.Module.Storage.LVM.LogicalVolumeGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Storage.LVM.LogicalVolumeGridPanel.superclass.
		  initToolbar.apply(this);
		// Override 'Add' button in top toolbar
		var item = tbar.get(0);
		item.setText(_("Create"));
		item.setIcon("images/lvm-create.png");
		// Override 'Edit' button in top toolbar
		item = tbar.get(1);
		item.setIcon("images/lvm-edit.png");
		// Add 'Extend' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-extend",
			xtype: "button",
			text: _("Extend"),
			icon: "images/lvm-extend.png",
			handler: this.cbExtendBtnHdl,
			scope: this,
			disabled: true
		});
		// Add 'Reduce' button to top toolbar
		tbar.insert(3, {
			id: this.getId() + "-reduce",
			xtype: "button",
			text: _("Reduce"),
			icon: "images/lvm-reduce.png",
			handler: this.cbReduceBtnHdl,
			scope: this,
			disabled: true,
			hidden: true // Not supported at the moment
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Storage.LVM.LogicalVolumeGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons
		var tbarBtnName = [ "extend","reduce" ];
		var tbarBtnDisabled = {
			"extend": true,
			"reduce": true
		};
		var records = model.getSelections();
		if (records.length <= 0) {
			tbarBtnDisabled["extend"] = true;
			tbarBtnDisabled["reduce"] = true;
		} else if (records.length == 1) {
			tbarBtnDisabled["extend"] = false;
			tbarBtnDisabled["reduce"] = false;
		} else {
			tbarBtnDisabled["extend"] = true;
			tbarBtnDisabled["reduce"] = true;
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

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Storage.LVM.CreateLogicalVolumeDialog({
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
		var wnd = new OMV.Module.Storage.LVM.EditLogicalVolumeDialog({
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbExtendBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog({
			devicefile: record.get("devicefile"),
			name: record.get("name"),
			size: parseInt(record.get("size")),
			vgname: record.get("vgname"),
			listeners: {
				submit: function() {
					this.doReload();
					OMV.MessageBox.info(null, _("After the logical volume has been extended you can resize the containing filesystem."));
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbReduceBtnHdl : function() {
/*
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.LVM.ReduceLogicalVolumeDialog({
			devicefile: record.get("devicefile"),
			size: parseInt(record.get("size")),
			vgname: record.get("vgname"),
			listeners: {
				submit: function() {
					this.doReload();
					OMV.MessageBox.info(null, "After the logical volume " +
					  "has been reduced you can resize the containing " +
					  "filesystem.");
				},
				scope: this
			}
		});
		wnd.show();
*/
	},

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "LogicalVolumeMgmt",
		  "deleteLogicalVolume", { "devicefile": record.get("devicefile") });
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "lvm", {
	cls: OMV.Module.Storage.LVM.LogicalVolumeGridPanel,
	title: _("Logical volumes"),
	position: 30
});

/**
 * @class OMV.Module.Storage.LVM.CreateLogicalVolumeDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Storage.LVM.CreateLogicalVolumeDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "createLogicalVolume",
		title: _("Create logical volume"),
		autoHeight: true,
		hideReset: true,
		width: 500,
		lvFree: 0
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.CreateLogicalVolumeDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.CreateLogicalVolumeDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		var fn = function(c, value) {
			var maxSize = this.vgFree;
			var unit = this.findFormField("unit").getValue();
			var bytes = value.binaryConvert(unit, "B");
			c.maxValue = maxSize.binaryConvert("B", unit);
			c.clearInvalid();
			if (bytes > maxSize) {
				var msg = String.format(_("The maximum value for this field is {0}"), c.maxValue);
				c.markInvalid(msg);
			}
		}
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "devname"
		},{
			xtype: "combo",
			name: "vgname",
			hiddenName: "vgname",
			fieldLabel: _("Volume group"),
			emptyText: _("Select a volume group ..."),
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "LogicalVolumeMgmt",
					"method": "enumerateVolumeGroups",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "description" },
						{ name: "free" }
					]
				})
			}),
			displayField: "description",
			valueField: "devicefile",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			listeners: {
				scope: this,
				select: function(combo, record, index) {
					var free = parseInt(record.get("free"));
					var value = free.binaryConvert("B", "MiB");
					this.vgFree = free;
					this.findFormField("unit").setValue("MiB");
					this.findFormField("size").setValue(value);
				}
			}
		},{
			xtype: "compositefield",
			fieldLabel: _("Size"),
			combineErrors: false,
			items: [{
				xtype: "numberfield",
				name: "size",
				minValue: 1,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				flex: 1,
				listeners: {
					scope: this,
					blur: function(c) {
						fn.call(this, c, c.getValue());
					},
					change: function(c, newValue, oldValue) {
						fn.call(this, c, newValue);
					}
				}
			},{
				xtype: "combo",
				name: "unit",
				width: 60,
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ "MiB",_("MiB") ],
						[ "GiB",_("GiB") ],
						[ "TiB",_("TiB") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				triggerAction: "all",
				editable: false,
				forceSelection: true,
				value: "MiB",
				listeners: {
					scope: this,
					select: function(c, record, index) {
						var field = this.findFormField("size");
						fn.call(this, field, field.getValue());
					}
				}
			}]
		}];
	},

	doSubmit : function() {
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to create the logical volume?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if (answer === "no") {
					return;
				}
				OMV.Module.Storage.LVM.CreateLogicalVolumeDialog.superclass.
				  doSubmit.apply(this, arguments);
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.Module.Storage.LVM.EditLogicalVolumeDialog
 * @derived OMV.FormPanelDialog
 * @config devicefile The devicefile of the volume group.
 * @config name The name of the volume group.
 */
OMV.Module.Storage.LVM.EditLogicalVolumeDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "renameLogicalVolume",
		title: _("Edit logical volume"),
		autoHeight: true,
		hideReset: false,
		width: 300
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.EditLogicalVolumeDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.EditLogicalVolumeDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "hidden",
			name: "devicefile",
			value: this.devicefile
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "devname",
			value: this.name
		}];
	}
});

/**
 * @class OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog
 * @derived OMV.FormPanelDialog
 * @config devicefile The devicefile of the logical volume.
 * @config name The name of the logical volume.
 * @config size The current size of the logical volume.
 * @config vgname The name of the volume group.
 */
OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog = function(config) {
	var initialConfig = {
		title: _("Extend logical volume"),
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "modifyLogicalVolume",
		autoHeight: true,
		hideReset: false,
		width: 500
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog,
  OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		var si = this.size.binaryFormat({ indexed: true });
		var fn = function(c, value) {
			var maxSize = this.size + this.vgFree;
			var unit = this.findFormField("unit").getValue();
			var bytes = value.binaryConvert(unit, "B");
			c.maxValue = maxSize.binaryConvert("B", unit);
			c.clearInvalid();
			if (bytes > maxSize) {
				var msg = String.format(_("The maximum value for this field is {0}"), c.maxValue);
				c.markInvalid(msg);
			} else if (bytes < this.size) {
				var msg = String.format(_("The minimum value for this field is {0}"), this.size.binaryConvert("B", unit));
				c.markInvalid(msg);
			}
		}
		return [{
			xtype: "hidden",
			name: "devicefile",
			value: this.devicefile
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			readOnly: true,
			submitValue: false,
			vtype: "devname",
			value: this.name
		},{
			xtype: "combo",
			name: "vgname",
//			hiddenName: "vgname", // Do not submit form field
			fieldLabel: _("Volume group"),
			emptyText: _("Select a volume group ..."),
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "LogicalVolumeMgmt",
					"method": "getVolumeGroup",
					"extraParams": { "name": this.vgname },
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "name",
					fields: [
						{ name: "name" },
						{ name: "description" }
					]
				})
			}),
			displayField: "description",
			valueField: "name",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			readOnly: true,
			submitValue: false,
			value: this.vgname
		},{
			xtype: "compositefield",
			fieldLabel: _("Size"),
			combineErrors: false,
			items: [{
				xtype: "numberfield",
				name: "size",
				minValue: 1,
				value: si.value,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				flex: 1,
				listeners: {
					scope: this,
					blur: function(c) {
						fn.call(this, c, c.getValue());
					},
					change: function(c, newValue, oldValue) {
						fn.call(this, c, newValue);
					}
				}
			},{
				xtype: "combo",
				name: "unit",
				width: 60,
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ "MiB",_("MiB") ],
						[ "GiB",_("GiB") ],
						[ "TiB",_("TiB") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				triggerAction: "all",
				editable: false,
				forceSelection: true,
				value: si.unit,
				listeners: {
					scope: this,
					select: function(c, record, index) {
						var field = this.findFormField("size");
						fn.call(this, field, field.getValue());
					}
				}
			}]
		}];
	},

	show : function() {
		// Before the dialog can be displayed it is necessary to request
		// some details about the underlaying volume group.
		OMV.Ajax.request(function(id, response, error) {
			if (error === null) {
				// Get the free capacity of the volume group.
				this.vgFree = parseInt(response.free);
				// Finally show the dialog.
				return OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog.
				  superclass.show.call(this, arguments);
			} else {
				OMV.MessageBox.error(null, error);
			}
		}, this, "LogicalVolumeMgmt", "getVolumeGroup", [ this.vgname ]);
	}
});
