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
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/module/admin/Logs.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "rsync", {
	text: "Rsync",
	icon: "images/rsync.png"
});

/**
 * @class OMV.Module.Services.RsyncSrvTabPanel
 * @derived Ext.TabPanel
 */
OMV.Module.Services.RsyncSrvTabPanel = function(config) {
	var initialConfig = {
		border: false,
		activeTab: 0,
		layoutOnTabChange: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.RsyncSrvTabPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.RsyncSrvTabPanel, Ext.TabPanel, {
	initComponent : function() {
		this.items = [
			new OMV.Module.Services.RsyncSrvSettingsPanel,
			new OMV.Module.Services.RsyncSrvModuleGridPanel
		];
		OMV.Module.Services.RsyncSrvTabPanel.superclass.initComponent.apply(
		  this, arguments);
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "rsync", {
	cls: OMV.Module.Services.RsyncSrvTabPanel,
	title: "Server",
	position: 10
});

/**
 * @class OMV.Module.Services.RsyncSrvSettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.RsyncSrvSettingsPanel = function(config) {
	var initialConfig = {
		title: "Settings",
		rpcService: "Rsyncd",
		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.RsyncSrvSettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.RsyncSrvSettingsPanel, OMV.FormPanelExt, {
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
				xtype: "numberfield",
				name: "port",
				fieldLabel: "Port",
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 873
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

/**
 * @class OMV.Module.Services.RsyncSrvModuleGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.RsyncSrvModuleGridPanel = function(config) {
	var initialConfig = {
		title: "Modules",
		hidePagingToolbar: false,
		stateId: "72d6ab93-f08d-4d34-820b-fcbb832f723c",
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
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.RsyncSrvModuleGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.RsyncSrvModuleGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("Rsyncd", "getModuleList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "name" },
					{ name: "sharedfoldername" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Services.RsyncSrvModuleGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.RsyncSrvModulePropertyDialog({
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
		var wnd = new OMV.Module.Services.RsyncSrvModulePropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "Rsyncd",
			"deleteModule", [ record.get("uuid") ]);
	}
});

/**
 * @class OMV.Module.Services.RsyncSrvModulePropertyDialog
 */
OMV.Module.Services.RsyncSrvModulePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Rsyncd",
		rpcGetMethod: "getModule",
		rpcSetMethod: "setModule",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " module",
		width: 700,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.RsyncSrvModulePropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.RsyncSrvModulePropertyDialog,
	OMV.CfgObjectDialog, {
	initComponent : function() {
		OMV.Module.Services.RsyncSrvModulePropertyDialog.superclass.initComponent.apply(
		  this, arguments);
		this.on("load", this._updateFormFields, this);
	},

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
			allowBlank: true
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			hiddenName: "sharedfolderref",
			fieldLabel: "Shared folder",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The location of the files to share. Make sure the user 'nobody' has the required permission for the shared folder."
		},{
			xtype: "checkbox",
			name: "guestok",
			fieldLabel: "Public",
			checked: false,
			inputValue: 1,
			boxLabel: "If enabled then no password is required to connect to the share."
		},{
			xtype: "checkbox",
			name: "readonly",
			fieldLabel: "Read only",
			checked: false,
			inputValue: 1,
			boxLabel: "Set read only.",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If this parameter is set, then any attempted uploads will fail."
		},{
			xtype: "checkbox",
			name: "writeonly",
			fieldLabel: "Write only",
			checked: false,
			inputValue: 1,
			boxLabel: "Set write only.",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If this parameter is set, then any attempted downloads will fail."
		},{
			xtype: "numberfield",
			name: "maxconnections",
			fieldLabel: "Max. connections",
			minValue: 0,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			value: 0,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Specifies the maximum number of simultaneous connections. 0 means no limit."
		},{
			xtype: "checkbox",
			name: "list",
			fieldLabel: "List",
			checked: true,
			inputValue: 1,
			boxLabel: "Enable module listing.",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This option determines if this module should be listed when the client asks for a listing of available modules."
		},{
			xtype: "textfield",
			name: "hostsallow",
			fieldLabel: "Hosts allow",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This option is a comma, space, or tab delimited set of hosts which are permitted to access this module. You can specify the hosts by name or IP number. Leave this field empty to use default settings."
		},{
			xtype: "textfield",
			name: "hostsdeny",
			fieldLabel: "Hosts deny",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This option is a comma, space, or tab delimited set of host which are NOT permitted to access this module. Where the lists conflict, the allow list takes precedence. In the event that it is necessary to deny all by default, use the keyword ALL (or the netmask 0.0.0.0/0) and then explicitly specify to the hosts allow parameter those hosts that should be permitted access. Leave this field empty to use default settings."
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
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("sharedfolderref");
		if ((this.uuid !== OMV.UUID_UNDEFINED) && Ext.isDefined(field)) {
			field.setReadOnly(true);
		}
	}
});

/**
 * @class OMV.Module.Services.RsyncJobsGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.RsyncJobsGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "31924bfb-8e25-4ada-82f4-99a3a5c9e9a5",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Enabled",
				sortable: true,
				dataIndex: "enable",
				id: "enable",
				align: "center",
				width: 60,
				renderer: OMV.util.Format.booleanRenderer()
			},{
				header: "Source",
				sortable: true,
				dataIndex: "srcname",
				id: "srcname"
			},{
				header: "Destination",
				sortable: true,
				dataIndex: "destname",
				id: "destname"
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.RsyncJobsGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.RsyncJobsGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("Rsync", "getList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "enable" },
					{ name: "srcname" },
					{ name: "destname" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Services.RsyncJobsGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Services.RsyncJobsGridPanel.superclass.
		  initToolbar.apply(this);
		// Add 'Run' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-run",
			xtype: "button",
			text: "Run",
			icon: "images/run.png",
			handler: this.cbRunBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Services.RsyncJobsGridPanel.superclass.cbSelectionChangeHdl.apply(
		  this, arguments);
		// Process additional buttons
		var records = model.getSelections();
		var tbarRunCtrl = this.getTopToolbar().findById(this.getId() + "-run");
		if (records.length <= 0) {
			tbarRunCtrl.disable();
		} else if (records.length == 1) {
			tbarRunCtrl.enable();
		} else {
			tbarRunCtrl.disable();
		}
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.RsyncJobPropertyDialog({
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
		var wnd = new OMV.Module.Services.RsyncJobPropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "Rsync",
			"delete", [ record.get("uuid") ]);
	},

	cbRunBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.ExecCmdDialog({
			title: "Execute rsync job",
			rpcService: "Rsync",
			rpcMethod: "execute",
			rpcArgs: record.get("uuid"),
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
OMV.NavigationPanelMgr.registerPanel("services", "rsync", {
	cls: OMV.Module.Services.RsyncJobsGridPanel,
	title: "Jobs",
	position: 20
});

/**
 * @class OMV.Module.Services.RsyncJobPropertyDialog
 */
OMV.Module.Services.RsyncJobPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Rsync",
		rpcGetMethod: "get",
		rpcSetMethod: "set",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " rsync job",
		width: 700,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.RsyncJobPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.RsyncJobPropertyDialog, OMV.CfgObjectDialog, {
	initComponent : function() {
		OMV.Module.Services.RsyncJobPropertyDialog.superclass.initComponent.apply(
		  this, arguments);
		this.on("load", this._updateFormFields, this);
		this.on("show", this._updateFormFields, this);
	},

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
			fieldLabel: "Enable",
			checked: true,
			inputValue: 1
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true
		},{
			xtype: "combo",
			name: "type",
			hiddenName: "type",
			fieldLabel: "Type",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "local","Local" ],
					[ "remote","Remote" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "local",
			listeners: {
				select: function(combo, record, index) {
					// Update the form fields
					this._updateFormFields();
				},
				scope: this
			}
		},{
			xtype: "combo",
			name: "mode",
			hiddenName: "mode",
			fieldLabel: "Mode",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "push","Push" ],
					[ "pull","Pull" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "push",
			listeners: {
				select: function(combo, record, index) {
					// Update the form fields
					this._updateFormFields();
				},
				scope: this
			}
		},{
			xtype: "sharedfoldercombo",
			name: "srcsharedfolderref",
			hiddenName: "srcsharedfolderref",
			fieldLabel: "Source shared folder",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The source shared folder."
		},{
			xtype: "textfield",
			name: "srcuri",
			fieldLabel: "Source server",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The source remote server, e.g. rsync://[USER@]HOST[:PORT]/DEST."
		},{
			xtype: "sharedfoldercombo",
			name: "destsharedfolderref",
			hiddenName: "destsharedfolderref",
			fieldLabel: "Destination shared folder",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The source shared folder."
		},{
			xtype: "textfield",
			name: "desturi",
			fieldLabel: "Destination server",
			allowBlank: true,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The destination remote server, e.g. rsync://[USER@]HOST[:PORT]/DEST."
		},{
			xtype: "combo",
			name: "minute",
			hiddenName: "minute",
			fieldLabel: "Minute",
			mode: "local",
			store: Array.range(0, 59, 1, true).insert(0, "*"),
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: new Date().format("i")
		},{
			xtype: "combo",
			name: "hour",
			hiddenName: "hour",
			fieldLabel: "Hour",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapHour
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: new Date().format("H")
		},{
			xtype: "combo",
			name: "dayofmonth",
			hiddenName: "dayofmonth",
			fieldLabel: "Day of month",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapDayOfMonth
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "combo",
			name: "month",
			hiddenName: "month",
			fieldLabel: "Month",
			mode: "local",
			store: new Ext.data.SimpleStore({
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
			hiddenName: "dayofweek",
			fieldLabel: "Day of week",
			mode: "local",
			store: new Ext.data.SimpleStore({
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
			name: "recursive",
			fieldLabel: "Recursive",
			checked: true,
			inputValue: 1,
			boxLabel: "Recurse into directories."
		},{
			xtype: "checkbox",
			name: "times",
			fieldLabel: "Times",
			checked: true,
			inputValue: 1,
			boxLabel: "Preserve modification times."
		},{
			xtype: "checkbox",
			name: "compress",
			fieldLabel: "Compress",
			checked: false,
			inputValue: 1,
			boxLabel: "Compress file data during the transfer."
		},{
			xtype: "checkbox",
			name: "archive",
			fieldLabel: "Archive",
			checked: true,
			inputValue: 1,
			boxLabel: "Enable archive mode."
		},{
			xtype: "checkbox",
			name: "delete",
			fieldLabel: "Delete",
			checked: false,
			inputValue: 1,
			boxLabel: "Delete files on the receiving side that don't exist on sender."
		},{
			xtype: "checkbox",
			name: "quiet",
			fieldLabel: "Quiet",
			checked: false,
			inputValue: 1,
			boxLabel: "Suppress non-error messages."
		},{
			xtype: "checkbox",
			name: "perms",
			fieldLabel: "Preserve permissions",
			checked: true,
			inputValue: 1,
			boxLabel: "Set the destination permissions to be the same as the source permissions."
		},{
			xtype: "checkbox",
			name: "acls",
			fieldLabel: "Preserve ACLs",
			checked: true,
			inputValue: 1,
			boxLabel: "Update the destination ACLs to be the same as the source ACLs."
		},{
			xtype: "checkbox",
			name: "xattrs",
			fieldLabel: "Preserve extended attributes",
			checked: false,
			inputValue: 1,
			boxLabel: "Update the destination extended attributes to be the same as the local ones."
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: "Extra options",
			allowBlank: true
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var values = this.getValues();
		if (this.uuid !== OMV.UUID_UNDEFINED) {
			this.findFormField("type").setReadOnly(true);
		}
		switch (values.type) {
		case "local":
			var fields = [ "srcsharedfolderref", "destsharedfolderref" ];
			for (var i = 0; i < fields.length; i++) {
				var field = this.findFormField(fields[i]);
				field.enable();
				field.show();
				field.allowBlank = false;
			}
			fields = [ "srcuri", "desturi" ];
			for (i = 0; i < fields.length; i++) {
				var field = this.findFormField(fields[i]);
				field.disable();
				field.hide();
				field.allowBlank = true;
			}
			this.findFormField("mode").hide();
			break;
		case "remote":
			this.findFormField("mode").show();
			switch (values.mode) {
			case "push":
				var fields = [ "srcsharedfolderref", "desturi" ];
				for (var i = 0; i < fields.length; i++) {
					var field = this.findFormField(fields[i]);
					field.enable();
					field.show();
					field.allowBlank = false;
				}
				fields = [ "srcuri", "destsharedfolderref" ];
				for (i = 0; i < fields.length; i++) {
					var field = this.findFormField(fields[i]);
					field.disable();
					field.hide();
					field.allowBlank = true;
				}
				break;
			case "pull":
				var fields = [ "srcuri", "destsharedfolderref" ];
				for (var i = 0; i < fields.length; i++) {
					var field = this.findFormField(fields[i]);
					field.enable();
					field.show();
					field.allowBlank = false;
				}
				fields = [ "srcsharedfolderref", "desturi" ];
				for (i = 0; i < fields.length; i++) {
					var field = this.findFormField(fields[i]);
					field.disable();
					field.hide();
					field.allowBlank = true;
				}
				break;
			}
			break;
		}
	},

	isValid : function() {
		var valid = OMV.Module.Services.RsyncJobPropertyDialog.superclass.
		  isValid.call(this);
		if (!valid) {
			return valid;
		}
		// Do additional checks
		var values = this.getValues();
		if (values.type === "local") {
			if (values.srcsharedfolderref === values.destsharedfolderref) {
				var basicForm = this.form.getForm();
				var msg = "Shared folder must not be equal";
				basicForm.markInvalid([
					{ id: "srcsharedfolderref", msg: msg },
					{ id: "destsharedfolderref", msg: msg }
				]);
				valid = false;
			}
		}
		return valid;
	}
});

Ext.ns("OMV.Module.Diagnostics.LogPlugin");

/**
 * @class OMV.Module.Diagnostics.LogPlugin.Rsync
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'Rsync' logfile diagnostics plugin
 */
OMV.Module.Diagnostics.LogPlugin.Rsync = function(config) {
	var initialConfig = {
		title: "Rsync",
		stateId: "835f3a32-bc7d-4102-b1cb-a598a0ae14ad",
		columns: [{
			header: "Date & Time",
			sortable: true,
			dataIndex: "date",
			id: "date",
			width: 40
		},{
			header: "Event",
			sortable: true,
			dataIndex: "event",
			id: "event"
		}],
		rpcArgs: "rsync",
		rpcFields: [
			{ name: "date" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.Rsync.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.Rsync,
  OMV.Module.Diagnostics.LogPlugin, {
});
OMV.preg("log", "rsync", OMV.Module.Diagnostics.LogPlugin.Rsync);
