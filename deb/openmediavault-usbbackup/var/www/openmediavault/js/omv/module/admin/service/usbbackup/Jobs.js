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
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/window/Execute.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.usbbackup.Job
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.usbbackup.Job", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "UsbBackup",
	rpcGetMethod: "get",
	rpcSetMethod: "set",
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

	getFormConfig: function() {
 		return {
 			plugins: [{
 				ptype: "linkedfields",
 				correlations: [{
 					name: [
						"optionrecursive",
						"optiontimes"
					],
 					conditions: [{ name: "optionarchive", value: true }],
 					properties: "checked"
 				},{
 					name: [
 						"optionperms",
 						"optiongroup",
 						"optionowner"
 					],
 					conditions: [{
						name: "optionarchive",
						func: function(values) {
							var valid = values.optionarchive;
							if (this) {
								var field = this.findField("devicefile");
								var record = field.getSelection();
								if (record && !record.get("propposixacl")) {
									valid = false;
								}
							}
							return valid;
						}
					}],
 					properties: "checked"
 				},{
 					name: [
 						"optionperms",
 						"optiongroup",
 						"optionowner",
 						"optionacls"
 					],
 					conditions: [{
						name: "devicefile",
						func: function() {
							var valid = false;
							if (this) {
								var field = this.findField("devicefile");
								var record = field.getSelection();
								if (record && !record.get("propposixacl")) {
									valid = true;
								}
							}
							return valid;
						}
					}],
 					properties: [
						"!checked",
						"readOnly"
					]
 				}]
 			}]
 		};
 	},

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: true
		},{
			xtype: "combo",
			name: "mode",
			fieldLabel: _("Mode"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "push", _("From shared folder to external storage") ],
					[ "pull", _("From external storage to shared folder") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "push"
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			fieldLabel: _("Shared folder"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The shared folder to synchronise when the external storage device is connected.")
			}]
		},{
			xtype: "combo",
			name: "devicefile",
			fieldLabel: _("Device"),
			emptyText: _("Select a device ..."),
			allowBlank: false,
			allowNone: false,
			editable: false,
			triggerAction: "all",
			displayField: "description",
			valueField: "devicefile",
			triggers: {
				search: {
					cls: Ext.baseCSSPrefix + "form-search-trigger",
					handler: "onTrigger2Click"
				}
			},
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "uuid",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "devicefile", type: "string" },
						{ name: "label", type: "string" },
						{ name: "type", type: "string" },
						{ name: "description", type: "string" },
						{ name: "propposixacl", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "UsbBackup",
						method: "getCandidates"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			}),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The external storage device.")
			}],
			listeners: {
				scope: me,
				afterrender: function(c, eOpts) {
					// Add tooltip to trigger button.
					var trigger = c.getTrigger("search");
					Ext.tip.QuickTipManager.register({
						target: trigger.getEl(),
						text: _("Scan")
					});
				},
				beforedestroy: function(c, eOpts) {
					// Remove the quick tip from the trigger button.
					var trigger = c.getTrigger("search");
					Ext.tip.QuickTipManager.unregister(trigger.getEl());
				}
			},
			onTrigger2Click: function(c) {
				var me = this;
				// Reload list of detected external storage devices.
				delete me.lastQuery;
				me.store.reload();
			}
		},{
			xtype: "checkbox",
			name: "usesubdir",
			fieldLabel: "&nbsp",
			checked: true,
			boxLabel: _("Synchronise from/to directory on external storage device."),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The shared folder content is synchronised from/to the root of the external storage device if this option is not set. The name of the directory is taken from the shared folder.")
			}]
		},{
			xtype: "checkbox",
			name: "sendemail",
			fieldLabel: _("Send email"),
			checked: false,
			boxLabel: _("Send command output via email."),
			plugins: [{
				ptype: "fieldinfo",
				text: _("An email message with the command output (if any produced) is send to the administrator.")
			}]
		},{
			xtype: "checkbox",
			name: "optionquiet",
			fieldLabel: _("Quiet"),
			checked: false,
			boxLabel: _("Suppress non-error messages.")
		},{
			xtype: "checkbox",
			name: "optionarchive",
			fieldLabel: _("Archive"),
			checked: true,
			boxLabel: _("Enable archive mode.")
		},{
			xtype: "checkbox",
			name: "optionrecursive",
			fieldLabel: _("Recursive"),
			checked: true,
			boxLabel: _("Recurse into directories.")
		},{
			xtype: "checkbox",
			name: "optionperms",
			fieldLabel: _("Preserve permissions"),
			checked: true,
			boxLabel: _("Set the destination permissions to be the same as the source permissions.")
		},{
			xtype: "checkbox",
			name: "optiontimes",
			fieldLabel: _("Preserve modification times"),
			checked: true,
			boxLabel: _("Transfer modification times along with the files and update them on the remote system.")
		},{
			xtype: "checkbox",
			name: "optiongroup",
			fieldLabel: _("Preserve group"),
			checked: true,
			boxLabel: _("Set the group of the destination file to be the same as the source file.")
		},{
			xtype: "checkbox",
			name: "optionowner",
			fieldLabel: _("Preserve owner"),
			checked: true,
			boxLabel: _("Set the owner of the destination file to be the same as the source file, but only if the receiving rsync is being run as the super-user.")
		},{
			xtype: "checkbox",
			name: "optioncompress",
			fieldLabel: _("Compress"),
			checked: false,
			boxLabel: _("Compress file data during the transfer.")
		},{
			xtype: "checkbox",
			name: "optionacls",
			fieldLabel: _("Preserve ACLs"),
			checked: false,
			boxLabel: _("Update the destination ACLs to be the same as the source ACLs.")
		},{
			xtype: "checkbox",
			name: "optionxattrs",
			fieldLabel: _("Preserve extended attributes"),
			checked: false,
			boxLabel: _("Update the destination extended attributes to be the same as the local ones.")
		},{
			xtype: "checkbox",
			name: "optionpartial",
			fieldLabel: _("Keep partially transferred files"),
			checked: false,
			boxLabel: _("Enable this option to keep partially transferred files, otherwise they will be deleted if the transfer is interrupted.")
		},{
			xtype: "checkbox",
			name: "optiondelete",
			fieldLabel: _("Delete"),
			checked: false,
			boxLabel: _("Delete files on the receiving side that don't exist on sender.")
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: _("Extra options"),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Please check the <a href='http://www.samba.org/ftp/rsync/rsync.html' target='_blank'>manual page</a> for more details.")
			}]
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			vtype: "comment"
		}];
	}
});

