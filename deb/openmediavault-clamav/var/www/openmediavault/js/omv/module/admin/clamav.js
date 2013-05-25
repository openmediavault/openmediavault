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
// require("js/omv/ModuleManager.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/field/SharedFolderComboBox.js")
// require("js/omv/form/field/plugin/FieldInfo.js")
// require("js/omv/module/admin/Logs.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.ModuleManager.registerMenu("services", "clamav", {
	text: _("Antivirus"),
	icon16: "images/antivirus.png"
});

Ext.ns("OMV.Module.Services.ClamAV");

/**
 * @class OMV.Module.Services.ClamAV.SettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.ClamAV.SettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "ClamAV",
 		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.ClamAV.SettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.ClamAV.SettingsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("General settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false
			},{
				xtype: "textfield",
				name: "databasemirror",
				fieldLabel: _("Database mirror"),
				allowBlank: false,
				value: "db.local.clamav.net",
				plugins: [{
					ptype: "fieldinfo",
					text: _("Server name where database updates are downloaded from. Get a complete list of database mirrors <a href='http://www.clamav.net/mirrors.html' target='_blank'>here</a>.")
				}]
			},{
				xtype: "numberfield",
				name: "checks",
				fieldLabel: _("Database checks"),
				minValue: 0,
				maxValue: 50,
				allowDecimals: false,
				allowBlank: false,
				value: 24,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Number of database update checks per day. Set to 0 to disable.")
				}]
			},{
				xtype: "checkbox",
				name: "logclean",
				fieldLabel: _("Log clean files"),
				boxLabel: _("Log clean files. This drastically increases the log size."),
				checked: false
			},{
				xtype: "checkbox",
				name: "scanpe",
				fieldLabel: _("Scan Portable Executable"),
				boxLabel: _("Perform a deeper analysis of executable files."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanole2",
				fieldLabel: _("Scan OLE2"),
				boxLabel: _("Enable scanning of OLE2 files, such as Microsoft Office documents and .msi files."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanhtml",
				fieldLabel: _("Scan HTML"),
				boxLabel: _("Enable HTML detection and normalisation."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanpdf",
				fieldLabel: _("Scan PDF"),
				boxLabel: _("Enable scanning within PDF files."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanelf",
				fieldLabel: _("Scan ELF"),
				boxLabel: _("Enable scanning of ELF files."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanarchive",
				fieldLabel: _("Scan archives"),
				boxLabel: _("Enable archive scanning."),
				checked: true
			},{
				xtype: "checkbox",
				name: "detectbrokenexecutables",
				fieldLabel: _("Detect broken executables"),
				boxLabel: _("Enable the detection of broken executables (both PE and ELF)."),
				checked: false
			},{
				xtype: "checkbox",
				name: "detectpua",
				fieldLabel: _("Detect PUA"),
				boxLabel: _("Enable the detection of possibly unwanted applications."),
				checked: false
			},{
				xtype: "checkbox",
				name: "algorithmicdetection",
				fieldLabel: _("Algorithmic detection"),
				boxLabel: _("Enable the algorithmic detection."),
				checked: true
			},{
				xtype: "checkbox",
				name: "followdirectorysymlinks",
				fieldLabel: _("Follow directory symlinks"),
		   		boxLabel: _("Follow directory symlinks."),
				checked: false
			},{
				xtype: "checkbox",
				name: "followfilesymlinks",
				fieldLabel: _("Follow file symlinks"),
				boxLabel: _("Follow regular file symlinks."),
				checked: false
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "3",
					cols: "65"
				},
				plugins: [{
					ptype: "fieldinfo",
					text: _("Please check the <a href='http://linux.die.net/man/5/clamd.conf' target='_blank'>manual page</a> for more details."),
				}]
			}]
		}];
	}
});
OMV.ModuleManager.registerPanel("services", "clamav", {
	cls: OMV.Module.Services.ClamAV.SettingsPanel,
	title: _("Settings"),
	position: 10
});

/**
 * @class OMV.Module.Services.ClamAV.JobGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.ClamAV.JobGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateful: true,
		stateId: "f8a8cf1c-a107-11e1-a5a0-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				text: _("Enabled"),
				sortable: true,
				dataIndex: "enable",
				stateId: "enable",
				align: "center",
				width: 80,
				resizable: false,
				renderer: OMV.util.Format.booleanIconRenderer(
				  "switch_on.png", "switch_off.png")
			},{
				text: _("Shared folder"),
				sortable: true,
				dataIndex: "sharedfoldername",
				stateId: "sharedfoldername"
			},{
				text: _("Minute"),
				sortable: true,
				dataIndex: "minute",
				stateId: "minute",
				renderer: function(value, metaData, record, rowIndex,
				  colIndex, store, view) {
					var everynminute = record.get("everynminute");
					if(everynminute == true) {
						value = "*/" + value;
					}
					return value;
				}
			},{
				text: _("Hour"),
				sortable: true,
				dataIndex: "hour",
				stateId: "hour",
				renderer: function(value, metaData, record, rowIndex,
				  colIndex, store, view) {
					var everynhour = record.get("everynhour");
					var func = OMV.util.Format.arrayRenderer(Date.mapHour);
					value = func(value);
					if(everynhour == true) {
						value = "*/" + value;
					}
					return value;
				}
			},{
				text: _("Day of month"),
				sortable: true,
				dataIndex: "dayofmonth",
				stateId: "dayofmonth",
				renderer: function(value, metaData, record, rowIndex,
				  colIndex, store, view) {
					var everyndayofmonth = record.get("everyndayofmonth");
					var func = OMV.util.Format.arrayRenderer(
					  Date.mapDayOfMonth);
					value = func(value);
					if(everyndayofmonth == true) {
						value = "*/" + value;
					}
					return value;
				}
			},{
				text: _("Month"),
				sortable: true,
				dataIndex: "month",
				stateId: "month",
				renderer: OMV.util.Format.arrayRenderer(Date.mapMonth)
			},{
				text: _("Day of week"),
				sortable: true,
				dataIndex: "dayofweek",
				stateId: "dayofweek",
				renderer: OMV.util.Format.arrayRenderer(Date.mapDayOfWeek)
			},{
				text: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				stateId: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.ClamAV.JobGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.ClamAV.JobGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			proxy: new OMV.data.DataProxy({
				"rpcOptions": {
					"rpcData": {
						"service": "ClamAV",
						"method": "getJobList"
					}
				}
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "enable" },
					{ name: "sharedfoldername" },
					{ name: "minute" },
					{ name: "everynminute" },
					{ name: "hour" },
					{ name: "everynhour" },
					{ name: "dayofmonth" },
					{ name: "everyndayofmonth" },
					{ name: "month" },
					{ name: "dayofweek" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Services.ClamAV.JobGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Services.ClamAV.JobGridPanel.superclass.
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

	onSelectionChange: function(model, records) {
		OMV.Module.Services.ClamAV.JobGridPanel.superclass.
		  onSelectionChange.apply(this, arguments);
		// Process additional buttons
		var tbarRunCtrl = this.queryById(this.getId() + "-run");
		if(records.length <= 0) {
			tbarRunCtrl.disable();
		} else if(records.length == 1) {
			tbarRunCtrl.enable();
		} else {
			tbarRunCtrl.disable();
		}
	},

	onAddButton : function() {
		var wnd = new OMV.Module.Services.ClamAV.JobPropertyDialog({
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

	onEditButton : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelection()[0];
		var wnd = new OMV.Module.Services.ClamAV.JobPropertyDialog({
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
		OMV.Ajax.request({
			  "scope": this,
			  "callback": this.onDeletion,
			  "rpcData": {
				  "service": "ClamAV",
				  "method": "deleteJob",
				  "params": {
					  "uuid": record.get("uuid")
				  }
			  }
		  });
	},

	cbRunBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelection()[0];
		var wnd = Ext.create("OMV.window.Execute", {
			title: _("Execute job"),
			rpcService: "ClamAV",
			rpcMethod: "executeJob",
			rpcParams: { "uuid": record.get("uuid") },
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
OMV.ModuleManager.registerPanel("services", "clamav", {
	cls: OMV.Module.Services.ClamAV.JobGridPanel,
	title: _("Jobs"),
	position: 20
});

/**
 * @class OMV.Module.Services.ClamAV.JobPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.ClamAV.JobPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "ClamAV",
		rpcGetMethod: "getJob",
		rpcSetMethod: "setJob",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add job") : _("Edit job"),
		width: 540,
		height: 375
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.ClamAV.JobPropertyDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.ClamAV.JobPropertyDialog,
  OMV.CfgObjectDialog, {
	getFormItems : function() {
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: true
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			vtype: "comment"
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			fieldLabel: _("Shared folder"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The location of the files to scan.")
			}]
		},{
			xtype: "compositefield",
			fieldLabel: _("Minute"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "minute",
				queryMode: "local",
				store: Array.range(0, 59, 1, true).insert(0, "*"),
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: Ext.Date.format(new Date(), "i"),
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
				value: Ext.Date.format(new Date(), "H"),
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
			name: "onaccess",
			fieldLabel: _("On-access"),
			checked: false,
			hidden: true,
			boxLabel: _("Enable on-access virus scanning"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("Monitor the shared folder and scan new or modified files.")
			}]
		},{
			xtype: "checkbox",
			name: "remove",
			fieldLabel: _("Remove"),
			boxLabel: _("Remove infected files."),
			checked: false
		},{
			xtype: "checkbox",
			name: "multiscan",
			fieldLabel: _("Multiscan"),
			boxLabel: _("Scan the directory contents in parallel using available threads."),
			checked: false
		},{
			xtype: "checkbox",
			name: "verbose",
			fieldLabel: _("Verbose"),
			boxLabel: _("Be verbose."),
			checked: false
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
		}];
	}
});

Ext.ns("OMV.Module.Diagnostics.LogPlugin");

/**
 * @class OMV.Module.Diagnostics.LogPlugin.ClamAV
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'ClamAV' logfile diagnostics plugin
 */
OMV.Module.Diagnostics.LogPlugin.ClamAV = function(config) {
	var initialConfig = {
		stateful: true,
		stateId: "487886d0-97cc-11e1-9f42-000c29f7c0eb",
		columns: [{
			text: _("Date & Time"),
			sortable: true,
			dataIndex: "rownum",
			stateId: "date",
			width: 35,
			renderer: function(value, metaData, record, rowIndex,
			  colIndex, store, view) {
				return record.get("date");
			}
		},{
			text: _("Event"),
			sortable: true,
			dataIndex: "event",
			stateId: "event"
		}],
		rpcParams: { "id": "clamav" },
		rpcFields: [
			{ name: "rownum" },
			{ name: "date" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.ClamAV.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.ClamAV,
  OMV.Module.Diagnostics.LogPlugin, {
});

OMV.PluginManager.register({
	ptype: "log",
	id: "clamav",
	text: _("Antivirus"),
	className: "OMV.Module.Diagnostics.LogPlugin.ClamAV"
});
