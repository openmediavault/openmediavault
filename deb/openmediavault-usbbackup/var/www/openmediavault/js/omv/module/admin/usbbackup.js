/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "usbbackup", {
	text: _("USB Backup"),
	icon: "images/rsync.png"
});

Ext.ns("OMV.Module.Services.UsbBackup");

/**
 * @class OMV.Module.Services.UsbBackup.JobsGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.UsbBackup.JobsGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "693bddb2-7765-11e2-8c62-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Enabled"),
				sortable: true,
				dataIndex: "enable",
				id: "enable",
				align: "center",
				width: 60,
				renderer: OMV.util.Format.booleanIconRenderer(
				  "switch_on.png", "switch_off.png")
			},{
				header: _("Shared folder"),
				sortable: true,
				dataIndex: "sharedfoldername",
				id: "sharedfoldername"
			},{
				header: _("External storage device"),
				sortable: true,
				dataIndex: "fsuuid",
				id: "fsuuid"
			},{
				header: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			},{
				header: _("Running"),
				sortable: true,
				dataIndex: "running",
				id: "running",
				align: "center",
				width: 60,
				renderer: OMV.util.Format.booleanIconRenderer()
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.UsbBackup.JobsGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.UsbBackup.JobsGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "UsbBackup",
				"method": "getList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "enable" },
					{ name: "fsuuid" },
					{ name: "sharedfoldername" },
					{ name: "comment" },
					{ name: "running" }
    			]
			})
		});
		OMV.Module.Services.UsbBackup.JobsGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Services.UsbBackup.JobsGridPanel.superclass.
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
		OMV.Module.Services.UsbBackup.JobsGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons
		var records = model.getSelections();
		var tbarRunCtrl = this.getTopToolbar().findById(this.getId() + "-run");
		if(records.length <= 0) {
			tbarRunCtrl.disable();
		} else if (records.length == 1) {
			tbarRunCtrl.enable();
		} else {
			tbarRunCtrl.disable();
		}
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.UsbBackup.JobPropertyDialog({
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
		var wnd = new OMV.Module.Services.UsbBackup.JobPropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "UsbBackup",
		  "delete", { "uuid": record.get("uuid") });
	},

	cbRunBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.ExecCmdDialog({
			title: _("Execute USB backup job"),
			rpcService: "UsbBackup",
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
OMV.NavigationPanelMgr.registerPanel("services", "usbbackup", {
	cls: OMV.Module.Services.UsbBackup.JobsGridPanel
});

/**
 * @class OMV.Module.Services.UsbBackup.JobPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.UsbBackup.JobPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "UsbBackup",
		rpcGetMethod: "get",
		rpcSetMethod: "set",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add USB backup job") : _("Edit USB backup job"),
		width: 570,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.UsbBackup.JobPropertyDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.UsbBackup.JobPropertyDialog,
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
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: true,
			inputValue: 1
		},{
			xtype: "combo",
			name: "mode",
			hiddenName: "mode",
			fieldLabel: _("Mode"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "push",_("From shared folder to external storage") ],
					[ "pull",_("From external storage to shared folder") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "push"
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			vtype: "comment"
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			hiddenName: "sharedfolderref",
			fieldLabel: _("Shared folder"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The shared folder to synchronise when the external storage device is connected.")
		},{
			xtype: "combo",
			name: "fsuuid",
			hiddenName: "fsuuid",
			fieldLabel: _("Volume"),
			emptyText: _("Select a volume ..."),
			allowBlank: false,
			allowNone: false,
			editable: false,
			triggerAction: "all",
			displayField: "description",
			valueField: "uuid",
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "UsbBackup",
					"method": "getCandidates",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "uuid",
					fields: [
						{ name: "uuid" },
						{ name: "description" }
					]
				})
			}),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The external storage device.")
		},{
			xtype: "checkbox",
			name: "recursive",
			fieldLabel: _("Recursive"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Recurse into directories.")
		},{
			xtype: "checkbox",
			name: "times",
			fieldLabel: _("Times"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Preserve modification times.")
		},{
			xtype: "checkbox",
			name: "compress",
			fieldLabel: _("Compress"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Compress file data during the transfer.")
		},{
			xtype: "checkbox",
			name: "archive",
			fieldLabel: _("Archive"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Enable archive mode.")
		},{
			xtype: "checkbox",
			name: "delete",
			fieldLabel: _("Delete"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Delete files on the receiving side that don't exist on sender.")
		},{
			xtype: "checkbox",
			name: "quiet",
			fieldLabel: _("Quiet"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Suppress non-error messages.")
		},{
			xtype: "checkbox",
			name: "perms",
			fieldLabel: _("Preserve permissions"),
			checked: true,
			inputValue: 1,
			boxLabel: _("Set the destination permissions to be the same as the source permissions.")
		},{
			xtype: "checkbox",
			name: "acls",
			fieldLabel: _("Preserve ACLs"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Update the destination ACLs to be the same as the source ACLs.")
		},{
			xtype: "checkbox",
			name: "xattrs",
			fieldLabel: _("Preserve extended attributes"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Update the destination extended attributes to be the same as the local ones.")
		},{
			xtype: "checkbox",
			name: "partial",
			fieldLabel: _("Keep partially transferred files"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Enable this option to keep partially transferred files, otherwise they will be deleted if the transfer is interrupted.")
		},{
			xtype: "checkbox",
			name: "sendemail",
			fieldLabel: _("Send email"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Send command output via email."),
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
	}
});
