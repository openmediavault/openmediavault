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
// require("js/omv/PluginMgr.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/ExecCmdDialog.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/CfgObjectTabDialog.js")
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/module/admin/Logs.js")
// require("js/omv/util/Format.js")

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "rsync", {
	text: _("Rsync"),
	icon: "images/rsync.png"
});

Ext.ns("OMV.Module.Services.Rsyncd");

/**
 * @class OMV.Module.Services.Rsyncd.TabPanel
 * @derived Ext.TabPanel
 */
OMV.Module.Services.Rsyncd.TabPanel = function(config) {
	var initialConfig = {
		border: false,
		activeTab: 0,
		layoutOnTabChange: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsyncd.TabPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsyncd.TabPanel, Ext.TabPanel, {
	initComponent : function() {
		this.items = [
			new OMV.Module.Services.Rsyncd.SettingsPanel,
			new OMV.Module.Services.Rsyncd.ModuleGridPanel
		];
		OMV.Module.Services.Rsyncd.TabPanel.superclass.initComponent.apply(
		  this, arguments);
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "rsync", {
	cls: OMV.Module.Services.Rsyncd.TabPanel,
	title: _("Server"),
	position: 10
});

/**
 * @class OMV.Module.Services.Rsyncd.SettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.Rsyncd.SettingsPanel = function(config) {
	var initialConfig = {
		title: _("Settings"),
		rpcService: "Rsyncd",
		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsyncd.SettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsyncd.SettingsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("General settings"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 873
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "3",
					cols: "65"
				},
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Please check the <a href='http://www.samba.org/ftp/rsync/rsyncd.conf.html' target='_blank'>manual page</a> for more details."),
				anchor: "100%"
			}]
		}];
	}
});

/**
 * @class OMV.Module.Services.Rsyncd.ModuleGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.Rsyncd.ModuleGridPanel = function(config) {
	var initialConfig = {
		title: _("Modules"),
		hidePagingToolbar: false,
		stateId: "72d6ab93-f08d-4d34-820b-fcbb832f723c",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Shared folder"),
				sortable: true,
				dataIndex: "sharedfoldername",
				id: "sharedfoldername"
			},{
				header: _("Name"),
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsyncd.ModuleGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsyncd.ModuleGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "Rsyncd",
				"method": "getModuleList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "name" },
					{ name: "sharedfoldername" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Services.Rsyncd.ModuleGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.Rsyncd.ModulePropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
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
		var wnd = new OMV.Module.Services.Rsyncd.ModulePropertyDialog({
			uuid: record.get("uuid"),
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "Rsyncd",
		  "deleteModule", { "uuid": record.get("uuid") });
	}
});

/**
 * @class OMV.Module.Services.Rsyncd.ModulePropertyDialog
 * @derived OMV.CfgObjectTabDialog
 */
OMV.Module.Services.Rsyncd.ModulePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Rsyncd",
		rpcGetMethod: "getModule",
		rpcSetMethod: "setModule",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add module") : _("Edit module"),
		width: 600,
		height: 450
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsyncd.ModulePropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsyncd.ModulePropertyDialog,
  OMV.CfgObjectTabDialog, {
	getTabItems : function() {
		return [
			new OMV.Module.Services.Rsyncd.ModuleGeneralPanel({
				"uuid": this.uuid
			}),
			new OMV.Module.Services.Rsyncd.ModuleAuthUsersGrid
		];
	},

	isValid : function() {
		var valid = OMV.Module.Services.Rsyncd.ModulePropertyDialog.
		  superclass.isValid.call(this);
		if (!valid)
			return valid;
		var values = this.getValues();
		if ((values.authusers === true) && (values.users.length == 0)) {
			var tab = this.getTab().find("title", _("General"));
			var basicForm = tab[0].getForm();
			basicForm.markInvalid([
				{ id: "authusers", msg: _("No users defined") }
			]);
			this.markInvalid(tab[0]);
			valid = false;
		}
		return valid;
	}
});

/**
 * @class OMV.Module.Services.Rsyncd.ModuleGeneralPanel
 * @derived OMV.form.FormPanel
 */
OMV.Module.Services.Rsyncd.ModuleGeneralPanel = function(config) {
	var initialConfig = {
		title: _("General"),
		autoScroll: true,
		trackResetOnLoad: true,
		defaults: {
			anchor: "-" + Ext.getScrollBarWidth()
		}
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsyncd.ModuleGeneralPanel.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsyncd.ModuleGeneralPanel, OMV.form.FormPanel, {
	initComponent : function() {
		this.items = [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "sharename",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The name of the share.")
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			hiddenName: "sharedfolderref",
			readOnly: (this.uuid !== OMV.UUID_UNDEFINED),
			fieldLabel: _("Shared folder"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The location of the files to share.")
		},{
			xtype: "combo",
			name: "uid",
			hiddenName: "uid",
			fieldLabel: _("User"),
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "UserMgmt",
					"method": "enumerateAllUsers",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "name",
					fields: [
						{ name: "name" }
					]
				})
			}),
			emptyText: _("Select a user ..."),
			valueField: "name",
			displayField: "name",
			value: "nobody",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("This option specifies the user name that file transfers to and from that module should take place.")
		},{
			xtype: "combo",
			name: "gid",
			hiddenName: "gid",
			fieldLabel: _("Group"),
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "UserMgmt",
					"method": "enumerateAllGroups",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "name",
					fields: [
						{ name: "name" }
					]
				})
			}),
			emptyText: _("Select a group ..."),
			valueField: "name",
			displayField: "name",
			value: "users",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("This option specifies the group name that file transfers to and from that module should take place.")
		},{
			xtype: "checkbox",
			name: "authusers",
			fieldLabel: _("Authenticate users"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Enable user authentication"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("If set then the client will be challenged to supply a username and password to connect to the module.")
		},{
			xtype: "checkbox",
			name: "readonly",
			fieldLabel: _("Read only"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Set read only"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("If this option is set, then any attempted uploads will fail.")
		},{
			xtype: "checkbox",
			name: "writeonly",
			fieldLabel: _("Write only"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Set write only"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("If this option is set, then any attempted downloads will fail.")
		},{
			xtype: "checkbox",
			name: "list",
			fieldLabel: _("List"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Enable module listing"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("This option determines if this module should be listed when the client asks for a listing of available modules.")
		},{
			xtype: "numberfield",
			name: "maxconnections",
			fieldLabel: _("Max. connections"),
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			value: 0,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("This option specifies the maximum number of simultaneous connections. 0 means no limit.")
		},{
			xtype: "textfield",
			name: "hostsallow",
			fieldLabel: _("Hosts allow"),
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("This option is a comma, space, or tab delimited set of hosts which are permitted to access this module. You can specify the hosts by name or IP number. Leave this field empty to use default settings.")
		},{
			xtype: "textfield",
			name: "hostsdeny",
			fieldLabel: _("Hosts deny"),
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("This option is a comma, space, or tab delimited set of host which are NOT permitted to access this module. Where the lists conflict, the allow list takes precedence. In the event that it is necessary to deny all by default, use the keyword ALL (or the netmask 0.0.0.0/0) and then explicitly specify to the hosts allow parameter those hosts that should be permitted access. Leave this field empty to use default settings.")
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: _("Extra options"),
			allowBlank: true,
			autoCreate: {
				tag: "textarea",
				autocomplete: "off",
				rows: "3",
				cols: "65"
			},
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("Please check the <a href='http://www.samba.org/ftp/rsync/rsyncd.conf.html' target='_blank'>manual page</a> for more details.")
		}];
		OMV.Module.Services.Rsyncd.ModuleGeneralPanel.superclass.
		  initComponent.apply(this, arguments);
		this.on("load", this._updateFormFields, this);
	},

	isValid : function() {
		var valid = OMV.Module.Services.Rsyncd.ModuleGeneralPanel.
		  superclass.isValid.call(this);
		if (!valid)
			return valid;
		// Do additional checks
		var values = this.getValues();
		if (values.readonly && values.writeonly) {
			var basicForm = this.getForm();
			var msg = _("Fields must be exclusive");
			basicForm.markInvalid([
				{ id: "readonly", msg: msg },
				{ id: "writeonly", msg: msg }
			]);
			valid = false;
		}
		return valid;
	}
});

