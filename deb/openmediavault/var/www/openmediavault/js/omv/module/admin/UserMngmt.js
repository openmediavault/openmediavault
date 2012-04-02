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
// require("js/omv/FormPanelDialog.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/CheckboxGrid.js")
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Privileges");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("privileges", "users", {
	text: _("User"),
	icon: "images/user.png",
	position: 10
});
OMV.NavigationPanelMgr.registerMenu("privileges", "groups", {
	text: _("Group"),
	icon: "images/group.png",
	position: 20
});

/**
 * @class OMV.Module.Privileges.UserGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Privileges.UserGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "98d6fe31-8e12-407b-82f2-7e0acf4006c1",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Name"),
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: _("Email"),
				sortable: true,
				dataIndex: "email",
				id: "email"
			},{
				header: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			},{
				header: _("Groups"),
				sortable: true,
				dataIndex: "groups",
				id: "groups",
				renderer: function(val, cell, record, row, col, store) {
					return val.join(",");
				},
				scope: this
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.UserGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.UserGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "UserMgmt",
				"method": "getUserList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "name",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "name" },
					{ name: "email" },
					{ name: "groups" },
					{ name: "comment" },
					{ name: "_used" }
    			]
			})
		});
		OMV.Module.Privileges.UserGridPanel.superclass.initComponent.apply(
		  this, arguments);
		this.on("activate", this.doReload, this);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Privileges.UserGridPanel.superclass.
		  initToolbar.apply(this, arguments);
		// Replace the default 'Add' button
		tbar.remove(0);
		tbar.insert(0, new Ext.SplitButton({
			text: _("Add"),
			icon: "images/add.png",
			handler: function() {
				this.showMenu();
			},
			menu: new Ext.menu.Menu({
				items: [
					{ text: _("Add"), value: "add" },
					{ text: _("Import"), value: "import" }
				],
				listeners: {
					itemclick: function(item, e) {
						this.cbAddBtnHdl(item.value);
					},
					scope: this
				}
			})
		}));
		return tbar;
	},

	cbAddBtnHdl : function(action) {
		var cls;
		switch (action) {
		case "add":
			cls = OMV.Module.Privileges.UserPropertyDialog;
			break;
		case "import":
			cls = OMV.Module.Privileges.UserImportDialog;
			break;
		}
		var wnd = new cls({
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
		var wnd = new OMV.Module.Privileges.UserPropertyDialog({
			rpcGetMethod: "getUser",
			rpcGetParams: { "name": record.get("name") },
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "UserMgmt",
		  "deleteUser", { "name": record.get("name") });
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "users", {
	cls: OMV.Module.Privileges.UserGridPanel,
	title: _("User"),
	position: 10
});

/**
 * @class OMV.Module.Privileges.UserSettings
 * @derived OMV.FormPanelExt
 */
OMV.Module.Privileges.UserSettings = function(config) {
	var initialConfig = {
		rpcService: "UserMgmt",
		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings",
		onlySubmitIfDirty: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.UserSettings.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.UserSettings, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Privileges.UserSettings.superclass.initComponent.apply(
		  this, arguments);
		this.on("load", this._updateFormFields, this);
	},

	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("User home directory"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1,
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				hiddenName: "sharedfolderref",
				fieldLabel: _("Location"),
				allowNone: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The location of the user home directories.")
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("enable");
		var checked = field.checked;
		var fields = [ "sharedfolderref" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "users", {
	cls: OMV.Module.Privileges.UserSettings,
	title: _("Settings"),
	position: 20
});

/**
 * @class OMV.Module.Privileges.UserPropertyDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Privileges.UserPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "UserMgmt",
		rpcSetMethod: "setUser",
		title: (!Ext.isDefined(config.rpcGetMethod)) ?
		  _("Add user") : _("Edit user"),
		width: 420,
		height: 385
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.UserPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.UserPropertyDialog, OMV.FormPanelDialog, {
	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "username",
			readOnly: Ext.isDefined(this.rpcGetMethod)
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			maxLength: 65,
			vtype: "comment"
		},{
			xtype: "textfield",
			name: "email",
			fieldLabel: _("Email"),
			allowBlank: true,
			vtype: "email"
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: _("Password"),
			allowBlank: Ext.isDefined(this.rpcGetMethod)
		},{
			xtype: "passwordfield",
			name: "passwordconf",
			fieldLabel: _("Confirm password"),
			allowBlank: Ext.isDefined(this.rpcGetMethod),
			submitValue: false
		},{
			xtype: "combo",
			name: "shell",
			hiddenName: "shell",
			fieldLabel: _("Shell"),
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "System",
					"method": "getShells",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "path",
					totalProperty: "total",
					root: "data",
					fields: [
						{ name: "path" }
					]
				}),
				sortInfo: {
					field: "path",
					direction: "ASC"
				}
			}),
			emptyText: _("Select a shell ..."),
			valueField: "path",
			displayField: "path",
			value: "/bin/dash"
		},{
			xtype: "checkboxgrid",
			name: "groups",
			fieldLabel: _("Groups"),
			height: 130,
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
				}),
				sortInfo: {
					field: "name",
					direction: "ASC"
				}
			}),
			valueField: "name",
			stateId: "c4689ae7-c8e6-4f3d-923f-6795fdf0ba45",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: _("Name"),
					sortable: true,
					dataIndex: "name",
					id: "name"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			listeners: {
				beforerender: function() {
					this.store.load();
				}
			}
		},{
			xtype: "checkbox",
			name: "disallowusermod",
			fieldLabel: _("Modify account"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Disallow the user to modify his account.")
		}];
	},

	isValid : function() {
		var valid = OMV.Module.Privileges.UserPropertyDialog.superclass.
		  isValid.call(this);
		if (valid) {
			var values = this.getValues();
			// Check the password
			var field = this.findFormField("passwordconf");
			if (values.password !== field.getValue()) {
				var msg = _("Passwords don't match");
				this.markInvalid([
					{ id: "password", msg: msg },
					{ id: "passwordconf", msg: msg }
				]);
				valid = false;
			}
		}
		return valid;
	},

	getValues : function() {
		var values = OMV.Module.Privileges.UserPropertyDialog.superclass.
		  getValues.call(this);
		// Convert 'groups' into an array
		values.groups = !Ext.isEmpty(values.groups) ?
		  values.groups.split(",") : [];
		return values;
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form object.
	 */
	setValues : function(values) {
		// Duplicate 'password'
		values.passwordconf = values.password;
		// Update various form fields, e.g. reset validation
		this._updateFormFields();
		// Set the form field values
		return OMV.Module.Privileges.UserPropertyDialog.superclass.
		  setValues.call(this, values);
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		if (this.uuid == OMV.UUID_UNDEFINED) {
			return;
		}
		var fields = [ "useuid", "uid" ];
		for (var i = 0; i < fields.length; i++) {
			var field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = true;
				field.setReadOnly(true);
				if ("numberfield" === field.getXType()) {
					// Reset validation
					field.minValue = Number.NEGATIVE_INFINITY;
				}
			}
		}
	}
});

