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
// require("js/omv/ModuleManager.js")
// require("js/omv/data/Connection.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/FormPanelDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/CheckboxGrid.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Storage.LVM");

// Append new vtypes used by LVM property dialogs.
// According to http://linux.die.net/man/8/lvm
Ext.apply(Ext.form.VTypes, {
	vgname: function(v) {
		return /^[a-zA-Z0-9+_\.\-]+$/.test(v);
	},
	vgnameText: _("Invalid name"),
	vgnameMask: /[a-zA-Z0-9+_\.\-]/,

	lvname: function(v) {
		return /^[a-zA-Z0-9+_\.\-]+$/.test(v);
	},
	lvnameText: _("Invalid name"),
	lvnameMask: /[a-zA-Z0-9+_\.\-]/
});

// Register the menu.
OMV.ModuleManager.registerMenu("storage", "lvm", {
	text: _("Logical Volume Management"),
	icon16: "images/lvm.png",
	position: 25
});

/**
 * @class OMV.Module.Storage.LVM.PhysicalVolumeGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.LVM.PhysicalVolumeGridPanel = function(config) {
	var initialConfig = {
		hideAddButton: false,
		hideEditButton: true,
		hideDeleteButton: false,
		hideRefreshButton: true,
		hidePagingToolbar: false,
		stateful: true,
		stateId: "4f7be743-777a-4547-9277-21a30ce0856b",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				text: _("Device"),
				sortable: true,
				dataIndex: "devicefile",
				stateId: "devicefile"
			},{
				text: _("Available"),
				sortable: true,
				dataIndex: "size",
				stateId: "size",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				text: _("Used"),
				sortable: true,
				dataIndex: "used",
				stateId: "used",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				text: _("Volume group"),
				sortable: true,
				dataIndex: "vgname",
				stateId: "vgName",
				renderer: function(value, metaData, record, rowIndex,
				  colIndex, store, view) {
					if(value.length == 0) {
						value = "-";
					}
					return value;
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
			proxy: new OMV.data.DataProxy({
				"rpcOptions": {
					"rpcData": {
						"service": "LogicalVolumeMgmt",
						"method": "getPhysicalVolumesList"
					}
				}
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

	onAddButton : function() {
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
		OMV.Ajax.request({
			  "scope": this,
			  "callback": this.onDeletion,
			  "rpcData": {
				  "service": "LogicalVolumeMgmt",
				  "method": "deletePhysicalVolume",
				  "params": {
					  "devicefile": record.get("devicefile")
				  }
			  }
		  });
	}
});
OMV.ModuleManager.registerPanel("storage", "lvm", {
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
		hideResetButton: true,
		width: 500
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.CreatePhysicalVolumeDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.CreatePhysicalVolumeDialog,
  OMV.FormPanelDialog, {
	getFormItems: function() {
		return [{
			xtype: "combo",
			name: "devicefile",
			fieldLabel: _("Device"),
			emptyText: _("Select an device ..."),
			store: new OMV.data.Store({
				proxy: new OMV.data.DataProxy({
					"rpcOptions": {
						"rpcData": {
							"service": "LogicalVolumeMgmt",
							"method": "getPhysicalVolumeCandidates"
						}
					},
					"appendSortParams": false
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
				if(answer === "no") {
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
		hideAddButton: false,
		hideEditButton: false,
		hideDeleteButton: false,
		hideRefreshButton: true,
		hidePagingToolbar: false,
		stateful: true,
		stateId: "af0712c4-9f60-493a-9be4-c9658f958f99",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				text: _("Name"),
				sortable: true,
				dataIndex: "name",
				stateId: "name"
			},{
				text: _("Available"),
				sortable: true,
				dataIndex: "size",
				stateId: "size",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				text: _("Free"),
				sortable: true,
				dataIndex: "free",
				stateId: "free",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				text: _("Physical volumes"),
				sortable: true,
				dataIndex: "pvname",
				stateId: "pvname",
				renderer: function(value, metaData, record, rowIndex,
				  colIndex, store, view) {
					var tpl = new Ext.XTemplate('<tpl for=".">{.}<br/></tpl>');
					return tpl.apply(record.get("pvname"));
				}
			},{
				text: _("Logical volumes"),
				sortable: true,
				dataIndex: "lvname",
				stateId: "lvname",
				renderer: function(value, metaData, record, rowIndex,
				  colIndex, store, view) {
					var tpl = new Ext.XTemplate('<tpl for=".">{.}<br/></tpl>');
					return tpl.apply(record.get("lvname"));
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
			proxy: new OMV.data.DataProxy({
				"rpcOptions": {
					"rpcData": {
						"service": "LogicalVolumeMgmt",
						"method": "getVolumeGroupsList"
					}
				}
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
					{ name: "pvname" },
					{ name: "lvname" },
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

	onSelectionChange: function(model, records) {
		OMV.Module.Storage.LVM.VolumeGroupGridPanel.superclass.
		  onSelectionChange.apply(this, arguments);
		// Process additional buttons
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
			var tbarBtnCtrl = this.queryById(this.getId() + "-" +
			  tbarBtnName[i]);
			if(!Ext.isEmpty(tbarBtnCtrl)) {
				if(true == tbarBtnDisabled[tbarBtnName[i]]) {
					tbarBtnCtrl.disable();
				} else {
					tbarBtnCtrl.enable();
				}
			}
		}
	},

	onAddButton : function() {
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

	onEditButton : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelection()[0];
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
		var record = selModel.getSelection()[0];
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
		var record = selModel.getSelection()[0];
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
		OMV.Ajax.request({
			  "scope": this,
			  "callback": this.onDeletion,
			  "rpcData": {
				  "service": "LogicalVolumeMgmt",
				  "method": "deleteVolumeGroup",
				  "params": {
					  "name": record.get("name")
				  }
			  }
		  });
	}
});
OMV.ModuleManager.registerPanel("storage", "lvm", {
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
		hideResetButton: true,
		width: 500
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.CreateVolumeGroupDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.CreateVolumeGroupDialog,
  OMV.FormPanelDialog, {
	getFormItems: function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "vgname"
		},{
			xtype: "checkboxgrid",
			name: "devices",
			fieldLabel: _("Devices"),
			store: new OMV.data.Store({
				autoLoad: true,
				proxy: new OMV.data.DataProxy({
					"rpcOptions": {
						"rpcData": {
							"service": "LogicalVolumeMgmt",
							"method": "getVolumeGroupCandidates"
						}
					},
					"appendSortParams": false
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
			stateful: true,
			stateId: "f5a7a042-ae59-4a5d-a1f9-276230cacab7",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					text: _("Description"),
					sortable: true,
					dataIndex: "description",
					stateId: "description"
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
				if(answer === "no") {
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
 * @param devicefile The devicefile of the volume group.
 * @param name The name of the volume group.
 */
