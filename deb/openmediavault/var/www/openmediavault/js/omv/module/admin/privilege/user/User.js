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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Grid.js")
// require("js/omv/workspace/window/Tab.js")
// require("js/omv/workspace/window/TextArea.js")
// require("js/omv/form/Panel.js")
// require("js/omv/form/field/CheckboxGrid.js")
// require("js/omv/form/field/Password.js")
// require("js/omv/grid/PrivilegesByRole.js")
// require("js/omv/grid/column/BooleanText.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/data/reader/RpcArray.js")
// require("js/omv/toolbar/Tip.js")

/**
 * @class OMV.module.admin.privilege.user.user.General
 * @derived OMV.form.Panel
 * @param editMode Set to TRUE if the dialog is in edit mode. Defaults
 *   to FALSE.
 */
Ext.define("OMV.module.admin.privilege.user.user.General", {
	extend: "OMV.form.Panel",
	uses: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.data.reader.RpcArray",
		"OMV.form.field.Password"
	],

	editMode: false,

	title: _("General"),
	bodyPadding: "5 5 0",

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			items: [{
				xtype: "textfield",
				name: "name",
				fieldLabel: _("Name"),
				allowBlank: false,
				vtype: "username",
				readOnly: me.editMode
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
				allowBlank: me.editMode,
				autoComplete: false
			},{
				xtype: "passwordfield",
				name: "passwordconf",
				fieldLabel: _("Confirm password"),
				allowBlank: me.editMode,
				autoComplete: false,
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
				value: "/bin/sh"
			},{
				xtype: "checkbox",
				name: "disallowusermod",
				fieldLabel: _("Modify account"),
				checked: false,
				boxLabel: _("Disallow the user to modify their account.")
			}]
		});
		me.callParent(arguments);
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
 * @class OMV.module.admin.privilege.user.user.Groups
 * @derived OMV.grid.Panel
 */
Ext.define("OMV.module.admin.privilege.user.user.Groups", {
	extend: "OMV.grid.Panel",
	uses: [
		"Ext.XTemplate",
		"Ext.grid.feature.Grouping",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.grid.column.BooleanText"
	],

	border: false,
	title: _("Groups"),
	selModel: "checkboxmodel",
	stateful: true,
	stateId: "e44f12ea-b1ed-11e2-9a57-00221568ca88",
	features: [{
		ftype: "grouping",
		groupHeaderTpl: Ext.create("Ext.XTemplate",
			"{[this.renderValue(values)]}", {
			renderValue: function(values) {
				var result;
				switch (values.groupField) {
				case "system":
					result = values.groupValue ? _("System accounts") :
					  _("User accounts");
					break;
				default:
					result = Ext.String.format("{0}: {1}", values.columnName,
					  values.name);
					break;
				}
				return result;
			}
		})
	}],
	columns: [{
		xtype: "textcolumn",
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name",
		flex: 2
	},{
		xtype: "textcolumn",
		text: _("GID"),
		sortable: true,
		dataIndex: "gid",
		stateId: "gid",
		hidden: true,
		flex: 1
	},{
		xtype: "booleantextcolumn",
		text: _("System"),
		sortable: true,
		groupable: true,
		hidden: true,
		dataIndex: "system",
		stateId: "system",
		align: "center",
		flex: 1
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				groupField: "system",
				model: OMV.data.Model.createImplicit({
					idProperty: "name",
					fields: [
						{ name: "name", type: "string" },
						{ name: "gid", type: "int" },
						{ name: "system", type: "boolean" }
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
				}],
				listeners: {
					scope: me,
					load: function(store, records, successful, operation, eOpts) {
						// Always select the 'users' group.
						var record = store.findExactRecord("name", "users");
						if (Ext.isObject(record) && record.isModel)
							me.getSelectionModel().select(record, true, true);
					}
				}
			})
		});
		me.callParent(arguments);
		// Mark the store as dirty whenever the selection has
		// been changed.
		me.on("selectionchange", function(c, selected) {
			me.getStore().markDirty();
		});
	},

	setValues: function(values) {
		var me = this;
		// Ensure the store is loaded to select the given groups.
		if (me.getStore().isLoading() || !me.getStore().isLoaded()) {
			var fn = Ext.Function.bind(me.setValues, me, arguments);
			me.getStore().on({
				single: true,
				load: fn
			});
			return false;
		}
		// Select the given groups.
		var records = [];
		me.getSelectionModel().deselectAll(true);
		Ext.Array.each(values.groups, function(name) {
			var record = me.getStore().findRecord("name", name);
			if (Ext.isObject(record) && record.isModel)
				Ext.Array.push(records, [ record ]);
		});
		me.getSelectionModel().select(records, true, true);
		return values.groups;
	},

	getValues: function() {
		var me = this;
		var groups = [];
		var records = me.getSelection();
		Ext.Array.each(records, function(record) {
			groups.push(record.get("name"));
		});
		return {
			groups: groups
		};
	}
});