/**
 * @class OMV.Module.Services.Rsyncd.ModuleAuthUsersGrid
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.Rsyncd.ModuleAuthUsersGrid = function(config) {
	var initialConfig = {
		title: _("Users"),
		mode: "local",
		stateId: "42a21c5a-3dc5-11e1-9f73-af7c01c6821f",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Name"),
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: _("Password"),
				sortable: false,
				dataIndex: "password",
				id: "password",
				renderer: function(val, cell, record, row, col, store) {
					return val.replace(/./g, "*");
				}
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsyncd.ModuleAuthUsersGrid.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsyncd.ModuleAuthUsersGrid,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			remoteSort: false,
			reader: new Ext.data.JsonReader({
				idProperty: "name",
				fields: [
					{ name: "name" },
					{ name: "password" }
    			]
			})
		});
		OMV.Module.Services.Rsyncd.ModuleAuthUsersGrid.superclass.
		  initComponent.apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.Rsyncd.ModuleAuthUserPropertyDialog({
			title: _("Add user"),
			listeners: {
				submit: function(dlg, data) {
					// Check if user already exists
					if (this.store.findExact("name", data.name) != -1) {
						OMV.MessageBox.failure(null, "User already exists.");
						return;
					}
					// Add new record
					var record = new this.store.recordType(data);
					record.markDirty();
					this.store.add(record);
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Services.Rsyncd.ModuleAuthUserPropertyDialog({
			title: _("Edit user"),
			listeners: {
				submit: function(dlg, data) {
					// Update the selected record
					record.beginEdit();
					for (var prop in data) {
						record.set(prop, data[prop]);
					}
					record.endEdit();
				},
				scope: this
			}
		});
		wnd.findFormField("name").setReadOnly(true);
		wnd.setValues(record.data);
		wnd.show();
	},

	/**
	 * @method setValues
	 * Load values into the grids store.
	 * @param values The values to load into the grids store.
	 */
	setValues : function(values) {
		OMV.Module.Services.Rsyncd.ModuleAuthUsersGrid.superclass.
		  setValues.call(this, values.users);
	},

	/**
	 * @method getValues
	 * Returns the records of the grids store as object with key/value pairs.
	 * @return The records of the grids store as key/value pairs.
	 */
	getValues : function() {
		var users = OMV.Module.Services.Rsyncd.ModuleAuthUsersGrid.superclass.
		  getValues.call(this);
		var values = {
			users: users
		};
		return values;
	}
});

