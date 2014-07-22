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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/form/field/CheckboxGrid.js")
// require("js/omv/form/field/Password.js")
// require("js/omv/grid/PrivilegesByRole.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/data/reader/RpcArray.js")

/**
 * @class OMV.module.admin.privilege.user.User
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.privilege.user.User", {
	extend: "OMV.workspace.window.Form",
	uses: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.data.reader.RpcArray",
		"OMV.form.field.CheckboxGrid",
		"OMV.form.field.Password"
	],

	rpcService: "UserMgmt",
	rpcSetMethod: "setUser",
	width: 420,

	getFormConfig: function() {
		return {
			layout: {
				type: "vbox",
				align: "stretch"
			}
		};
	},

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "username",
			readOnly: Ext.isDefined(me.rpcGetMethod)
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
			allowBlank: Ext.isDefined(me.rpcGetMethod)
		},{
			xtype: "passwordfield",
			name: "passwordconf",
			fieldLabel: _("Confirm password"),
			allowBlank: Ext.isDefined(me.rpcGetMethod),
			submitValue: false
		},{
			xtype: "combo",
			name: "shell",
			fieldLabel: _("Shell"),
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				fields: [
					{ name: "path", type: "string" }
				],
				proxy: {
					type: "rpc",
					reader: "rpcarray",
					appendSortParams: false,
					rpcData: {
						service: "System",
						method: "getShells"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "path"
				}]
			}),
			emptyText: _("Select a shell ..."),
			valueField: "path",
			displayField: "path",
			value: "/bin/dash"
		},{
			xtype: "checkboxgridfield",
			name: "groups",
			fieldLabel: _("Groups"),
			valueField: "name",
			flex: 1,
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "name",
					fields: [
						{ name: "name", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "UserMgmt",
						method: "enumerateAllGroups"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "name"
				}]
			}),
			gridConfig: {
				stateful: true,
				stateId: "e44f12ea-b1ed-11e2-9a57-00221568ca88",
				columns: [{
					text: _("Name"),
					sortable: true,
					dataIndex: "name",
					stateId: "name",
					flex: 1
				}]
			},
			value: [ "users" ]
		},{
			xtype: "checkbox",
			name: "disallowusermod",
			fieldLabel: _("Modify account"),
			checked: false,
			boxLabel: _("Disallow the user to modify his account.")
		}];
	},

	isValid: function() {
		var me = this;
		if (!me.callParent(arguments))
			return false;
		var valid = true;
		var values = me.getValues();
		// Check the password.
		var field = me.findField("passwordconf");
		if (values.password !== field.getValue()) {
			var msg = _("Passwords don't match");
			me.markInvalid([
				{ id: "password", msg: msg },
				{ id: "passwordconf", msg: msg }
			]);
			valid = false;
		}
		return valid;
	}
});

/**
 * @class OMV.module.admin.privilege.user.Import
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.privilege.user.Import", {
	extend: "OMV.workspace.window.Form",

	title: _("Import users"),
	width: 580,
	height: 350,

	rpcService: "UserMgmt",
	rpcSetMethod: "importUsers",
	rpcSetPollStatus: true,
	autoLoadData: false,
	submitMsg: _("Importing users ..."),
	okButtonText: _("Import"),

	getFormConfig: function() {
		return {
			layout: "fit",
			bodyPadding: "",
			dockedItems: [{
				xtype: "tiptoolbar",
				dock: "bottom",
				ui: "footer",
				text: _("Each line represents one user. Note, the password must be entered in plain text.")
			}]
		};
	},

	getFormItems: function() {
		return {
			xtype: "textarea",
			name: "csv",
			hideLabel: true,
			allowBlank: false,
			value: "# <name>;<uid>;<comment>;<email>;<password>;<group,group,...>;<disallowusermod>",
			fieldStyle: {
				border: "0px"
			}
		};
	}
});

/**
 * @class OMV.module.admin.privilege.user.SharedFolderPrivileges
 * @derived OMV.workspace.window.Form
 * Display all shared folder privileges from the given user.
 */
