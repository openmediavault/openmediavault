/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/CheckboxGrid.js")
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/form/SharedFolderComboBox.js")

Ext.ns("OMV.Module.Privileges");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("privileges", "users", {
	text: "User",
	icon: "images/user.png",
	position: 10
});
OMV.NavigationPanelMgr.registerMenu("privileges", "groups", {
	text: "Group",
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
		iconCls: "x-tab-strip-usermngmt-user",
		stateId: "98d6fe31-8e12-407b-82f2-7e0acf4006c1",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: "Email",
				sortable: true,
				dataIndex: "email",
				id: "email"
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			},{
				header: "Groups",
				sortable: true,
				dataIndex: "groups",
				id: "groups"
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
			proxy: new OMV.data.DataProxy("UserMgmt", "getUserList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "name" },
					{ name: "email" },
					{ name: "groups" },
					{ name: "comment" },
					{ name: "_readOnly" },
					{ name: "_used" }
    			]
			})
		});
		OMV.Module.Privileges.UserGridPanel.superclass.initComponent.apply(
		  this, arguments);
		this.on("activate", this.doReload, this);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Privileges.UserPropertyDialog({
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
		var wnd = new OMV.Module.Privileges.UserPropertyDialog({
			uuid: record.get("uuid"),
			readOnly: record.get("_readOnly"),
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
		  "deleteUser", [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "users", {
	cls: OMV.Module.Privileges.UserGridPanel
});

/**
 * @class OMV.Module.Privileges.UserPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Privileges.UserPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "UserMgmt",
		rpcGetMethod: "getUser",
		rpcSetMethod: "setUser",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " user",
		width: 420,
		height: 500
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.UserPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.UserPropertyDialog, OMV.CfgObjectDialog, {
	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: "Name",
			allowBlank: false,
			vtype: "username",
			readOnly: (this.uuid !== OMV.UUID_UNDEFINED)
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: "Comment",
			maxLength: 65,
			vtype: "comment"
		},{
			xtype: "textfield",
			name: "email",
			fieldLabel: "Email",
			allowBlank: true,
			vtype: "email"
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: "Password",
			allowBlank: false
		},{
			xtype: "passwordfield",
			name: "passwordconf",
			fieldLabel: "Confirm password",
			allowBlank: false,
			submitValue: false
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			hiddenName: "sharedfolderref",
			fieldLabel: "Home directory",
			allowBlank: true,
			allowNone: true,
			noneText: "Default",
			value: ""
		},{
			xtype: "combo",
			name: "shell",
			hiddenName: "shell",
			fieldLabel: "Shell",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy("System", "getShells",
				  null, false),
				reader: new Ext.data.JsonReader({
					idProperty: "path",
					totalProperty: "total",
					root: "data",
					fields: [
						{ name: "path" }
					]
				})
			}),
			emptyText: "Select a shell ...",
			valueField: "path",
			displayField: "path",
			value: "/bin/dash",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The name of the users login shell. Note, you may lock out a user from various services when selecting no login shell."
		},{
			xtype: "checkbox",
			name: "useuid",
			fieldLabel: "Specify user ID manually",
			inputValue: 1,
			listeners: {
				check: function(checkbox, checked) {
					if (this.uuid == OMV.UUID_UNDEFINED) {
						var field = this.findFormField("uid");
						if (!Ext.isEmpty(field)) {
							field.allowBlank = !checked;
							field.setReadOnly(!checked);
						}
					}
				},
				scope: this
			}
		},{
			xtype: "numberfield",
			name: "uid",
			fieldLabel: "UID",
			allowBlank: true,
			minValue: 1000,
			maxValue: 60000, // see /etc/login.defs
			allowDecimals: false,
			allowNegative: false,
			listeners: {
				beforerender: function(comp) {
					comp.setReadOnly(true);
				},
				scope: this
			}
		},{
			xtype: "checkboxgrid",
			name: "groups",
			fieldLabel: "Groups",
			height: 125,
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy("UserMgmt",
					"getGroupList"),
				reader: new Ext.data.JsonReader({
					idProperty: "uuid",
					totalProperty: "total",
					root: "data",
					fields: [
						{ name: "uuid" },
						{ name: "name" },
						{ name: "comment" }
					]
				})
			}),
			valueField: "name",
			stateId: "c4689ae7-c8e6-4f3d-923f-6795fdf0ba45",
			colModel: new Ext.grid.ColumnModel({
				defaults: {
					sortable: true
				},
				columns: [{
					header: "Group",
					sortable: true,
					dataIndex: "name",
					id: "name"
				},{
					header: "Comment",
					sortable: true,
					dataIndex: "comment",
					id: "comment"
				}]
			}),
			viewConfig: {
				forceFit: true
			},
			frame: true,
			listeners: {
				beforerender: function() {
					this.store.load();
				}
			}
		},{
			xtype: "checkbox",
			name: "disallowusermod",
			fieldLabel: "Modify account",
			checked: false,
			inputValue: 1,
			boxLabel: "Disallow the user to modify his account."
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
				var msg = "Passwords don't match";
				this.markInvalid([
					{ id: "password", msg: msg },
					{ id: "passwordconf", msg: msg }
				]);
				valid = false;
			}
		}
		return valid;
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
 * @class OMV.Module.Privileges.GroupGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Privileges.GroupGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		iconCls: "x-tab-strip-usermngmt-group",
		stateId: "d7c66fd9-2ef5-4107-9a6f-562dcdc2643a",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: "Comment",
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
			proxy: new OMV.data.DataProxy("UserMgmt", "getGroupList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "name" },
					{ name: "comment" },
					{ name: "_readOnly" },
					{ name: "_used" }
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
		var wnd = new OMV.Module.Privileges.GroupPropertyDialog({
			uuid: record.get("uuid"),
			readOnly: record.get("_readOnly"),
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
		  "deleteGroup", [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "groups", {
	cls: OMV.Module.Privileges.GroupGridPanel
});

/**
 * @class OMV.Module.Privileges.GroupPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Privileges.GroupPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "UserMgmt",
		rpcGetMethod: "getGroup",
		rpcSetMethod: "setGroup",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " group",
		height: 243
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.GroupPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.GroupPropertyDialog, OMV.CfgObjectDialog, {
	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: "Name",
			allowBlank: false,
			vtype: "groupname",
			readOnly: (this.uuid !== OMV.UUID_UNDEFINED)
		},{
			xtype: "checkbox",
			name: "usegid",
			fieldLabel: "Specify group ID manually",
			readOnly: (this.uuid !== OMV.UUID_UNDEFINED),
			inputValue: 1,
			listeners: {
				check: function(checkbox, checked) {
					if (this.uuid == OMV.UUID_UNDEFINED) {
						var field = this.findFormField("gid");
						if (!Ext.isEmpty(field)) {
							field.allowBlank = !checked;
							field.setReadOnly(!checked);
						}
					}
				},
				scope: this
			}
		},{
			xtype: "numberfield",
			name: "gid",
			fieldLabel: "GID",
			allowBlank: true,
			minValue: 1001,
			maxValue: 60000, // see /etc/login.defs
			allowDecimals: false,
			allowNegative: false,
			listeners: {
				beforerender: function(comp) {
					comp.setReadOnly(true);
				},
				scope: this
			}
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true,
			vtype: "comment"
		}];
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form object.
	 */
	setValues : function(values) {
		this._updateFormFields();
		OMV.Module.Privileges.GroupPropertyDialog.superclass.setValues.call(
		  this, values);
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		if (this.uuid == OMV.UUID_UNDEFINED)
			return;
		var fields = [ "usegid", "gid" ];
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