/**
 * @class OMV.Module.Services.Rsyncd.ModuleAuthUserPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.Rsyncd.ModuleAuthUserPropertyDialog = function(config) {
	var initialConfig = {
		mode: "local",
		hideReset: true,
		width: 300,
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsyncd.ModuleAuthUserPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsyncd.ModuleAuthUserPropertyDialog,
  OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		}
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: _("Password"),
			allowBlank: false
		}];
	}
});

Ext.ns("OMV.Module.Services.Rsync");

/**
 * @class OMV.Module.Services.Rsync.JobsGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.Rsync.JobsGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "31924bfb-8e25-4ada-82f4-99a3a5c9e9a5",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Enabled"),
				sortable: true,
				dataIndex: "enable",
				id: "enable",
				align: "center",
				width: 60,
				renderer: OMV.util.Format.booleanRenderer()
			},{
				header: _("Source"),
				sortable: true,
				dataIndex: "srcname",
				id: "srcname"
			},{
				header: _("Destination"),
				sortable: true,
				dataIndex: "destname",
				id: "destname"
			},{
				header: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsync.JobsGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsync.JobsGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "Rsync",
				"method": "getList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "enable" },
					{ name: "srcname" },
					{ name: "destname" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Services.Rsync.JobsGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Services.Rsync.JobsGridPanel.superclass.
		  initToolbar.apply(this);
		// Add 'Run' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-run",
			xtype: "button",
			text: _("Run"),
			icon: "images/run.png",
			handler: this.cbRunBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Services.Rsync.JobsGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons
		var records = model.getSelections();
		var tbarRunCtrl = this.getTopToolbar().findById(this.getId() + "-run");
		if (records.length <= 0) {
			tbarRunCtrl.disable();
		} else if (records.length == 1) {
			tbarRunCtrl.enable();
		} else {
			tbarRunCtrl.disable();
		}
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.Rsync.JobPropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
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
		var wnd = new OMV.Module.Services.Rsync.JobPropertyDialog({
			uuid: record.get("uuid"),
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "Rsync",
		  "delete", { "uuid": record.get("uuid") });
	},

	cbRunBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.ExecCmdDialog({
			title: _("Execute rsync job"),
			rpcService: "Rsync",
			rpcMethod: "execute",
			rpcArgs: { "uuid": record.get("uuid") },
			listeners: {
				exception: function(wnd, error) {
					OMV.MessageBox.error(null, error);
				},
				scope: this
			}
		});
		wnd.show();
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "rsync", {
	cls: OMV.Module.Services.Rsync.JobsGridPanel,
	title: _("Jobs"),
	position: 20
});

/**
 * @class OMV.Module.Services.Rsync.JobPropertyDialog
 */
