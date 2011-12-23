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
// require("js/omv/PluginMgr.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/DiagPanel.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Services.SMB");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "smbcifs", {
	text: "SMB/CIFS",
	icon: "images/smb.png"
});

/**
 * @class OMV.Module.Services.SMB.SettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.SMB.SettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "SMB",
 		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.SMB.SettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.SMB.SettingsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "General settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1
			},{
				xtype: "textfield",
				name: "workgroup",
				fieldLabel: "Workgroup",
				allowBlank: false,
				value: "WORKGROUP",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The workgroup the server will appear to be in when queried by clients."
			},{
				xtype: "textfield",
				name: "serverstring",
				fieldLabel: "Description",
				allowBlank: false,
				value: "%h server",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The NT description field."
			},{
				xtype: "checkbox",
				name: "localmaster",
				fieldLabel: "Local master browser",
				checked: true,
				inputValue: 1,
				boxLabel: "Allow this server to try and become a local master browser"
			},{
				xtype: "checkbox",
				name: "timeserver",
				fieldLabel: "Time server",
				checked: false,
				inputValue: 1,
				boxLabel: "Allow this server to advertise itself as a time server to Windows clients"
			}]
		},{
			xtype: "fieldset",
			title: "Home directories",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "homesenable",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable user home directories"
			},{
				xtype: "checkbox",
				name: "homesbrowseable",
				fieldLabel: "Browseable",
				checked: true,
				inputValue: 1,
				boxLabel: "Set browseable",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "This controls whether this share is seen in the list of available shares in a net view and in the browse list."
			}]
		},{
			xtype: "fieldset",
			title: "WINS",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "winssupport",
				fieldLabel: "WINS support",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable WINS server",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Act as a WINS server."
			},{
				xtype: "textfield",
				name: "winsserver",
				fieldLabel: "WINS server",
				allowBlank: true,
				value: "",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Use the specified WINS server."
			}]
		},{
			xtype: "fieldset",
			title: "Advanced settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "combo",
				name: "loglevel",
				hiddenName: "loglevel",
				fieldLabel: "Log level",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,"None" ],
						[ 1,"Minimum" ],
						[ 2,"Normal" ],
						[ 3,"Full" ],
						[ 10,"Debug" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 0
			},{
				xtype: "checkbox",
				name: "nullpasswords",
				fieldLabel: "Null passwords",
				checked: false,
				inputValue: 1,
				boxLabel: "Allow client access to accounts that have null passwords"
			},{
				xtype: "checkbox",
				name: "usesendfile",
				fieldLabel: "Use sendfile",
				checked: false,
				inputValue: 1,
				boxLabel: "Use the more efficient sendfile system call for files that are exclusively oplocked",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "This may make more efficient use of the system CPU's and cause Samba to be faster. Samba automatically turns this off for clients that use protocol levels lower than NT LM 0.12 and when it detects a client is Windows 9x."
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: "Extra options",
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "3",
					cols: "65"
				}
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "smbcifs", {
	cls: OMV.Module.Services.SMB.SettingsPanel,
	title: "Settings",
	position: 10
});

/**
 * @class OMV.Module.Services.SMB.SharesGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.SMB.SharesGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "e0bbeb12-8441-4b70-899e-800e2dab53ee",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Shared folder",
				sortable: true,
				dataIndex: "sharedfoldername",
				id: "sharedfoldername"
			},{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			},{
				header: "Public",
				sortable: true,
				dataIndex: "guestok",
				id: "guestok",
				renderer: OMV.util.Format.booleanRenderer()
			},{
				header: "Read only",
				sortable: true,
				dataIndex: "readonly",
				id: "readonly",
				renderer: OMV.util.Format.booleanRenderer()
			},{
				header: "Browseable",
				sortable: true,
				dataIndex: "browseable",
				id: "browseable",
				renderer: OMV.util.Format.booleanRenderer()
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.SMB.SharesGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.SMB.SharesGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("SMB", "getShareList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "sharedfoldername" },
					{ name: "name" },
					{ name: "comment" },
					{ name: "browseable" },
					{ name: "readonly" },
					{ name: "guestok" }
    			]
			})
		});
		OMV.Module.Services.SMB.SharesGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.SMB.SharePropertyDialog({
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
		var wnd = new OMV.Module.Services.SMB.SharePropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "SMB", "deleteShare",
		  [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "smbcifs", {
	cls: OMV.Module.Services.SMB.SharesGridPanel,
	title: "Shares",
	position: 20
});

/**
 * @class OMV.Module.Services.SMB.SharePropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.SMB.SharePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "SMB",
		rpcGetMethod: "getShare",
		rpcSetMethod: "setShare",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " share",
		width: 700,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.SMB.SharePropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.SMB.SharePropertyDialog,
  OMV.CfgObjectDialog, {
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
			xtype: "textfield",
			name: "name",
			fieldLabel: "Name",
			allowBlank: false,
			vtype: "sharename",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The name of the share."
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This is a text field that is seen next to a share when a client queries the server."
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			hiddenName: "sharedfolderref",
			fieldLabel: "Shared folder",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The location of the files to share."
		},{
			xtype: "checkbox",
			name: "guestok",
			fieldLabel: "Public",
			checked: false,
			inputValue: 1,
			boxLabel: "If enabled then no password is required to connect to the share",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Make sure the user 'nobody' has the required permission for the shared folder."
		},{
			xtype: "checkbox",
			name: "readonly",
			fieldLabel: "Read only",
			checked: false,
			inputValue: 1,
			boxLabel: "Set read only",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If this parameter is set, then users may not create or modify files in the share."
		},{
			xtype: "checkbox",
			name: "browseable",
			fieldLabel: "Browseable",
			checked: true,
			inputValue: 1,
			boxLabel: "Set browseable",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This controls whether this share is seen in the list of available shares in a net view and in the browse list."
		},{
			xtype: "checkbox",
			name: "inheritacls",
			fieldLabel: "Inherit ACLs",
			checked: false,
			inputValue: 1,
			boxLabel: "Honor existing ACLs",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This parameter can be used to ensure that if default acls exist on parent directories, they are always honored when creating a new file or subdirectory in these parent directories."
		},{
			xtype: "checkbox",
			name: "inheritpermissions",
			fieldLabel: "Inherit permissions",
			checked: false,
			inputValue: 1,
			boxLabel: "Enable permission inheritance",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The permissions on new files and directories are normally governed by create mask and directory mask but the inherit permissions parameter overrides this. This can be particularly useful on systems with many users to allow a single share to be used flexibly by each user."
		},{
			xtype: "checkbox",
			name: "recyclebin",
			fieldLabel: "Recycle bin",
			checked: false,
			inputValue: 1,
			boxLabel: "Enable recycle bin",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This will create a recycle bin on the share."
		},{
			xtype: "checkbox",
			name: "hidedotfiles",
			fieldLabel: "Hide dot files",
			checked: true,
			inputValue: 1,
			boxLabel: "This parameter controls whether files starting with a dot appear as hidden files"
		},{
			xtype: "checkbox",
			name: "easupport",
			fieldLabel: "Extended attributes",
			checked: false,
			inputValue: 1,
			boxLabel: "Enable extended attribute support",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Allow clients to attempt to store OS/2 style extended attributes on a share."
		},{
			xtype: "checkbox",
			name: "storedosattributes",
			fieldLabel: "Store DOS attributes",
			checked: false,
			inputValue: 1,
			boxLabel: "Enable store DOS attributes support",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If this parameter is set, Samba attempts to first read DOS attributes (SYSTEM, HIDDEN, ARCHIVE or READ-ONLY) from a filesystem extended attribute, before mapping DOS attributes to UNIX permission bits. When set, DOS attributes will be stored onto an extended attribute in the UNIX filesystem, associated with the file or directory."
		},{
			xtype: "textfield",
			name: "hostsallow",
			fieldLabel: "Hosts allow",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This option is a comma, space, or tab delimited set of hosts which are permitted to access this share. You can specify the hosts by name or IP number. Leave this field empty to use default settings."
		},{
			xtype: "textfield",
			name: "hostsdeny",
			fieldLabel: "Hosts deny",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This option is a comma, space, or tab delimited set of host which are NOT permitted to access this share. Where the lists conflict, the allow list takes precedence. In the event that it is necessary to deny all by default, use the keyword ALL (or the netmask 0.0.0.0/0) and then explicitly specify to the hosts allow parameter those hosts that should be permitted access. Leave this field empty to use default settings."
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: "Extra options",
			allowBlank: true,
			autoCreate: {
				tag: "textarea",
				autocomplete: "off",
				rows: "3",
				cols: "65"
			}
		}];
	}
});

OMV.Module.Services.SMB.DiagPanel = function(config) {
	var initialConfig = {
		title: "SMB/CIFS",
		layout: "fit",
		items: [{
			id: this.getId() + "-content",
			xtype: "textarea",
			readOnly: true,
			cls: "x-form-textarea-monospaced",
			disabledClass: ""
		}]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.SMB.DiagPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.SMB.DiagPanel, OMV.DiagPanel, {
	doLoad : function() {
		OMV.MessageBox.wait(null, "Loading ...");
		OMV.Ajax.request(function(id, response, error) {
			  OMV.MessageBox.updateProgress(1);
			  OMV.MessageBox.hide();
			  if (error !== null) {
				  OMV.MessageBox.error(null, error);
			  } else {
				  var comp = this.getComponent(this.getId() + "-content");
				  if (!Ext.isEmpty(comp)) {
					  comp.setValue(response);
				  }
			  }
		  }, this, "SMB", "getStats");
	}
});
OMV.preg("sysinfo", "service", OMV.Module.Services.SMB.DiagPanel);
