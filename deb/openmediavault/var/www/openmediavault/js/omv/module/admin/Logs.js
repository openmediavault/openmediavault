/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
// require("js/omv/MessageBox.js")
// require("js/omv/PluginMgr.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/grid/GridPanel.js")
// require("js/omv/data/Connection.js")
// require("js/omv/data/DataRequest.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Diagnostics");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("diagnostics", "syslogs", {
	text: _("System Logs"),
	icon: "images/logs.png"
});

/**
 * @class OMV.Module.Diagnostics.LogPlugin
 * @derived Ext.util.Observable
 */
OMV.Module.Diagnostics.LogPlugin = function(config) {
	var initialConfig = {
		title: "",
		stateId: "162f0b7b-8d3d-4401-98c3-cac33d909b58",
		columns: [],
		rpcService: "LogFile",
		rpcGetMethod: "getList",
		rpcClearMethod: "clear",
		rpcDownloadMethod: "getContent",
		rpcArgs: undefined,
		rpcFields: [],
		rpcRemoteSort: true
	};
	Ext.apply(this, config, initialConfig);
	OMV.Module.Diagnostics.LogPlugin.superclass.constructor.call(this);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin, Ext.util.Observable,  {
	getTitle : function() {
		return this.title;
	}
});

/**
 * @class OMV.Module.Diagnostics.LogPlugin.Syslog
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'Syslog' plugin
 */
