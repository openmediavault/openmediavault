/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/Panel.js")
// require("js/omv/form/field/Password.js")
// require("js/omv/form/field/GroupComboBox.js")
// require("js/omv/form/field/UserComboBox.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.rsyncd.module.General
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.rsyncd.module.General", {
	extend: "OMV.form.Panel",
	requires: [
		"OMV.form.field.SharedFolderComboBox",
		"OMV.form.field.UserComboBox",
		"OMV.form.field.GroupComboBox"
	],

	title: _("General"),

	getFormConfig: function() {
		return {
			plugins: [{
				ptype: "linkedfields",
				correlations: [{
					// Automatically set the share name, except it has
					// already been entered.
					name: "name",
					conditions: [
						{ name: "sharedfolderref", op: "n" },
						{ name: "name", op: "z" }
					],
					properties: function(valid, field) {
						if (!valid)
							return;
						var record = this.findField("sharedfolderref").
						  getRecord();
						field.setValue(record.get("name"));
					}
				}]
			}]
		}
	},

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: true
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				readOnly: (me.uuid !== OMV.UUID_UNDEFINED),
				fieldLabel: _("Shared folder"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("The location of the files to share.")
				}]
			},{
				xtype: "textfield",
				name: "name",
				fieldLabel: _("Name"),
				allowBlank: false,
				vtype: "sharename",
				plugins: [{
					ptype: "fieldinfo",
					text: _("The name of the share.")
				}]
			},{
				xtype: "usercombo",
				name: "uid",
				fieldLabel: _("User"),
				value: "nobody",
				plugins: [{
					ptype: "fieldinfo",
					text: _("This option specifies the user name that file transfers to and from that module should take place.")
				}]
			},{
				xtype: "groupcombo",
				name: "gid",
				fieldLabel: _("Group"),
				value: "users",
				plugins: [{
					ptype: "fieldinfo",
					text: _("This option specifies the group name that file transfers to and from that module should take place.")
				}]
			},{
				xtype: "checkbox",
				name: "usechroot",
				fieldLabel: _("Use chroot"),
				checked: true,
				boxLabel: _("Enable chroot."),
				plugins: [{
					ptype: "fieldinfo",
					text: _("If this option is set, the daemon will chroot to the shared folder path before starting the file transfer with the client. Then it is not possible to map users and groups by name and the daemon is not being able to follow symbolic links that are either absolute or outside of the new root path.")
				}]
			},{
				xtype: "checkbox",
				name: "authusers",
				fieldLabel: _("Authenticate users"),
				checked: false,
				boxLabel: _("Enable user authentication"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("If set then the client will be challenged to supply a username and password to connect to the module.")
				}]
			},{
				xtype: "checkbox",
				name: "readonly",
				fieldLabel: _("Read only"),
				checked: false,
				boxLabel: _("Set read only"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("If this option is set, then any attempted uploads will fail.")
				}]
			},{
				xtype: "checkbox",
				name: "writeonly",
				fieldLabel: _("Write only"),
				checked: false,
				boxLabel: _("Set write only"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("If this option is set, then any attempted downloads will fail.")
				}]
			},{
				xtype: "checkbox",
				name: "list",
				fieldLabel: _("List"),
				checked: true,
				boxLabel: _("Enable module listing"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("This option determines if this module should be listed when the client asks for a listing of available modules.")
				}]
			},{
				xtype: "numberfield",
				name: "maxconnections",
				fieldLabel: _("Max. connections"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 0,
				plugins: [{
					ptype: "fieldinfo",
					text: _("This option specifies the maximum number of simultaneous connections. 0 means no limit.")
				}]
			},{
				xtype: "textfield",
				name: "hostsallow",
				fieldLabel: _("Hosts allow"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("This option is a comma, space, or tab delimited set of hosts which are permitted to access this module. You can specify the hosts by name or IP number. Leave this field empty to use default settings.")
				}]
			},{
				xtype: "textfield",
				name: "hostsdeny",
				fieldLabel: _("Hosts deny"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("This option is a comma, space, or tab delimited set of host which are NOT permitted to access this module. Where the lists conflict, the allow list takes precedence. In the event that it is necessary to deny all by default, use the keyword ALL (or the netmask 0.0.0.0/0) and then explicitly specify to the hosts allow parameter those hosts that should be permitted access. Leave this field empty to use default settings.")
				}]
			},{
				xtype: "textarea",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Please check the <a href='http://www.samba.org/ftp/rsync/rsyncd.conf.html' target='_blank'>manual page</a> for more details.")
				}]
			},{
				xtype: "textfield",
				name: "comment",
				fieldLabel: _("Comment"),
				allowBlank: true,
				vtype: "comment"
			}]
		});
		me.callParent(arguments);
	},

	isValid: function() {
		var me = this;
		if(!me.callParent(arguments))
			return false;
		var valid = true;
		var values = me.getValues();
		if(values.readonly && values.writeonly) {
			var msg = _("Fields must be exclusive");
			me.markInvalid([
				{ id: "readonly", msg: msg },
				{ id: "writeonly", msg: msg }
			]);
			valid = false;
		}
		return valid;
	}
});