/**
 * @class OMV.module.admin.service.usbbackup.Jobs
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.usbbackup.Jobs", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.window.Execute"
	],
	uses: [
		"OMV.module.admin.service.usbbackup.Job"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "693bddb2-7765-11e2-8c62-00221568ca88",
	columns: [{
		xtype: "enabledcolumn",
		text: _("Enabled"),
		sortable: true,
		dataIndex: "enable",
		stateId: "enable"
	},{
		xtype: "textcolumn",
		text: _("Shared folder"),
		sortable: true,
		dataIndex: "sharedfoldername",
		stateId: "sharedfoldername"
	},{
		xtype: "textcolumn",
		text: _("External storage device"),
		sortable: true,
		dataIndex: "devicefile",
		stateId: "devicefile"
	},{
		xtype: "textcolumn",
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
						{ name: "fsuuid", type: "string" },
						{ name: "sharedfoldername", type: "string" },
						{ name: "comment", type: "string" },
						{ name: "running", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "UsbBackup",
						method: "getList"
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

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		// Add 'Run' button to top toolbar
		Ext.Array.insert(items, 2, [{
			id: me.getId() + "-run",
			xtype: "button",
			text: _("Run"),
			iconCls: "x-fa fa-play",
			handler: Ext.Function.bind(me.onRunButton, me, [ me ]),
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1
			}
		}]);
		return items;
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.usbbackup.Job", {
			title: _("Add backup job"),
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
		Ext.create("OMV.module.admin.service.usbbackup.Job", {
			title: _("Edit backup job"),
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
				service: "UsbBackup",
				method: "delete",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	},

	onRunButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.window.Execute", {
			title: _("Execute backup job"),
			infoText: _("Please note that the web interface is blocked until the manually started backup job has been finished. However the backup job is automatically executed in the background when the storage device is connected to the host."),
			rpcService: "UsbBackup",
			rpcMethod: "execute",
			rpcParams: {
				uuid: record.get("uuid")
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
});

OMV.WorkspaceManager.registerPanel({
	id: "jobs",
	path: "/service/usbbackup",
	text: _("Backup jobs"),
	position: 10,
	className: "OMV.module.admin.service.usbbackup.Jobs"
});
