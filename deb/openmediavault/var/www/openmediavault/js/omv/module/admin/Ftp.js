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
// require("js/omv/PluginMgr.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/DiagPanel.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/CertificateComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/module/admin/Logs.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "ftp", {
	text: "FTP",
	icon: "images/ftp.png"
});

/**
 * @class OMV.Module.Services.FTPSettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.FTPSettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "FTP",
 		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.FTPSettingsPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Services.FTPSettingsPanel, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.FTPSettingsPanel.superclass.initComponent.apply(
		  this, arguments);
		this.on("load", this._updateFormFields, this);
	},

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
				value: 21
			},{
				xtype: "numberfield",
				name: "maxclients",
				fieldLabel: "Max. clients",
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 5,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Maximum number of simultaneous clients."
			},{
				xtype: "numberfield",
				name: "maxconnectionsperhost",
				fieldLabel: "Max. connections per host",
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 2,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Maximum number of connections per IP (0 = unlimited)."
			},{
				xtype: "numberfield",
				name: "maxloginattempts",
				fieldLabel: "Max. login attempts",
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Maximum number of allowed password attempts " +
				  "before disconnection."
			},{
				xtype: "numberfield",
				name: "timeoutidle",
				fieldLabel: "Timeout",
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 1200,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Maximum idle time in seconds. Setting idle " +
				  "timeout to 0 disables the idle timer completely (clients " +
				  "can stay connected for ever, without sending data)."
			},{
				xtype: "checkbox",
				name: "anonymous",
				fieldLabel: "Anonymous FTP",
				inputValue: 1,
				boxLabel: "Enable anonymous FTP",
				checked: false
			},{
				xtype: "textfield",
				name: "displaylogin",
				fieldLabel: "Welcome message",
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "3",
					cols: "65"
				},
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The welcome message which will be displayed to " +
				  "the user when they initially login."
			}]
		},{
			xtype: "fieldset",
			title: "Advanced settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "rootlogin",
				fieldLabel: "Permit root login",
				inputValue: 1,
				boxLabel: "Specifies whether it is allowed to login as " +
				  "superuser directly"
			},{
				xtype: "checkbox",
				name: "requirevalidshell",
				fieldLabel: "Require valid shell",
				inputValue: 1,
				boxLabel: "Deny logins which do not have a valid shell",
				checked: true
			},{
				xtype: "checkbox",
				name: "limittransferrate",
				fieldLabel: "Bandwidth restriction",
				inputValue: 1,
				boxLabel: "Use the following bandwidth restriction:",
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "container",
				fieldLabel: " ",
				items: [{
					layout: "column",
					defaults: {
						layout: "form",
						labelWidth: 180
					},
					items: [{
						defaults: {
							labelSeparator: ""
						},
						items: [{
							xtype: "numberfield",
							name: "maxuptransferrate",
							fieldLabel: "Maximum upload rate (KiB/s)",
							minValue: 0,
							allowDecimals: false,
							allowNegative: false,
							allowBlank: true,
							readOnly: true,
							plugins: [ OMV.form.plugins.FieldInfo ],
							infoText: "0 KiB/s means unlimited.",
							value: 0
						},{
							xtype: "numberfield",
							name: "maxdowntransferrate",
							fieldLabel: "Maximum download rate (KiB/s)",
							minValue: 0,
							allowDecimals: false,
							allowNegative: false,
							allowBlank: true,
							readOnly: true,
							plugins: [ OMV.form.plugins.FieldInfo ],
							infoText: "0 KiB/s means unlimited.",
							value: 0
						}]
					}]
				}]
			},{
				xtype: "checkbox",
				name: "usepassiveports",
				fieldLabel: "Passive FTP",
				inputValue: 1,
				boxLabel: "Use the following port range:",
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "container",
				fieldLabel: " ",
				items: [{
					layout: "column",
					items: [{
						items: [{
							xtype: "numberfield",
							name: "minpassiveports",
							vtype: "port",
							minValue: 1025,
							maxValue: 65535,
							allowDecimals: false,
							allowNegative: false,
							allowBlank: true,
							readOnly: true,
							value: 49152
						}]
					},{
						items: [{
							xtype: "displayfield",
							width: 14,
							value: "-",
							style: "text-align:center"
						}]
					},{
						items: [{
							xtype: "numberfield",
							name: "maxpassiveports",
							vtype: "port",
							minValue: 1025,
							maxValue: 65535,
							allowDecimals: false,
							allowNegative: false,
							allowBlank: true,
							readOnly: true,
							value: 65534
						}]
					}]
				}],
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "In some cases you have to specify passive ports " +
				  "range to by-pass firewall limitations. Passive ports " +
				  "restricts the range of ports from which the server will " +
				  "select when sent the PASV command from a client. The " +
				  "server will randomly choose a number from within the " +
				  "specified range until an open port is found. The port " +
				  "range selected must be in the non-privileged range (eg. " +
				  "greater than or equal to 1024). It is strongly " +
				  "recommended that the chosen range be large enough to " +
				  "handle many simultaneous passive connections (for " +
				  "example, 49152-65534, the IANA-registered ephemeral port " +
				  "range)."
			},{
				xtype: "textfield",
				name: "masqueradeaddress",
				fieldLabel: "Masquerade address",
				vtype: "hostnameIPv4",
				allowBlank: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "If your host is acting as a NAT gateway or port " +
				  "forwarder for the server, this option is useful in order " +
				  "to allow passive tranfers to work. You have to use your " +
				  "public address and opening the passive ports used on your " +
				  "firewall as well."
			},{
				xtype: "checkbox",
				name: "allowforeignaddress",
				fieldLabel: "FXP",
				inputValue: 1,
				boxLabel: "Enable FXP protocol",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "FXP allows transfers between two remote servers " +
				  "without any file data going to the client asking for the " +
				  "transfer."
			},{
				xtype: "checkbox",
				name: "allowrestart",
				fieldLabel: "Resume",
				inputValue: 1,
				boxLabel: "Allow clients to resume interrupted uploads " +
				  "and downloads"
			},{
				xtype: "checkbox",
				name: "identlookups",
				fieldLabel: "Ident protocol",
				inputValue: 1,
				boxLabel: "Enable the ident protocol (RFC1413)",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "When a client initially connects to the server " +
				  "the ident protocol is used to attempt to identify the " +
				  "remote username."
			},{
				xtype: "checkbox",
				name: "usereversedns",
				fieldLabel: "Reverse DNS lookup",
				inputValue: 1,
				boxLabel: "Enable reverse DNS lookup",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Enable reverse DNS lookup performed on the " +
				  "remote host's IP address for incoming active mode data " +
				  "connections and outgoing passive mode data connections."
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
				},
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Please check the <a href='http://www.proftpd.org/docs/directives/configuration_full.html' target='_blank'>manual page</a> for more details."
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("usepassiveports");
		var checked = field.checked;
		var fields = [ "minpassiveports", "maxpassiveports" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
		field = this.findFormField("limittransferrate");
		checked = field.checked;
		fields = [ "maxuptransferrate", "maxdowntransferrate" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "ftp", {
	cls: OMV.Module.Services.FTPSettingsPanel,
	title: "Settings",
	position: 10
});

/**
 * @class OMV.Module.Services.FTPModTLSPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.FTPModTLSPanel = function(config) {
	var initialConfig = {
		rpcService: "FTP",
 		rpcGetMethod: "getModTLSSettings",
		rpcSetMethod: "setModTLSSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.FTPModTLSPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Services.FTPModTLSPanel, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.FTPModTLSPanel.superclass.initComponent.apply(
		  this, arguments);
		this.on("load", this._updateFormFields, this);
	},

	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "Settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable SSL/TLS connections",
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "checkbox",
				name: "required",
				fieldLabel: "Required",
				checked: false,
				inputValue: 1,
				boxLabel: "Are clients required to use FTP over TLS when " +
				  "talking to this server?"
			},{
				xtype: "certificatecombo",
				name: "sslcertificateref",
				hiddenName: "sslcertificateref",
				fieldLabel: "Certificate",
				allowBlank: false,
				allowNone: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The SSL certificate."
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("enable");
		var checked = field.checked;
		var fields = [ "sslcertificateref" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "ftp", {
	cls: OMV.Module.Services.FTPModTLSPanel,
	title: "SSL/TLS",
	position: 20
});

/**
 * @class OMV.Module.Services.FTPSharesGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.FTPSharesGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "9889057b-b1c0-4c48-a4c1-8c8b4fb54d7b",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Shared folder",
				sortable: true,
				dataIndex: "sharedfoldername",
				id: "sharedfoldername"
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.FTPSharesGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.FTPSharesGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("FTP", "getShareList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "sharedfoldername" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Services.FTPSharesGridPanel.superclass.initComponent.apply(
		  this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.FTPSharePropertyDialog({
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
		var wnd = new OMV.Module.Services.FTPSharePropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "FTP",
			"deleteShare", [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "ftp", {
	cls: OMV.Module.Services.FTPSharesGridPanel,
	title: "Shares",
	position: 40
});

/**
 * @class OMV.Module.Services.FTPSharePropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.FTPSharePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "FTP",
		rpcGetMethod: "getShare",
		rpcSetMethod: "setShare",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " share",
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.FTPSharePropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.FTPSharePropertyDialog, OMV.CfgObjectDialog, {
	initComponent : function() {
		OMV.Module.Services.FTPSharePropertyDialog.superclass.initComponent.
		  apply(this, arguments);
		this.on("load", this._updateFormFields, this);
	},

	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			hiddenName: "sharedfolderref",
			fieldLabel: "Shared folder",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The location of the files to share."
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true
		},{
			xtype: "hidden",
			name: "mntentref",
			value: OMV.UUID_UNDEFINED
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
 * @class OMV.Module.Services.FTPModBanRuleGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.FTPModBanRuleGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "b145dd0c-8fe8-4570-947f-e4c0ee40b900",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Event",
				sortable: true,
				dataIndex: "event",
				id: "event"
			},{
				header: "Occurrence",
				sortable: true,
				dataIndex: "occurrence",
				id: "occurrence"
			},{
				header: "Time interval",
				sortable: true,
				dataIndex: "timeinterval",
				id: "timeinterval"
			},{
				header: "Expire",
				sortable: true,
				dataIndex: "expire",
				id: "expire"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.FTPModBanRuleGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.FTPModBanRuleGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("FTP", "getModBanRuleList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "event" },
					{ name: "occurrence" },
					{ name: "timeinterval" },
					{ name: "expire" }
    			]
			})
		});
		OMV.Module.Services.FTPModBanRuleGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.FTPModBanRulePropertyDialog({
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
		var wnd = new OMV.Module.Services.FTPModBanRulePropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "FTP",
			"deleteModBanRule", [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "ftp", {
	cls: OMV.Module.Services.FTPModBanRuleGridPanel,
	title: "Ban list",
	position: 30
});

/**
 * @class OMV.Module.Services.FTPModBanRulePropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.FTPModBanRulePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "FTP",
		rpcGetMethod: "getModBanRule",
		rpcSetMethod: "setModBanRule",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " rule",
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.FTPModBanRulePropertyDialog.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.FTPModBanRulePropertyDialog,
  OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "event",
			hiddenName: "event",
			fieldLabel: "Event",
			mode: "local",
			store: [
				"AnonRejectPasswords",
				"ClientConnectRate",
				"MaxClientsPerClass",
				"MaxClientsPerHost",
				"MaxClientsPerUser",
				"MaxConnectionsPerHost",
				"MaxHostsPerUser",
				"MaxLoginAttempts",
				"TimeoutIdle",
				"TimeoutNoTransfer"
			],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "MaxConnectionsPerHost",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This rule is triggered whenever the selected event directive occurs."
		},{
			xtype: "numberfield",
			name: "occurrence",
			fieldLabel: "Occurrence",
			minValue: 1,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			value: 2,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This parameter says that if N occurrences of the event happen within the given time interval, then a ban is automatically added."
		},{
			xtype: "textfield",
			name: "timeinterval",
			fieldLabel: "Time interval",
			allowBlank: false,
			maskRe: /[\d:]/,
			regex: /^\d{2}:\d{2}:\d{2}$/,
			maxLength: 8,
			value: "00:30:00",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Specifies the time interval in hh:mm:ss in which the given number of occurrences must happen to add the ban."
		},{
			xtype: "textfield",
			name: "expire",
			fieldLabel: "Expire",
			allowBlank: false,
			maskRe: /[\d:]/,
			regex: /^\d{2}:\d{2}:\d{2}$/,
			maxLength: 8,
			value: "00:10:00",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Specifies the time in hh:mm:ss after which the ban expires."
		}];
	}
});

/**
 * @class OMV.Module.Services.FTPDiagPanel
 * @derived OMV.DiagPanel
 * Class that implements the 'FTP' service page.
 */