/**
 * @class OMV.module.admin.privilege.user.user.sshpubkeys.PubKey
 * @derived OMV.workspace.window.TextArea
 */
Ext.define("OMV.module.admin.privilege.user.user.sshpubkeys.PubKey", {
	extend: "OMV.workspace.window.TextArea",
	uses: [
		"OMV.toolbar.Tip"
	],

	width: 500,
	height: 250,
	hideOkButton: false,
	mode: "local",
	textAreaConfig: {
		readOnly: false,
		allowBlank: false,
		vtype: "sshPubKey"
	},

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		// Add the tip toolbar at the bottom of the window.
		me.addDocked({
			xtype: "tiptoolbar",
			dock: "bottom",
			ui: "footer",
			text: _("The SSH public key in OpenSSH or RFC 4716 format.")
		});
	}
});

/**
 * @class OMV.module.admin.privilege.user.user.SshPubKeys
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.privilege.user.user.SshPubKeys", {
	extend: "OMV.workspace.grid.Panel",
	uses: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.reader.RpcArray",
		"OMV.module.admin.privilege.user.user.sshpubkeys.PubKey"
	],

	title: _("Public keys"),
	hideEditButton: true,
	hidePagingToolbar: true,
	mode: "local",
	columns: [{
		text: _("Public key"),
		dataIndex: "sshpubkey",
		sortable: false,
		flex: 1,
		renderer: function(value, metaData) {
			metaData.tdCls += " x-monospaced";
			return value.replace(/\n/g, "<br>");
		}
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: false,
				model: OMV.data.Model.createImplicit({
					idProperty: "sshpubkey",
					fields: [
						{ name: "sshpubkey", type: "string", mapping: 0 }
					]
				}),
				proxy: {
					type: "memory",
					reader: "rpcarray"
				},
				data: []
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.privilege.user.user.sshpubkeys.PubKey", {
			title: _("Add public key"),
			listeners: {
				scope: me,
				submit: function(c, value) {
					value = value.rtrim(" \n");
					me.getStore().addRawData([ value ]);
				}
			}
		}).show();
	},

	setValues: function(values) {
		var me = this;
		me.getStore().loadRawData(values.sshpubkeys);
		return values.sshpubkeys;
	},

	getValues: function() {
		var me = this;
		var sshpubkeys = [];
		me.getStore().each(function(record) {
			Ext.Array.push(sshpubkeys, record.get("sshpubkey"));
		});
		return {
			sshpubkeys: sshpubkeys
		};
	}
});

/**
 * @class OMV.module.admin.privilege.user.User
 * @derived OMV.workspace.window.Tab
 */