OMV.Module.Services.Rsync.JobPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Rsync",
		rpcGetMethod: "get",
		rpcSetMethod: "set",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add rsync job") : _("Edit rsync job"),
		width: 570,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Rsync.JobPropertyDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.Rsync.JobPropertyDialog, OMV.CfgObjectDialog, {
	initComponent : function() {
		OMV.Module.Services.Rsync.JobPropertyDialog.superclass.initComponent.
		  apply(this, arguments);
		this.on("load", this._updateFormFields, this);
		this.on("show", this._updateFormFields, this);
	},

	getFormConfig : function() {
		return {
			autoScroll: true,
			defaults: {
				anchor: "-" + Ext.getScrollBarWidth(),
				labelSeparator: ""
			}
		};
	},

	getFormItems : function() {
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: true,
			inputValue: 1
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		},{
			xtype: "combo",
			name: "type",
			hiddenName: "type",
			fieldLabel: _("Type"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "local",_("Local") ],
					[ "remote",_("Remote") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "local",
			listeners: {
				select: function(combo, record, index) {
					// Update the form fields
					this._updateFormFields();
				},
				scope: this
			}
		},{
			xtype: "combo",
			name: "mode",
			hiddenName: "mode",
			fieldLabel: _("Mode"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "push",_("Push") ],
					[ "pull",_("Pull") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "push",
			listeners: {
				select: function(combo, record, index) {
					// Update the form fields
					this._updateFormFields();
				},
				scope: this
			}
		},{
			xtype: "sharedfoldercombo",
			name: "srcsharedfolderref",
			hiddenName: "srcsharedfolderref",
			fieldLabel: _("Source shared folder"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The source shared folder.")
		},{
			xtype: "textfield",
			name: "srcuri",
			fieldLabel: _("Source server"),
			allowBlank: true,
			hidden: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The source remote server, e.g. [USER@]HOST:SRC, [USER@]HOST::SRC or rsync://[USER@]HOST[:PORT]/SRC.")
		},{
			xtype: "sharedfoldercombo",
			name: "destsharedfolderref",
			hiddenName: "destsharedfolderref",
			fieldLabel: _("Destination shared folder"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The source shared folder.")
		},{
			xtype: "textfield",
			name: "desturi",
			fieldLabel: _("Destination server"),
			allowBlank: true,
			hidden: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The destination remote server, e.g. [USER@]HOST:DEST, [USER@]HOST::DEST or rsync://[USER@]HOST[:PORT]/DEST.")
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: _("Password"),
			allowBlank: true,
			hidden: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The password that is used for access via rsync daemon. Note, this is not used for remote shell transport such as ssh.")
		},{
			xtype: "compositefield",
			fieldLabel: _("Minute"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "minute",
				hiddenName: "minute",
				mode: "local",
				store: Array.range(0, 59, 1, true).insert(0, "*"),
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: new Date().format("i"),
				flex: 1
			},{
				xtype: "checkbox",
				name: "everynminute",
				fieldLabel: "",
				checked: false,
				inputValue: 1,
				boxLabel: _("Every N minute"),
				width: 140
			}]
		},{
			xtype: "compositefield",
			fieldLabel: _("Hour"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "hour",
				hiddenName: "hour",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value", "text" ],
					data: Date.mapHour
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: new Date().format("H"),
				flex: 1
			},{
				xtype: "checkbox",
				name: "everynhour",
				fieldLabel: "",
				checked: false,
				inputValue: 1,
				boxLabel: _("Every N hour"),
				width: 140
			}]
		},{
			xtype: "compositefield",
			fieldLabel: _("Day of month"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "dayofmonth",
				hiddenName: "dayofmonth",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value", "text" ],
					data: Date.mapDayOfMonth
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "*",
				flex: 1
			},{
				xtype: "checkbox",
				name: "everyndayofmonth",
				fieldLabel: "",
				checked: false,
				inputValue: 1,
				boxLabel: _("Every N day of month"),
				width: 140
			}]
		},{
			xtype: "combo",
			name: "month",
			hiddenName: "month",
			fieldLabel: _("Month"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapMonth
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "combo",
			name: "dayofweek",
			hiddenName: "dayofweek",
			fieldLabel: _("Day of week"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapDayOfWeek
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "checkbox",
			name: "recursive",
			fieldLabel: _("Recursive"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Recurse into directories")
		},{
			xtype: "checkbox",
			name: "times",
			fieldLabel: _("Times"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Preserve modification times")
		},{
			xtype: "checkbox",
			name: "compress",
			fieldLabel: _("Compress"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Compress file data during the transfer")
		},{
			xtype: "checkbox",
			name: "archive",
			fieldLabel: _("Archive"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Enable archive mode")
		},{
			xtype: "checkbox",
			name: "delete",
			fieldLabel: _("Delete"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Delete files on the receiving side that don't exist on sender")
		},{
			xtype: "checkbox",
			name: "quiet",
			fieldLabel: _("Quiet"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Suppress non-error messages")
		},{
			xtype: "checkbox",
			name: "perms",
			fieldLabel: _("Preserve permissions"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Set the destination permissions to be the same as the source permissions")
		},{
			xtype: "checkbox",
			name: "acls",
			fieldLabel: _("Preserve ACLs"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Update the destination ACLs to be the same as the source ACLs")
		},{
			xtype: "checkbox",
			name: "xattrs",
			fieldLabel: _("Preserve extended attributes"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Update the destination extended attributes to be the same as the local ones")
		},{
			xtype: "checkbox",
			name: "sendemail",
			fieldLabel: _("Send email"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Send command output via email"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("An email message with the command output (if any produced) is send to the administrator.")
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: _("Extra options"),
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("Please check the <a href='http://www.samba.org/ftp/rsync/rsync.html' target='_blank'>manual page</a> for more details.")
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var values = this.getValues();
		if (this.uuid !== OMV.UUID_UNDEFINED) {
			this.findFormField("type").setReadOnly(true);
		}
		switch (values.type) {
		case "local":
			var fields = [ "srcsharedfolderref", "destsharedfolderref" ];
			for (var i = 0; i < fields.length; i++) {
				var field = this.findFormField(fields[i]);
				field.enable();
				field.show();
				field.allowBlank = false;
			}
			fields = [ "srcuri", "desturi" ];
			for (i = 0; i < fields.length; i++) {
				var field = this.findFormField(fields[i]);
				field.disable();
				field.hide();
				field.allowBlank = true;
			}
			this.findFormField("mode").hide();
			this.findFormField("password").hide();
			break;
		case "remote":
			this.findFormField("mode").show();
			this.findFormField("password").show();
			switch (values.mode) {
			case "push":
				var fields = [ "srcsharedfolderref", "desturi" ];
				for (var i = 0; i < fields.length; i++) {
					var field = this.findFormField(fields[i]);
					field.enable();
					field.show();
					field.allowBlank = false;
				}
				fields = [ "srcuri", "destsharedfolderref" ];
				for (i = 0; i < fields.length; i++) {
					var field = this.findFormField(fields[i]);
					field.disable();
					field.hide();
					field.allowBlank = true;
				}
				break;
			case "pull":
				var fields = [ "srcuri", "destsharedfolderref" ];
				for (var i = 0; i < fields.length; i++) {
					var field = this.findFormField(fields[i]);
					field.enable();
					field.show();
					field.allowBlank = false;
				}
				fields = [ "srcsharedfolderref", "desturi" ];
				for (i = 0; i < fields.length; i++) {
					var field = this.findFormField(fields[i]);
					field.disable();
					field.hide();
					field.allowBlank = true;
				}
				break;
			}
			break;
		}
	},

	isValid : function() {
		var valid = OMV.Module.Services.Rsync.JobPropertyDialog.superclass.
		  isValid.call(this);
		if (!valid) {
			return valid;
		}
		// Do additional checks
		var values = this.getValues();
		if (values.type === "local") {
			if (values.srcsharedfolderref === values.destsharedfolderref) {
				var basicForm = this.form.getForm();
				var msg = _("Shared folder must not be equal");
				basicForm.markInvalid([
					{ id: "srcsharedfolderref", msg: msg },
					{ id: "destsharedfolderref", msg: msg }
				]);
				valid = false;
			}
		}
		// It is not allowed to select '*' if the everyxxx checkbox is checked.
		[ "minute", "hour", "dayofmonth" ].each(function(fieldName) {
			var field = this.findFormField(fieldName);
			field.clearInvalid(); // combineErrors is false
			if ((field.getValue() === "*") && (this.findFormField(
			  "everyn" + fieldName).checked)) {
				field.markInvalid(_("Ranges of numbers are not allowed"));
				valid = false;
			}
		}, this);
		return valid;
	}
});

Ext.ns("OMV.Module.Diagnostics.LogPlugin");

/**
 * @class OMV.Module.Diagnostics.LogPlugin.Rsync
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'Rsync' logfile diagnostics plugin
 */
OMV.Module.Diagnostics.LogPlugin.Rsync = function(config) {
	var initialConfig = {
		title: _("Rsync"),
		stateId: "835f3a32-bc7d-4102-b1cb-a598a0ae14ad",
		columns: [{
			header: _("Date & Time"),
			sortable: true,
			dataIndex: "rownum",
			id: "date",
			width: 35,
			renderer: function(val, cell, record, row, col, store) {
				return OMV.util.Format.localeTime(record.get("date"));
			}
		},{
			header: _("Event"),
			sortable: true,
			dataIndex: "event",
			id: "event"
		}],
		rpcArgs: { "id": "rpcArgs" },
		rpcFields: [
			{ name: "rownum" },
			{ name: "date" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.Rsync.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.Rsync,
  OMV.Module.Diagnostics.LogPlugin, {
});
OMV.preg("log", "rsync", OMV.Module.Diagnostics.LogPlugin.Rsync);