OMV.Module.Services.FTPDiagPanel = function(config) {
	var initialConfig = {
		title: "FTP",
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
	OMV.Module.Services.FTPDiagPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.FTPDiagPanel, OMV.DiagPanel, {
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
		  }, this, "FTP", "getStats");
	}
});
OMV.preg("sysinfo", "service", OMV.Module.Services.FTPDiagPanel);

/**
 * @class OMV.Module.Diagnostics.LogPlugin.FTP
 * @derived OMV.Module.Diagnostics.LogPlugin
 * Class that implements the 'FTP' log file diagnostics plugin
 */
OMV.Module.Diagnostics.LogPlugin.FTP = function(config) {
	var initialConfig = {
		title: "FTP",
		stateId: "c9d06952-00da-11e1-aa29-00221568ca88",
		columns: [{
			header: "Date & Time",
			sortable: true,
			dataIndex: "date",
			id: "date",
			width: 35,
			renderer: OMV.util.Format.localeTimeRenderer()
		},{
			header: "Event",
			sortable: true,
			dataIndex: "event",
			id: "event"
		}],
		rpcArgs: "proftpd",
		rpcFields: [
			{ name: "date" },
			{ name: "event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.FTP.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.LogPlugin.FTP,
  OMV.Module.Diagnostics.LogPlugin, {
});
OMV.preg("log", "proftpd", OMV.Module.Diagnostics.LogPlugin.FTP);