Ext.define("OMV.module.admin.privilege.user.User", {
	extend: "OMV.workspace.window.Tab",
	uses: [
		"OMV.module.admin.privilege.user.user.General",
		"OMV.module.admin.privilege.user.user.Groups",
		"OMV.module.admin.privilege.user.user.SshPubKeys"
	],

	rpcService: "UserMgmt",
	rpcSetMethod: "setUser",

	width: 420,
	height: 450,

	getTabItems: function() {
		var me = this;
		return [
			Ext.create("OMV.module.admin.privilege.user.user.General", {
				editMode: Ext.isDefined(me.rpcGetMethod)
			}),
			Ext.create("OMV.module.admin.privilege.user.user.Groups"),
			Ext.create("OMV.module.admin.privilege.user.user.SshPubKeys")
		];
	}
});

/**
 * @class OMV.module.admin.privilege.user.Import
 * @derived OMV.workspace.window.TextArea
 */
Ext.define("OMV.module.admin.privilege.user.Import", {
	extend: "OMV.workspace.window.TextArea",
	uses: [
		"OMV.toolbar.Tip"
	],

	title: _("Import users"),
	width: 580,
	height: 350,

	rpcService: "UserMgmt",
	rpcSetMethod: "importUsers",
	rpcSetPollStatus: true,
	autoLoadData: false,
	submitMsg: _("Importing users ..."),
	hideOkButton: false,
	okButtonText: _("Import"),
	readOnly: false,

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		// Add the tip toolbar at the bottom of the window.
		me.addDocked({
			xtype: "tiptoolbar",
			dock: "bottom",
			ui: "footer",
			text: _("Each line represents one user. Note, the password must be entered in plain text.")
		});
	},

	getTextAreaConfig: function() {
		return {
			allowBlank: false,
			value: "# <name>;<uid>;<comment>;<email>;<password>;<shell>;<group,group,...>;<disallowusermod>"
		};
	},

	getValues: function() {
		var me = this;
		var values = me.callParent(arguments);
		return {
			csv: values
		};
	},

	setValues: function(values) {
		var me = this;
		return me.setValues(values.cvs);
	}
});

/**
 * @class OMV.module.admin.privilege.user.SharedFolderPrivileges
 * @derived OMV.workspace.window.Grid
 * Display all shared folder privileges from the given user.
 */
Ext.define("OMV.module.admin.privilege.user.SharedFolderPrivileges", {
	extend: "OMV.workspace.window.Grid",
	requires: [
		"OMV.grid.PrivilegesByRole"
	],
	uses: [
		"OMV.toolbar.Tip"
	],

	rpcService: "ShareMgmt",
	rpcSetMethod: "setPrivilegesByRole",

	title: _("Shared folder privileges"),
	width: 500,
	height: 300,
	hideResetButton: true,
	gridClassName: "OMV.grid.PrivilegesByRole",

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param roleName The name of the user. Required.
	 */

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		// Add the tip toolbar at the bottom of the window.
		me.addDocked({
			xtype: "tiptoolbar",
			dock: "bottom",
			ui: "footer",
			text: _("These settings are used by the services to configure the user access rights. Please note that these settings have no effect on file system permissions.")
		});
	},

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
		xtype: "textcolumn",
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name"
	},{
		xtype: "textcolumn",
		text: _("Email"),
		sortable: true,
		dataIndex: "email",
		stateId: "email"
	},{
		xtype: "textcolumn",
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
			text: me.addButtonText,
			iconCls: me.addButtonIconCls,
			handler: function(c) {
				c.showMenu();
			},
			menu: Ext.create("Ext.menu.Menu", {
				items: [{
					iconCls: me.addButtonIconCls,
					text: me.addButtonText,
					value: "create"
				},{
					iconCls: "mdi mdi-import",
					text: _("Import"),
					value: "import"
				}],
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
			iconCls: "x-fa fa-share-alt",
			handler: me.onPrivilegesButton,
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1
			}
		}]);
		return items;
	},

	onAddButton: function(action) {
		var me = this;
		switch(action) {
		case "create":
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
					submit: function() {
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
	iconCls: "mdi mdi-account",
	position: 10
});

OMV.WorkspaceManager.registerPanel({
	id: "users",
	path: "/privilege/user",
	text: _("Users"),
	position: 10,
	className: "OMV.module.admin.privilege.user.Users"
});