Ext.define("OMV.module.admin.privilege.user.SharedFolderPrivileges", {
	extend: "OMV.workspace.window.Grid",
	requires: [
		"OMV.grid.PrivilegesByRole"
	],

	rpcService: "ShareMgmt",
	rpcSetMethod: "setPrivilegesByRole",

	title: _("Shared folder privileges"),
	width: 550,
	height: 350,
	hideResetButton: true,
	gridClassName: "OMV.grid.PrivilegesByRole",

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param roleName The name of the user. Required.
	 */

	getGridConfig: function() {
		var me = this;
		return {
			border: false,
			stateful: true,
			stateId: "41e79486-1192-11e4-baab-0002b3a176b4",
			roleType: "user",
			roleName: me.roleName
		};
	},

	getRpcSetParams: function() {
		var me = this;
		var privileges = [];
		var items = me.getValues();
		Ext.Array.each(items, function(item) {
			// Set default values.
			var privilege = {
				uuid: item.uuid,
				perms: -1
			};
			if ((true === item.deny) || (true === item.readonly) ||
			  (true === item.writeable)) {
				privilege.perms = 0; // No access
				if (true === item.readonly)
					privilege.perms = 5;
				else if (true === item.writeable)
					privilege.perms = 7;
			}
			Ext.Array.push(privileges, privilege);
		});
		return {
			role: "user",
			name: me.roleName,
			privileges: privileges
		};
	}
});

/**
 * @class OMV.module.admin.privilege.user.Users
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.privilege.user.Users", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format"
	],
	uses: [
		"OMV.module.admin.privilege.user.User",
		"OMV.module.admin.privilege.user.Import",
		"OMV.module.admin.privilege.user.SharedFolderPrivileges"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "98d6fe31-8e12-407b-82f2-7e0acf4006c1",
	columns: [{
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name"
	},{
		text: _("Email"),
		sortable: true,
		dataIndex: "email",
		stateId: "email"
	},{
		text: _("Comment"),
		sortable: true,
		dataIndex: "comment",
		stateId: "comment"
	},{
		text: _("Groups"),
		dataIndex: "groups",
		stateId: "groups",
		renderer: function(value) {
			if(Ext.isArray(value))
				value = value.join(", ");
			return OMV.util.Format.whitespace(value);
		}
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "name",
					fields: [
						{ name: "name", type: "string" },
						{ name: "email", type: "string" },
						{ name: "groups", type: "object" },
						{ name: "comment", type: "string" },
						{ name: "system", type: "boolean" },
						{ name: "_used", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "UserMgmt",
						method: "getUserList"
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

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		// Replace the default 'Add' button.
		Ext.Array.erase(items, 0, 1);
		Ext.Array.insert(items, 0, [{
			id: me.getId() + "-add",
			xtype: "splitbutton",
			text: _("Add"),
			icon: "images/add.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: function() {
				this.showMenu();
			},
			menu: Ext.create("Ext.menu.Menu", {
				items: [
					{ text: _("Add"), value: "add" },
					{ text: _("Import"), value: "import" }
				],
				listeners: {
					scope: me,
					click: function(menu, item, e, eOpts) {
						this.onAddButton(item.value);
					}
				}
			})
		}]);
		// Add the 'Privileges' button.
		Ext.Array.insert(items, 2, [{
			id: me.getId() + "-privileges",
			xtype: "button",
			text: _("Privileges"),
			icon: "images/group.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: me.onPrivilegesButton,
			scope: me,
			disabled: true,
			selectionChangeConfig: {
				minSelection: 1,
				maxSelection: 1
			}
		}]);
		return items;
	},

	onAddButton: function(action) {
		var me = this;
		switch(action) {
		case "add":
			Ext.create("OMV.module.admin.privilege.user.User", {
				title: _("Add user"),
				listeners: {
					scope: me,
					submit: function() {
						this.doReload();
					}
				}
			}).show();
			break;
		case "import":
			Ext.create("OMV.module.admin.privilege.user.Import", {
				type: "user",
				listeners: {
					scope: me,
					finish: function() {
						this.doReload();
					}
				}
			}).show();
			break;
		}
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.privilege.user.User", {
			title: _("Edit user"),
			rpcGetMethod: "getUser",
			rpcGetParams: {
				name: record.get("name")
			},
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onPrivilegesButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.privilege.user.SharedFolderPrivileges", {
			roleName: record.get("name")
		}).show();
	},

	doDeletion: function(record) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "UserMgmt",
				method: "deleteUser",
				params: {
					name: record.get("name")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerNode({
	id: "user",
	path: "/privilege",
	text: _("User"),
	icon16: "images/user.png",
	iconSvg: "images/user.svg",
	position: 10
});

OMV.WorkspaceManager.registerPanel({
	id: "users",
	path: "/privilege/user",
	text: _("Users"),
	position: 10,
	className: "OMV.module.admin.privilege.user.Users"
});