/**
 * @class OMV.Module.Privileges.UserImportDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Privileges.UserImportDialog = function(config) {
	var initialConfig = {
		rpcService: "UserMgmt",
		rpcSetMethod: "importUsers",
		title: _("Import users"),
		width: 580,
		height: 350
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.UserImportDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.UserImportDialog, OMV.FormPanelDialog, {
	getFormItems : function() {
		return [{
			xtype: "textarea",
			name: "csv",
			hideLabel: true,
			allowBlank: false,
			autoCreate: {
				tag: "textarea",
				autocomplete: "off",
				rows: "8"
			},
			value: "# <name>;<uid>;<comment>;<email>;<password>;<group,group,...>;<disallowusermod>",
			anchor: "100% -15",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("Each line represents one user. Note, the password must be entered in plain text.")
		}];
	}
});

/**
 * @class OMV.Module.Privileges.GroupGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Privileges.GroupGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "d7c66fd9-2ef5-4107-9a6f-562dcdc2643a",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
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
	OMV.Module.Privileges.GroupGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.GroupGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "UserMgmt",
				"method": "getGroupList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "name",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "name" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Privileges.GroupGridPanel.superclass.initComponent.apply(
			this, arguments);
		// Add event handler
		this.on("activate", this.doReload, this);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Privileges.GroupPropertyDialog({
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
		var wnd = new OMV.Module.Privileges.GroupPropertyDialog({
			rpcGetMethod: "getGroup",
			rpcGetParams: { "name": record.get("name") },
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "UserMgmt",
		  "deleteGroup", { "name": record.get("name") });
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "groups", {
	cls: OMV.Module.Privileges.GroupGridPanel
});

/**
 * @class OMV.Module.Privileges.GroupPropertyDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Privileges.GroupPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "UserMgmt",
		rpcSetMethod: "setGroup",
		title: (!Ext.isDefined(config.rpcGetMethod)) ?
		  _("Add group") : _("Edit group"),
		height: 305
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.GroupPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.GroupPropertyDialog, OMV.FormPanelDialog, {
	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "groupname",
			readOnly: Ext.isDefined(this.rpcGetMethod)
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			vtype: "comment"
		},{
			xtype: "checkboxgrid",
			name: "members",
			fieldLabel: _("Members"),
			height: 130,
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "UserMgmt",
					"method": "enumerateUsers",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "name",
					fields: [
						{ name: "name" }
					]
				}),
				sortInfo: {
					field: "name",
					direction: "ASC"
				}
			}),
			valueField: "name",
			stateId: "c4689ae7-c8e6-4f3d-923f-6795fdf0ba45",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: _("Name"),
					sortable: true,
					dataIndex: "name",
					id: "name"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			listeners: {
				beforerender: function() {
					this.store.load();
				}
			}
		}];
	},

	getValues : function() {
		var values = OMV.Module.Privileges.GroupPropertyDialog.superclass.
		  getValues.call(this);
		// Convert 'members' into an array
		values.members = !Ext.isEmpty(values.members) ?
		  values.members.split(",") : [];
		return values;
	}
});