OMV.Module.Storage.LVM.EditVolumeGroupDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "renameVolumeGroup",
		title: _("Edit volume group"),
		hideResetButton: false,
		width: 300
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.EditVolumeGroupDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.EditVolumeGroupDialog,
  OMV.FormPanelDialog, {
	getFormItems: function() {
		return [{
			xtype: "hidden",
			name: "devicefile",
			value: this.devicefile
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "vgname",
			value: this.name
		}];
	}
});

/**
 * @class OMV.Module.Storage.LVM.ExtendVolumeGroupDialog
 * @derived OMV.FormPanelDialog
 * @param devicefile The devicefile of the volume group.
 * @param name The name of the volume group.
 */
OMV.Module.Storage.LVM.ExtendVolumeGroupDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "extendVolumeGroup",
		title: _("Extend volume group"),
		hideResetButton: true,
		width: 450
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.ExtendVolumeGroupDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.ExtendVolumeGroupDialog,
  OMV.FormPanelDialog, {
	getFormItems: function() {
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
			vtype: "vgname",
			value: this.name
		},{
			xtype: "checkboxgrid",
			name: "devices",
			fieldLabel: _("Devices"),
			store: new OMV.data.Store({
				autoLoad: true,
				proxy: new OMV.data.DataProxy({
					"rpcOptions": {
						"rpcData": {
							"service": "LogicalVolumeMgmt",
							"method": "getVolumeGroupCandidates"
						}
					},
					"appendSortParams": false
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
			stateful: true,
			stateId: "713c4486-89cd-4e80-b21d-11921e5f717e",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					text: _("Description"),
					sortable: true,
					dataIndex: "description",
					stateId: "description"
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
 * @param devicefile The devicefile of the volume group.
 * @param name The name of the volume group.
 */
OMV.Module.Storage.LVM.ReduceVolumeGroupDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "reduceVolumeGroup",
		title: _("Reduce volume group"),
		hideResetButton: true,
		width: 450
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.ReduceVolumeGroupDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.ReduceVolumeGroupDialog,
  OMV.FormPanelDialog, {
	getFormItems: function() {
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
			vtype: "vgname",
			value: this.name
		},{
			xtype: "checkboxgrid",
			name: "devices",
			fieldLabel: _("Devices"),
			store: new OMV.data.Store({
				autoLoad: true,
				proxy: new OMV.data.DataProxy({
					"rpcOptions": {
						"rpcData": {
							"service": "LogicalVolumeMgmt",
							"method": "getVolumeGroupPhysicalVolumes"
						}
					},
					"extraParams": {
						"name": this.name
					},
					"appendSortParams": false
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
			stateful: true,
			stateId: "6060bab3-e7da-40a9-99cf-5f7570f16a68",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					text: _("Description"),
					sortable: true,
					dataIndex: "description",
					stateId: "description"
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
		hideAddButton: false,
		hideEditButton: false,
		hideDeleteButton: false,
		hideRefreshButton: true,
		hidePagingToolbar: false,
		stateful: true,
		stateId: "87081dac-a91b-4a5e-901e-e69290b533ee",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				text: _("Name"),
				sortable: true,
				dataIndex: "name",
				stateId: "name"
			},{
				text: _("Capacity"),
				sortable: true,
				dataIndex: "size",
				stateId: "size",
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				text: _("Volume group"),
				sortable: true,
				dataIndex: "vgname",
				stateId: "vgname"
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
			proxy: new OMV.data.DataProxy({
				"rpcOptions": {
					"rpcData": {
						"service": "LogicalVolumeMgmt",
						"method": "getLogicalVolumesList"
					}
				}
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

	onSelectionChange: function(model, records) {
		OMV.Module.Storage.LVM.LogicalVolumeGridPanel.superclass.
		  onSelectionChange.apply(this, arguments);
		// Process additional buttons
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
			var tbarBtnCtrl = this.queryById(this.getId() + "-" +
			  tbarBtnName[i]);
			if(!Ext.isEmpty(tbarBtnCtrl)) {
				if(true == tbarBtnDisabled[tbarBtnName[i]]) {
					tbarBtnCtrl.disable();
				} else {
					tbarBtnCtrl.enable();
				}
			}
		}
	},

	onAddButton : function() {
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

	onEditButton : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelection()[0];
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
		var record = selModel.getSelection()[0];
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
		var record = selModel.getSelection()[0];
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
		OMV.Ajax.request({
			  "scope": this,
			  "callback": this.onDeletion,
			  "rpcData": {
				  "service": "LogicalVolumeMgmt",
				  "method": "deleteLogicalVolume",
				  "params": {
					  "devicefile": record.get("devicefile")
				  }
			  }
		  });
	}
});
OMV.ModuleManager.registerPanel("storage", "lvm", {
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
		hideResetButton: true,
		width: 500,
		lvFree: 0
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.CreateLogicalVolumeDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.CreateLogicalVolumeDialog,
  OMV.FormPanelDialog, {
	getFormItems: function() {
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
			store: new OMV.data.Store({
				proxy: new OMV.data.DataProxy({
					"rpcOptions": {
						"rpcData": {
							"service": "LogicalVolumeMgmt",
							"method": "enumerateVolumeGroups"
						}
					},
					"appendSortParams": false
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
					var disabled = false;
					var free = parseInt(record.get("free"));
					// Update the 'Size' slider control.
					field = this.findField("sizeslider");
					if(0 >= free) {
						disabled = true;
						OMV.MessageBox.info(null, _("No free space available to create a logical volume."));
					} else {
						field.slider.on("change", function(c, newValue) {
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

	doSubmit : function() {
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to create the logical volume?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if(answer === "no") {
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
 * @param devicefile The devicefile of the volume group.
 * @param name The name of the volume group.
 */
OMV.Module.Storage.LVM.EditLogicalVolumeDialog = function(config) {
	var initialConfig = {
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "renameLogicalVolume",
		title: _("Edit logical volume"),
		hideResetButton: false,
		width: 300
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.EditLogicalVolumeDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.EditLogicalVolumeDialog,
  OMV.FormPanelDialog, {
	getFormItems: function() {
		return [{
			xtype: "hidden",
			name: "devicefile",
			value: this.devicefile
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "lvname",
			value: this.name
		}];
	}
});

/**
 * @class OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog
 * @derived OMV.FormPanelDialog
 * @param devicefile The devicefile of the logical volume.
 * @param name The name of the logical volume.
 * @param size The current size of the logical volume.
 * @param vgname The name of the volume group.
 */
OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog = function(config) {
	var initialConfig = {
		title: _("Extend logical volume"),
		rpcService: "LogicalVolumeMgmt",
		rpcSetMethod: "modifyLogicalVolume",
		hideResetButton: false,
		width: 500
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog,
  OMV.FormPanelDialog, {
	getFormItems: function() {
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
			vtype: "lvname",
			value: this.name
		},{
			xtype: "combo",
			name: "vgname",
			submitValue: false,
			fieldLabel: _("Volume group"),
			emptyText: _("Select a volume group ..."),
			store: new OMV.data.Store({
				proxy: new OMV.data.DataProxy({
					"rpcOptions": {
						"rpcData": {
							"service": "LogicalVolumeMgmt",
							"method": "getVolumeGroup"
						}
					},
					"extraParams": {
						"name": this.vgname
					},
					"appendSortParams": false
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

	show : function() {
		// Before the dialog can be displayed it is necessary to request
		// some details about the underlaying volume group.
		OMV.Ajax.request({
			  "scope": this,
			  "callback": function(id, response, error) {
				  if(error === null) {
					  var currSize = this.size;
					  var free = parseInt(response.free);
					  // Display a info message if no free space is available
					  // and close the dialog.
					  if(0 >= free) {
						  OMV.MessageBox.info(null, _("No free space available to extend the logical volume."));
						  this.close();
					  } else {
						  // Finally show the dialog.
						  OMV.Module.Storage.LVM.ExtendLogicalVolumeDialog.
							superclass.show.call(this, arguments);
						  // Update the 'Size' slider control.
						  var field = this.findField("sizeslider");
						  field.slider.on("change", function(c, newValue) {
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
					  OMV.MessageBox.error(null, error);
					  this.close();
				  }
			  },
			  "rpcData": {
				  "service": "LogicalVolumeMgmt",
				  "method": "getVolumeGroup",
				  "params": {
					  "name": this.vgname
				  }
			  }
		  });
	}
});