OMV.Module.Diagnostics.LogPlugin.Syslog = function(config) {
	var initialConfig = {
		title: _("Syslog"),
		stateId: "8779c5e8-cf69-441a-8c9f-93259362f2fb",
		columns: [{
			header: _("Date & Time"),
			sortable: true,
			dataIndex: "rownum",
			id: "date",
			width: 35,
			renderer: function(val, cell, record, row, col, store) {
				return OMV.util.Format.localeTime(record.get("date"));
			}
		},{
			header: _("User"),
			sortable: true,
			dataIndex: "user",
			id: "user",
			width: 20
		},{
			header: _("Event"),
			sortable: true,
			dataIndex: "event",
			id: "event",
			renderer: OMV.util.Format.whitespaceRenderer()
		}],
		rpcArgs: { "id": "syslog" },
		rpcFields: [
			{ name: "rownum" },
			{ name: "date" },
			{ name: "user" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.Syslog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.Syslog,
  OMV.Module.Diagnostics.LogPlugin, {
});
OMV.preg("log", "syslog", OMV.Module.Diagnostics.LogPlugin.Syslog);

/**
 * @class OMV.Module.Diagnostics.LogPlugin.Daemon
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'Daemon' plugin
 */
OMV.Module.Diagnostics.LogPlugin.Daemon = function(config) {
	var initialConfig = {
		title: _("Daemon"),
		stateId: "b3f076a3-3a5a-4e4d-863c-cdec5dee5a07",
		columns: [{
			header: _("Date & Time"),
			sortable: true,
			dataIndex: "rownum",
			id: "date",
			width: 35,
			renderer: function(val, cell, record, row, col, store) {
				return OMV.util.Format.localeTime(record.get("date"));
			}
		},{
			header: _("User"),
			sortable: true,
			dataIndex: "user",
			id: "user",
			width: 20
		},{
			header: _("Event"),
			sortable: true,
			dataIndex: "event",
			id: "event"
		}],
		rpcArgs: { "id": "daemon" },
		rpcFields: [
			{ name: "rownum" },
			{ name: "date" },
			{ name: "user" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.Daemon.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.Daemon,
  OMV.Module.Diagnostics.LogPlugin, {
});
OMV.preg("log", "daemon", OMV.Module.Diagnostics.LogPlugin.Daemon);

/**
 * @class OMV.Module.Diagnostics.LogPlugin.Auth
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'Daemon' plugin
 */
OMV.Module.Diagnostics.LogPlugin.Auth = function(config) {
	var initialConfig = {
		title: _("Authentication"),
		stateId: "bd8370a3-fb43-42f4-b036-3e8aaa86d72e",
		columns: [{
			header: _("Date & Time"),
			sortable: true,
			dataIndex: "rownum",
			id: "date",
			width: 35,
			renderer: function(val, cell, record, row, col, store) {
				return OMV.util.Format.localeTime(record.get("date"));
			}
		},{
			header: _("User"),
			sortable: true,
			dataIndex: "user",
			id: "user",
			width: 20
		},{
			header: _("Event"),
			sortable: true,
			dataIndex: "event",
			id: "event"
		}],
		rpcArgs: { "id": "auth" },
		rpcFields: [
			{ name: "rownum" },
			{ name: "date" },
			{ name: "user" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.Auth.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.Auth,
  OMV.Module.Diagnostics.LogPlugin, {
});
OMV.preg("log", "auth", OMV.Module.Diagnostics.LogPlugin.Auth);

/**
 * @class OMV.Module.Diagnostics.LogPlugin.Messages
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'Messages' plugin
 */
OMV.Module.Diagnostics.LogPlugin.Messages = function(config) {
	var initialConfig = {
		title: _("Messages"),
		stateId: "5c3900b7-7ab4-400c-94f0-db259cc94bed",
		columns: [{
			header: _("Date & Time"),
			sortable: true,
			dataIndex: "rownum",
			id: "date",
			width: 35,
			renderer: function(val, cell, record, row, col, store) {
				return OMV.util.Format.localeTime(record.get("date"));
			}
		},{
			header: _("User"),
			sortable: true,
			dataIndex: "user",
			id: "user",
			width: 20
		},{
			header: _("Event"),
			sortable: true,
			dataIndex: "event",
			id: "event"
		}],
		rpcArgs: { "id": "messages" },
		rpcFields: [
			{ name: "rownum" },
			{ name: "date" },
			{ name: "user" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.Messages.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.Messages,
  OMV.Module.Diagnostics.LogPlugin, {
});
OMV.preg("log", "messages", OMV.Module.Diagnostics.LogPlugin.Messages);

/**
 * @class OMV.Module.Diagnostics.LogGridPanel
 * @derived OMV.grid.GridPanel
 */
OMV.Module.Diagnostics.LogGridPanel = function(config) {
	var initialConfig = {
		logPlugins: []
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogGridPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogGridPanel, OMV.grid.GridPanel, {
	initComponent : function() {
		// Initialize log plugins
		var storeData = [];
		var plugins = OMV.PluginMgr.get("log");
		for (var k = 0; k < plugins.length; k++) {
			// Create plugin instance
			var plugin = new plugins[k].cls();
			Ext.apply(plugin, {
				ptype: plugins[k].ptype
			});
			// Append plugin to internal list of log plugins
			this.logPlugins[plugin.ptype] = plugin;
			// Add plugin to combobox store data
			storeData.push([ plugin.ptype, plugin.getTitle() ]);
		}
		// Display the 'Syslog' per default
		this.plugin = this.logPlugins["syslog"];
		// Initialize combobox containing list of registered log plugins
		this.logCtrl = new Ext.form.ComboBox({
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: storeData,
				sortInfo: {
					field: "text",
					direction: "ASC"
				}
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: this.plugin.ptype,
			listeners: {
				scope: this,
				select: this.cbSelectHdl
			}
		});
		// Initialize column model
		this.stateId = this.plugin.stateId;
		this.colModel = new Ext.grid.ColumnModel({
			columns: this.plugin.columns
		});
		// Initialize store
		this.store = new OMV.data.Store({
			autoLoad: {
				params: {
					start: 0,
					limit: 50
				}
			},
			remoteSort: this.plugin.rpcRemoteSort,
			sortInfo: {
				field: "rownum",
				direction: "DESC"
			},
			proxy: new OMV.data.DataProxy({
				"service": this.plugin.rpcService,
				"method": this.plugin.rpcGetMethod,
				"extraParams": this.plugin.rpcArgs
			}),
			reader: new Ext.data.JsonReader({
				totalProperty: "total",
				root: "data",
				fields: this.plugin.rpcFields
			})
		});
		// Initialize top toolbar
		this.tbar = new Ext.Toolbar({
			items: [
				this.logCtrl,
			{
				id: "clear",
				xtype: "button",
				text: _("Clear"),
				icon: "images/trashcan.png",
				handler: this.cbClearBtnHdl,
				scope: this
			},{
				id: "download",
				xtype: "button",
				text: _("Download"),
				icon: "images/save.png",
				handler: this.cbDownloadBtnHdl,
				scope: this
			}]
		});
		// Add bottom toolbar
		this.bbar = new Ext.PagingToolbar({
			store: this.store,
			pageSize: 50,
			displayInfo: true,
			displayMsg: 'Displaying topics {0} - {1} of {2}',
			emptyMsg: _("No log entries to display")
		});
		OMV.Module.Diagnostics.LogGridPanel.superclass.initComponent.apply(
		  this, arguments);
	},

	/**
		* @method doLoad
		* Reload the grid content.
		*/
	doReload : function() {
		this.store.reload();
	},

	/**
	 * @method cbClearBtnHdl
	 * Handler that is called when the 'Clear' button in the top toolbar
	 * is pressed.
	 */
	cbClearBtnHdl : function() {
		var msg = _("Do you really want to clear the log file?");
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: msg,
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if (answer == "no")
					return;
				OMV.Ajax.request(function(id, response, error) {
					if (error === null) {
						this.doReload();
					} else {
						OMV.MessageBox.error(null, error);
					}
				}, this, this.plugin.rpcService, this.plugin.rpcClearMethod,
				this.plugin.rpcArgs);
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	},

	/**
	 * @method cbDownloadBtnHdl
	 * Handler that is called when the 'Download' button in the top toolbar
	 * is pressed.
	 */
	cbDownloadBtnHdl : function() {
		OMV.Download.request(this.plugin.rpcService,
		  this.plugin.rpcDownloadMethod, this.plugin.rpcArgs);
	},

	/**
	 * @method cbSelectHdl
	 * Handler that is called when a comboxbox entry has been selected
	 * in the top toolbar.
	 */
	cbSelectHdl : function(combo, record, index) {
		var ptype = record.get("value");
		this.plugin = this.logPlugins[ptype];
		// Initialize new column model
		var colModel = new Ext.grid.ColumnModel({
			columns: this.plugin.columns
		});
		// Initialize new store
		var store = new OMV.data.Store({
			autoLoad: {
				params: {
					start: 0,
					limit: 50
				}
			},
			remoteSort: this.plugin.rpcRemoteSort,
			sortInfo: {
				field: "rownum",
				direction: "DESC"
			},
			proxy: new OMV.data.DataProxy({
				"service": this.plugin.rpcService,
				"method": this.plugin.rpcGetMethod,
				"extraParams": this.plugin.rpcArgs
			}),
			reader: new Ext.data.JsonReader({
				totalProperty: "total",
				root: "data",
				fields: this.plugin.rpcFields
			})
		});
		// Reconfigure grid to use new store and colum model
		this.stateId = this.plugin.stateId;
		this.reconfigure(store, colModel);
		// Update paging toolbar
		this.getBottomToolbar().bindStore(this.store);
	}
});
OMV.NavigationPanelMgr.registerPanel("diagnostics", "syslogs", {
	cls: OMV.Module.Diagnostics.LogGridPanel,
	title: _("Logs"),
	position: 10
});

/**
 * @class OMV.Module.Diagnostics.LogSettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Diagnostics.LogSettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "Syslog",
 		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogSettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogSettingsPanel, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Diagnostics.LogSettingsPanel.superclass.initComponent.
		  apply(this, arguments);
		this.on("load", this._updateFormFields, this);
	},

	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("Remote syslog"),
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false,
				listeners: {
					check: this._updateFormFields.createDelegate(this)
				}
			},{
				xtype: "textfield",
				name: "host",
				fieldLabel: _("Host"),
				allowBlank: true,
				vtype: "IPv4",
				value: ""
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				allowBlank: true,
				vtype: "port",
				value: 514
			},{
				xtype: "combo",
				name: "protocol",
				hiddenName: "protocol",
				fieldLabel: _("Protocol"),
				emptyText: _("Select a protocol ..."),
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ "tcp","TCP" ],
						[ "udp","UDP" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "udp"
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields
	 * depending on whether remote syslog'ing is enabled or disabled.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("enable");
		if (field) {
			var fields = [ "host", "port", "protocol" ];
			var readOnly = (false === field.getValue());
			for (var i = 0; i < fields.length; i++) {
				field = this.findFormField(fields[i]);
				if (field) {
					field.allowBlank = readOnly;
					field.setReadOnly(readOnly);
				}
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("diagnostics", "syslogs", {
	cls: OMV.Module.Diagnostics.LogSettingsPanel,
	title: _("Settings"),
	position: 20
});
