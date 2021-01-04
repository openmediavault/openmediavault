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
// require("js/omv/form/field/Password.js")
// require("js/omv/form/field/SharedFolderComboBox.js")
// require("js/omv/form/field/SshCertificateComboBox.js")

/**
 * @class OMV.module.admin.service.rsync.Job
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.rsync.Job", {
	extend: "OMV.workspace.window.Form",
	uses: [
		"OMV.form.field.Password",
		"OMV.form.field.SharedFolderComboBox",
		"OMV.form.field.SshCertificateComboBox",
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "Rsync",
	rpcGetMethod: "get",
	rpcSetMethod: "set",
	plugins: [{
		ptype: "configobject"
	}],
	width: 570,
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
					name: "srcsharedfolderref",
					conditions: [{
						func: function(values) {
							var valid = false;
							if(values.type === "local")
								valid = true;
							else if((values.type === "remote") &&
							  (values.mode === "push"))
								valid = true;
							return valid;
						}
					}],
					properties: [
						"show",
						"!allowBlank"
					]
				},{
					name: "destsharedfolderref",
					conditions: [{
						func: function(values) {
							var valid = false;
							if(values.type === "local")
								valid = true;
							else if((values.type === "remote") &&
							  (values.mode === "pull"))
								valid = true;
							return valid;
						}
					}],
					properties: [
						"show",
						"!allowBlank"
					]
				},{
					name: "mode",
					conditions: [
						{ name: "type", value: "local" }
					],
					properties: "hide"
				},{
					name: "desturi",
					conditions: [
						{ name: "type", value: "remote" },
						{ name: "mode", value: "push" }
					],
					properties: [
						"show",
						"!allowBlank"
					]
				},{
					name: "srcuri",
					conditions: [
						{ name: "type", value: "remote" },
						{ name: "mode", value: "pull" }
					],
					properties: [
						"show",
						"!allowBlank"
					]
				},{
					name: "authentication",
					conditions: [
						{ name: "type", value: "remote" }
					],
					properties: "show"
				},{
					name: "sshport",
					conditions: [
						{ name: "type", value: "remote" },
						{ name: "authentication", value: "pubkey" }
					],
					properties: [
						"show",
						"!allowBlank"
					]
				},{
					name: "sshcertificateref",
					conditions: [
						{ name: "type", value: "remote" },
						{ name: "authentication", value: "pubkey" }
					],
					properties: [
						"show",
						"!allowBlank"
					]
				},{
					name: "password",
					conditions: [
						{ name: "type", value: "remote" },
						{ name: "authentication", value: "password" }
					],
					properties: "show"
				},{
					name: [
						"optionrecursive",
						"optionperms",
						"optiontimes",
						"optiongroup",
						"optionowner"
					],
					conditions: [
						{ name: "optionarchive", value: true }
					],
					properties: "checked"
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
			name: "type",
			fieldLabel: _("Type"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "local", _("Local") ],
					[ "remote", _("Remote") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			readOnly: (me.uuid !== OMV.UUID_UNDEFINED),
			value: "local"
		},{
			xtype: "combo",
			name: "mode",
			fieldLabel: _("Mode"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "push", _("Push") ],
					[ "pull", _("Pull") ]
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
			name: "srcsharedfolderref",
			fieldLabel: _("Source shared folder"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The source shared folder.")
			}]
		},{
			xtype: "textfield",
			name: "srcuri",
			fieldLabel: _("Source server"),
			allowBlank: true,
			hidden: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("The source remote server, e.g. [USER@]HOST:SRC, [USER@]HOST::SRC or rsync://[USER@]HOST[:PORT]/SRC.")
			}]
		},{
			xtype: "sharedfoldercombo",
			name: "destsharedfolderref",
			fieldLabel: _("Destination shared folder"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The destination shared folder.")
			}]
		},{
			xtype: "textfield",
			name: "desturi",
			fieldLabel: _("Destination server"),
			allowBlank: true,
			hidden: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("The destination remote server, e.g. [USER@]HOST:DEST, [USER@]HOST::DEST or rsync://[USER@]HOST[:PORT]/DEST.")
			}]
		},{
			xtype: "combo",
			name: "authentication",
			fieldLabel: _("Authentication"),
			emptyText: _("Select an authentication mode ..."),
			queryMode: "local",
			store: [
				[ "password", _("Password") ],
				[ "pubkey", _("Public key") ]
			],
			allowBlank: false,
			editable: false,
			hidden: true,
			triggerAction: "all",
			value: "password"
		},{
			xtype: "numberfield",
			name: "sshport",
			fieldLabel: _("SSH port"),
			vtype: "port",
			minValue: 1,
			maxValue: 65535,
			allowDecimals: false,
			allowBlank: false,
			value: 22
		},{
			xtype: "sshcertificatecombo",
			name: "sshcertificateref",
			fieldLabel: _("SSH certificate"),
			allowBlank: false,
			hidden: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("The SSH certificate used for authentication.")
			}]
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: _("Password"),
			allowBlank: true,
			autoComplete: false,
			hidden: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("The password that is used for access via rsync daemon. Note, this is not used for remote shell transport such as ssh.")
			}]
		},{
			xtype: "compositefield",
			fieldLabel: _("Minute"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "minute",
				queryMode: "local",
				store: Ext.Array.insert(Ext.Array.range(0, 59, 1, true),
				  0, [ "*" ]),
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: String(new Date().getMinutes()),
				flex: 1
			},{
				xtype: "checkbox",
				name: "everynminute",
				fieldLabel: "",
				checked: false,
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
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: Date.mapHour
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: String(new Date().getHours()),
				flex: 1
			},{
				xtype: "checkbox",
				name: "everynhour",
				fieldLabel: "",
				checked: false,
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
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
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
				boxLabel: _("Every N day of month"),
				width: 140
			}]
		},{
			xtype: "combo",
			name: "month",
			fieldLabel: _("Month"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
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
			fieldLabel: _("Day of week"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
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
			name: "sendemail",
			fieldLabel: _("Send email"),
			checked: false,
			boxLabel: _("Send command output via email"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("An email message with the command output (if any produced) is send to the administrator.")
			}]
		},{
			xtype: "checkbox",
			name: "optiondryrun",
			fieldLabel: _("Trial run"),
			checked: false,
			boxLabel: _("Perform a trial run with no changes made")
		},{
			xtype: "checkbox",
			name: "optionquiet",
			fieldLabel: _("Quiet"),
			checked: false,
			boxLabel: _("Suppress non-error messages")
		},{
			xtype: "checkbox",
			name: "optionarchive",
			fieldLabel: _("Archive"),
			checked: true,
			boxLabel: _("Enable archive mode")
		},{
			xtype: "checkbox",
			name: "optionrecursive",
			fieldLabel: _("Recursive"),
			checked: true,
			boxLabel: _("Recurse into directories")
		},{
			xtype: "checkbox",
			name: "optionperms",
			fieldLabel: _("Preserve permissions"),
			checked: true,
			boxLabel: _("Set the destination permissions to be the same as the source permissions")
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
			boxLabel: _("Compress file data during the transfer")
		},{
			xtype: "checkbox",
			name: "optionacls",
			fieldLabel: _("Preserve ACLs"),
			checked: false,
			boxLabel: _("Update the destination ACLs to be the same as the source ACLs")
		},{
			xtype: "checkbox",
			name: "optionxattrs",
			fieldLabel: _("Preserve extended attributes"),
			checked: false,
			boxLabel: _("Update the destination extended attributes to be the same as the local ones")
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
			boxLabel: _("Delete files on the receiving side that don't exist on sender")
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
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	updateFormFields: function() {
		var values = this.getValues();
		if (this.uuid !== OMV.UUID_UNDEFINED) {
			this.findField("type").setReadOnly(true);
		}
		switch (values.type) {
		case "local":
			var fields = [ "srcsharedfolderref", "destsharedfolderref" ];
			for (var i = 0; i < fields.length; i++) {
				var field = this.findField(fields[i]);
				field.enable();
				field.show();
				field.allowBlank = false;
			}
			fields = [ "srcuri", "desturi" ];
			for (i = 0; i < fields.length; i++) {
				var field = this.findField(fields[i]);
				field.disable();
				field.hide();
				field.allowBlank = true;
			}
			this.findField("mode").hide();
			this.findField("password").hide();
			break;
		case "remote":
			this.findField("mode").show();
			this.findField("password").show();
			switch (values.mode) {
			case "push":
				var fields = [ "srcsharedfolderref", "desturi" ];
				for (var i = 0; i < fields.length; i++) {
					var field = this.findField(fields[i]);
					field.enable();
					field.show();
					field.allowBlank = false;
				}
				fields = [ "srcuri", "destsharedfolderref" ];
				for (i = 0; i < fields.length; i++) {
					var field = this.findField(fields[i]);
					field.disable();
					field.hide();
					field.allowBlank = true;
				}
				break;
			case "pull":
				var fields = [ "srcuri", "destsharedfolderref" ];
				for (var i = 0; i < fields.length; i++) {
					var field = this.findField(fields[i]);
					field.enable();
					field.show();
					field.allowBlank = false;
				}
				fields = [ "srcsharedfolderref", "desturi" ];
				for (i = 0; i < fields.length; i++) {
					var field = this.findField(fields[i]);
					field.disable();
					field.hide();
					field.allowBlank = true;
				}
				break;
			}
			break;
		}
		this.clearInvalid();
	},

	isValid: function() {
		var me = this;
		if (!me.callParent(arguments))
			return false;
		// Do additional checks.
		var valid = true;
		var values = me.getValues();
		if (values.type === "local") {
			if (values.srcsharedfolderref === values.destsharedfolderref) {
				var msg = _("Shared folder must not be equal");
				me.markInvalid([
					{ id: "srcsharedfolderref", msg: msg },
					{ id: "destsharedfolderref", msg: msg }
				]);
				valid = false;
			}
		}
		// It is not allowed to select '*' if the everyxxx checkbox is checked.
		[ "minute", "hour", "dayofmonth" ].forEach(function(name) {
			var field = me.findField(name);
			field.clearInvalid(); // combineErrors is false
			if ((field.getValue() === "*") && (me.findField("everyn" +
			  name).checked)) {
				field.markInvalid(_("Ranges of numbers are not allowed"));
				valid = false;
			}
		});
		return valid;
	}
});

/**
 * @class OMV.module.admin.service.rsync.Jobs
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.rsync.Jobs", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.window.Execute"
	],
	uses: [
		"OMV.module.admin.service.rsync.Job"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "31924bfb-8e25-4ada-82f4-99a3a5c9e9a5",
	columns: [{
		xtype: "enabledcolumn",
		text: _("Enabled"),
		sortable: true,
		dataIndex: "enable",
		stateId: "enable"
	},{
		xtype: "cronscheduling",
		text: _("Scheduling"),
		sortable: true,
		stateId: "scheduling"
	},{
		xtype: "mapcolumn",
		text: _("Type"),
		sortable: true,
		dataIndex: "type",
		stateId: "type",
		mapItems: {
			"local": _("Local"),
			"remote": _("Remote")
		}
	},{
		xtype: "textcolumn",
		text: _("Source"),
		sortable: true,
		dataIndex: "srcname",
		stateId: "srcname"
	},{
		xtype: "textcolumn",
		text: _("Destination"),
		sortable: true,
		dataIndex: "destname",
		stateId: "destname"
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
						{ name: "srcname", type: "string" },
						{ name: "destname", type: "string" },
						{ name: "comment", type: "string" },
						{ name: "type", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Rsync",
						method: "getList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "srcname"
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
		Ext.create("OMV.module.admin.service.rsync.Job", {
			title: _("Add rsync job"),
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
		Ext.create("OMV.module.admin.service.rsync.Job", {
			title: _("Edit rsync job"),
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
				service: "Rsync",
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
			title: _("Execute rsync job"),
			rpcService: "Rsync",
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
	path: "/service/rsync",
	text: _("Jobs"),
	position: 10,
	className: "OMV.module.admin.service.rsync.Jobs"
});
