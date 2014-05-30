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
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/field/SharedFolderComboBox.js")
// require("js/omv/window/Execute.js")

/**
 * @class OMV.module.admin.service.smb.Share
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.smb.Share", {
	extend: "OMV.workspace.window.Form",
	requires: [
	    "OMV.workspace.window.plugin.ConfigObject",
		"OMV.form.field.SharedFolderComboBox",
		"OMV.window.Execute"
	],

	rpcService: "SMB",
	rpcGetMethod: "getShare",
	rpcSetMethod: "setShare",
	plugins: [{
		ptype: "configobject"
	}],
	width: 700,
	height: 400,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormConfig: function() {
		return {
			plugins: [{
				ptype: "linkedfields",
				correlations: [{
					name: "recyclenow",
					conditions: [
						{ name: "recyclebin", value: true }
					],
					properties: "enabled"
				},{
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

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: true
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
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
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			vtype: "comment",
			plugins: [{
				ptype: "fieldinfo",
				text: _("This is a text field that is seen next to a share when a client queries the server.")
			}]
		},{
			xtype: "combo",
			name: "guest",
			fieldLabel: _("Public"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "no", _("No") ],
					[ "allow", _("Guests allowed") ],
					[ "only", _("Only guests") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "no",
			plugins: [{
				ptype: "fieldinfo",
				text: _("If 'Guests allowed' is selected and no login credential is provided, then access as guest. Always access as guest when 'Only guests' is selecting; in this case no password is required to connect to the share.")
			}]
		},{
			xtype: "checkbox",
			name: "readonly",
			fieldLabel: _("Read only"),
			checked: false,
			boxLabel: _("Set read only"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("If this parameter is set, then users may not create or modify files in the share.")
			}]
		},{
			xtype: "checkbox",
			name: "browseable",
			fieldLabel: _("Browseable"),
			checked: true,
			boxLabel: _("Set browseable"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("This controls whether this share is seen in the list of available shares in a net view and in the browse list.")
			}]
		},{
			xtype: "checkbox",
			name: "inheritacls",
			fieldLabel: _("Inherit ACLs"),
			checked: true,
			boxLabel: _("Honor existing ACLs"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("This parameter can be used to ensure that if default acls exist on parent directories, they are always honored when creating a new file or subdirectory in these parent directories.")
			}]
		},{
			xtype: "checkbox",
			name: "inheritpermissions",
			fieldLabel: _("Inherit permissions"),
			checked: false,
			boxLabel: _("Enable permission inheritance"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The permissions on new files and directories are normally governed by create mask and directory mask but the inherit permissions parameter overrides this. This can be particularly useful on systems with many users to allow a single share to be used flexibly by each user.")
			}]
		},{
			xtype: "fieldcontainer",
			fieldLabel: _("Recycle bin"),
			layout: "anchor",
			fieldDefaults: {
				readOnly: me.readOnly,
				anchor: "100%"
			},
			items: [{
				xtype: "checkbox",
				name: "recyclebin",
				checked: false,
				hideLabel: true,
				boxLabel: _("Enable recycle bin"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("This will create a recycle bin on the share.")
				}]
			},{
				xtype: "numberfield",
				name: "recyclemaxsize",
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				hideLabel: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Files that are larger than the specified number of bytes will not be put into the recycle bin. Set to 0 for unrestricted file size."),
				}],
				value: 0
			},{
				xtype: "numberfield",
				name: "recyclemaxage",
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				hideLabel: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Files in the recycle bin will be deleted automatically after the specified number of days. Set to 0 for manual deletion."),
				}],
				value: 0
			},{
				xtype: "button",
				name: "recyclenow",
				disabled: true,
				text: _("Empty now"),
				scope: me,
				handler: function() {
					// Empty recycle bin.
					Ext.create("OMV.window.Execute", {
						title: _("Empty recycle bin"),
						rpcService: "SMB",
						rpcMethod: "emptyRecycleBin",
						rpcParams: {
							uuid: me.uuid
						},
						listeners: {
							scope: me,
							finish: function(wnd, response) {
								wnd.appendValue(_("Done ..."));
							},
							exception: function(wnd, error) {
								OMV.MessageBox.error(null, error);
							}
						}
					}).show();
				}
			}]
		},{
			xtype: "checkbox",
			name: "hidedotfiles",
			fieldLabel: _("Hide dot files"),
			checked: true,
			boxLabel: _("This parameter controls whether files starting with a dot appear as hidden files")
		},{
			xtype: "checkbox",
			name: "easupport",
			fieldLabel: _("Extended attributes"),
			checked: false,
			boxLabel: _("Enable extended attribute support"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("Allow clients to attempt to store OS/2 style extended attributes on a share.")
			}]
		},{
			xtype: "checkbox",
			name: "storedosattributes",
			fieldLabel: _("Store DOS attributes"),
			checked: false,
			boxLabel: _("Enable store DOS attributes support"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("If this parameter is set, Samba attempts to first read DOS attributes (SYSTEM, HIDDEN, ARCHIVE or READ-ONLY) from a file system extended attribute, before mapping DOS attributes to UNIX permission bits. When set, DOS attributes will be stored onto an extended attribute in the UNIX file system, associated with the file or directory.")
			}]
		},{
			xtype: "textfield",
			name: "hostsallow",
			fieldLabel: _("Hosts allow"),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("This option is a comma, space, or tab delimited set of hosts which are permitted to access this share. You can specify the hosts by name or IP number. Leave this field empty to use default settings.")
			}]
		},{
			xtype: "textfield",
			name: "hostsdeny",
			fieldLabel: _("Hosts deny"),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("This option is a comma, space, or tab delimited set of host which are NOT permitted to access this share. Where the lists conflict, the allow list takes precedence. In the event that it is necessary to deny all by default, use the keyword ALL (or the netmask 0.0.0.0/0) and then explicitly specify to the hosts allow parameter those hosts that should be permitted access. Leave this field empty to use default settings.")
			}]
		},{
			xtype: "checkbox",
			name: "audit",
			fieldLabel: _("Audit"),
			checked: false,
			boxLabel: _("Audit file operations.")
		},{
			xtype: "textarea",
			name: "extraoptions",
			fieldLabel: _("Extra options"),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Please check the <a href='http://www.samba.org/samba/docs/man/manpages-3/smb.conf.5.html' target='_blank'>manual page</a> for more details.")
			}]
		}];
	}
});

/**
 * @class OMV.module.admin.service.smb.Shares
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.smb.Shares", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.smb.Share"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "e0bbeb12-8441-4b70-899e-800e2dab53ee",
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
	},{
		xtype: "mapcolumn",
		text: _("Public"),
		sortable: true,
		dataIndex: "guest",
		stateId: "guest",
		mapItems: {
			"no": _("No"),
			"allow": _("Guest allowed"),
			"only": _("Only guests")
		}
	},{
		xtype: "booleantextcolumn",
		text: _("Read only"),
		sortable: true,
		dataIndex: "readonly",
		stateId: "readonly"
	},{
		xtype: "booleantextcolumn",
		text: _("Browseable"),
		sortable: true,
		dataIndex: "browseable",
		stateId: "browseable"
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
						{ name: "sharedfoldername", type: "string" },
						{ name: "name", type: "string" },
						{ name: "comment", type: "string" },
						{ name: "browseable", type: "boolean" },
						{ name: "readonly", type: "boolean" },
						{ name: "guest", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "SMB",
						method: "getShareList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "sharedfoldername"
				}]
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.smb.Share", {
			title: _("Add share"),
			uuid: OMV.UUID_UNDEFINED,
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
		Ext.create("OMV.module.admin.service.smb.Share", {
			title: _("Edit share"),
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
				service: "SMB",
				method: "deleteShare",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "shares",
	path: "/service/smb",
	text: _("Shares"),
	position: 20,
	className: "OMV.module.admin.service.smb.Shares"
});