/**
 * @class OMV.module.admin.service.rsyncd.module.AuthUser
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.rsyncd.module.AuthUser", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.form.field.UserComboBox",
		"OMV.form.field.Password"
	],

	mode: "local",
	hideResetButton: true,

	getFormItems: function() {
		return [{
			xtype: "usercombo",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			forceSelection: false,
			userType: "normal",
			emptyText: _("Select or enter a user name ...")
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: _("Password"),
			allowBlank: false
		}];
	}
});

/**
 * @class OMV.module.admin.service.rsyncd.module.AuthUsers
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.rsyncd.module.AuthUsers", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.rsyncd.module.AuthUser"
	],

	title: _("Users"),
	mode: "local",
	stateful: true,
	stateId: "42a21c5a-3dc5-11e1-9f73-af7c01c6821f",
	columns: [{
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name"
	},{
		text: _("Password"),
		sortable: false,
		dataIndex: "password",
		stateId: "password",
		renderer: function(value) {
			return value.replace(/./g, "*");
		}
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("Ext.data.JsonStore", {
				autoLoad: false,
				model: OMV.data.Model.createImplicit({
					idProperty: "name",
					fields: [
						{ name: "name", type: "string" },
						{ name: "password", type: "string" }
					]
				}),
				data: [],
				sorters: [{
					property: "name",
					direction: "ASC"
				}]
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.rsyncd.module.AuthUser", {
			title: _("Add user"),
			listeners: {
				scope: me,
				submit: function(wnd, values) {
					// Check if user already exists.
					if(this.store.findExact("name", values.name) != -1) {
						OMV.MessageBox.failure(null, "User already exists.");
						return;
					}
					this.store.addData(values);
				}
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		var wnd = Ext.create("OMV.module.admin.service.rsyncd.module.AuthUser", {
			title: _("Edit user"),
			listeners: {
				scope: me,
				submit: function(wnd, values) {
					// Update the selected record.
					record.beginEdit();
					record.set(values);
					record.endEdit();
				}
			}
		});
		wnd.findField("name").setReadOnly(true);
		wnd.setValues(record.getData());
		wnd.show();
	},

	setValues: function(values) {
		return this.callParent([ values.users ]);
	},

	getValues: function() {
		var values = this.callParent(arguments);
		return {
			users: values
		};
	}
});

/**
 * @class OMV.module.admin.service.rsyncd.Module
 * @derived OMV.workspace.window.Tab
 */
Ext.define("OMV.module.admin.service.rsyncd.Module", {
	extend: "OMV.workspace.window.Tab",
	requires: [
		"OMV.module.admin.service.rsyncd.module.General",
		"OMV.module.admin.service.rsyncd.module.AuthUsers",
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "Rsyncd",
	rpcGetMethod: "getModule",
	rpcSetMethod: "setModule",
	plugins: [{
		ptype: "configobject"
	}],
	width: 600,
	height: 450,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getTabItems: function() {
		var me = this;
		return [
			Ext.create("OMV.module.admin.service.rsyncd.module.General", {
				uuid: me.uuid,
				bodyPadding: "5 5 0"
			}),
			Ext.create("OMV.module.admin.service.rsyncd.module.AuthUsers")
		];
	},

	isValid: function() {
		var me = this;
		if(!me.callParent(arguments))
			return false;
		var valid = true;
		var values = me.getValues();
		if((values.authusers === true) && (values.users.length == 0)) {
			var tab = me.getTab().find("title", _("General"));
			var basicForm = tab[0].getForm();
			basicForm.markInvalid([
				{ id: "authusers", msg: _("No users defined") }
			]);
			me.markInvalid(tab[0]);
			valid = false;
		}
		return valid;
	}
});

/**
 * @class OMV.module.admin.service.rsyncd.Modules
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.rsyncd.Modules", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.rsyncd.Module"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "72d6ab93-f08d-4d34-820b-fcbb832f723c",
	columns: [{
		xtype: "booleaniconcolumn",
		text: _("Enabled"),
		sortable: true,
		dataIndex: "enable",
		stateId: "enable",
		align: "center",
		width: 80,
		resizable: false,
		trueIcon: "switch_on.png",
		falseIcon: "switch_off.png"
	},{
		text: _("Shared folder"),
		sortable: true,
		dataIndex: "sharedfoldername",
		stateId: "sharedfoldername"
	},{
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name"
	},{
		text: _("Comment"),
		sortable: true,
		dataIndex: "comment",
		stateId: "comment"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "uuid",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "enable", type: "boolean" },
						{ name: "name", type: "string" },
						{ name: "sharedfoldername", type: "string" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Rsyncd",
						method: "getModuleList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "name"
				}]
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.rsyncd.Module", {
			title: _("Add module"),
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onEditButton : function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.service.rsyncd.Module", {
			title: _("Edit module"),
			uuid: record.get("uuid"),
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
				service: "Rsyncd",
				method: "deleteModule",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});
