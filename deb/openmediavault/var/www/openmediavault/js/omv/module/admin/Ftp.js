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
	text: _("FTP"),
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
			title: _("General settings"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
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
				fieldLabel: _("Max. clients"),
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 5,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Maximum number of simultaneous clients.")
			},{
				xtype: "numberfield",
				name: "maxconnectionsperhost",
				fieldLabel: _("Max. connections per host"),
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 2,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Maximum number of connections per IP (0 = unlimited).")
			},{
				xtype: "numberfield",
				name: "maxloginattempts",
				fieldLabel: _("Max. login attempts"),
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Maximum number of allowed password attempts before disconnection.")
			},{
				xtype: "numberfield",
				name: "timeoutidle",
				fieldLabel: _("Timeout"),
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 1200,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Maximum idle time in seconds. Setting idle timeout to 0 disables the idle timer completely (clients can stay connected for ever, without sending data).")
			},{
				xtype: "checkbox",
				name: "anonymous",
				fieldLabel: _("Anonymous FTP"),
				inputValue: 1,
				boxLabel: _("Enable anonymous FTP"),
				checked: false
			},{
				xtype: "textfield",
				name: "displaylogin",
				fieldLabel: _("Welcome message"),
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "3",
					cols: "65"
				},
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The welcome message which will be displayed to the user when they initially login."),
				anchor: "100%"
			}]
		},{
			xtype: "fieldset",
			title: _("Advanced settings"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "rootlogin",
				fieldLabel: _("Permit root login"),
				inputValue: 1,
				boxLabel: _("Specifies whether it is allowed to login as superuser directly")
			},{
				xtype: "checkbox",
				name: "requirevalidshell",
				fieldLabel: _("Require valid shell"),
				inputValue: 1,
				boxLabel: _("Deny logins which do not have a valid shell"),
				checked: true
			},{
				xtype: "checkbox",
				name: "limittransferrate",
				fieldLabel: _("Bandwidth restriction"),
				inputValue: 1,
				boxLabel: _("Use the following bandwidth restriction:"),
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
							fieldLabel: _("Maximum upload rate (KiB/s)"),
							minValue: 0,
							allowDecimals: false,
							allowNegative: false,
							allowBlank: true,
							readOnly: true,
							plugins: [ OMV.form.plugins.FieldInfo ],
							infoText: _("0 KiB/s means unlimited."),
							value: 0
						},{
							xtype: "numberfield",
							name: "maxdowntransferrate",
							fieldLabel: _("Maximum download rate (KiB/s)"),
							minValue: 0,
							allowDecimals: false,
							allowNegative: false,
							allowBlank: true,
							readOnly: true,
							plugins: [ OMV.form.plugins.FieldInfo ],
							infoText: _("0 KiB/s means unlimited."),
							value: 0
						}]
					}]
				}]
			},{
				xtype: "checkbox",
				name: "usepassiveports",
				fieldLabel: _("Passive FTP"),
				inputValue: 1,
				boxLabel: _("Use the following port range:"),
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
				infoText: _("In some cases you have to specify passive ports range to by-pass firewall limitations. Passive ports restricts the range of ports from which the server will select when sent the PASV command from a client. The server will randomly choose a number from within the specified range until an open port is found. The port range selected must be in the non-privileged range (eg. greater than or equal to 1024). It is strongly recommended that the chosen range be large enough to handle many simultaneous passive connections (for example, 49152-65534, the IANA-registered ephemeral port range).")
			},{
				xtype: "textfield",
				name: "masqueradeaddress",
				fieldLabel: _("Masquerade address"),
				vtype: "domainnameIPv4",
				allowBlank: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("If your host is acting as a NAT gateway or port forwarder for the server, this option is useful in order to allow passive tranfers to work. You have to use your public address and opening the passive ports used on your firewall as well.")
			},{
				xtype: "checkbox",
				name: "allowforeignaddress",
				fieldLabel: _("FXP"),
				inputValue: 1,
				boxLabel: _("Enable FXP protocol"),
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("FXP allows transfers between two remote servers without any file data going to the client asking for the transfer.")
			},{
				xtype: "checkbox",
				name: "allowrestart",
				fieldLabel: _("Resume"),
				inputValue: 1,
				boxLabel: _("Allow clients to resume interrupted uploads and downloads")
			},{
				xtype: "checkbox",
				name: "identlookups",
				fieldLabel: _("Ident protocol"),
				inputValue: 1,
				boxLabel: _("Enable the ident protocol (RFC1413)"),
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("When a client initially connects to the server the ident protocol is used to attempt to identify the remote username.")
			},{
				xtype: "checkbox",
				name: "usereversedns",
				fieldLabel: _("Reverse DNS lookup"),
				inputValue: 1,
				boxLabel: _("Enable reverse DNS lookup"),
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Enable reverse DNS lookup performed on the remote host's IP address for incoming active mode data connections and outgoing passive mode data connections.")
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
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Please check the <a href='http://www.proftpd.org/docs/directives/configuration_full.html' target='_blank'>manual page</a> for more details."),
				anchor: "100%"
			}]
		},{
			xtype: "fieldset",
			title: _("DNS Service Discovery"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "dnssdenable",
				fieldLabel: _("Enable"),
				checked: true,
				inputValue: 1,
				boxLabel: _("Advertise this service via mDNS/DNS-SD.")
			},{
				xtype: "textfield",
				name: "dnssdname",
				fieldLabel: _("Name"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The service name."),
				value: "%h - FTP"
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
	title: _("Settings"),
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
			title: _("General settings"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable SSL/TLS connections"),
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "certificatecombo",
				name: "sslcertificateref",
				hiddenName: "sslcertificateref",
				fieldLabel: _("Certificate"),
				allowBlank: false,
				allowNone: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The SSL certificate.")
			}]
		},{
			xtype: "fieldset",
			title: _("Advanced settings"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "required",
				fieldLabel: _("Required"),
				checked: false,
				inputValue: 1,
				boxLabel: _("This option requires clients to use FTP over TLS when talking to this server.")
			},{
				xtype: "checkbox",
				name: "nocertrequest",
				fieldLabel: _("No certificate request"),
				checked: false,
				inputValue: 1,
				boxLabel: _("This option causes the server to not send a certificate request during a SSL handshake.")
			},{
				xtype: "checkbox",
				name: "nosessionreuserequired",
				fieldLabel: _("No session reuse required"),
				checked: false,
				inputValue: 1,
				boxLabel: _("The requirement that the SSL session from the control connection is reused for data connections is not required.")
			},{
				xtype: "checkbox",
				name: "useimplicitssl",
				fieldLabel: _("Implicit SSL"),
				checked: false,
				inputValue: 1,
				boxLabel: _("This option will handle all connections as if they are SSL connections implicitly.")
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
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Please check the <a href='http://www.proftpd.org/docs/contrib/mod_tls.html' target='_blank'>manual page</a> for more details."),
				anchor: "100%"
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
	title: _("SSL/TLS"),
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
				header: _("Shared folder"),
				sortable: true,
				dataIndex: "sharedfoldername",
				id: "sharedfoldername"
			},{
				header: _("Comment"),
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
			proxy: new OMV.data.DataProxy({
				"service": "FTP",
				"method": "getShareList"
			}),
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
		  "deleteShare", { "uuid": record.get("uuid") });
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "ftp", {
	cls: OMV.Module.Services.FTPSharesGridPanel,
	title: _("Shares"),
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
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add share") : _("Edit share"),
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
			fieldLabel: _("Shared folder"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The location of the files to share.")
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
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
				header: _("Event"),
				sortable: true,
				dataIndex: "event",
				id: "event"
			},{
				header: _("Occurrence"),
				sortable: true,
				dataIndex: "occurrence",
				id: "occurrence"
			},{
				header: _("Time interval"),
				sortable: true,
				dataIndex: "timeinterval",
				id: "timeinterval"
			},{
				header: _("Expire"),
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
			proxy: new OMV.data.DataProxy({
				"service": "FTP",
				"method": "getModBanRuleList"
			}),
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
		  "deleteModBanRule", { "uuid": record.get("uuid") });
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "ftp", {
	cls: OMV.Module.Services.FTPModBanRuleGridPanel,
	title: _("Ban list"),
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
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add rule") : _("Edit rule"),
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
			fieldLabel: _("Event"),
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
			infoText: _("This rule is triggered whenever the selected event directive occurs.")
		},{
			xtype: "numberfield",
			name: "occurrence",
			fieldLabel: _("Occurrence"),
			minValue: 1,
			allowDecimals: false,
			allowNegative: false,
			allowBlank: false,
			value: 2,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("This parameter says that if N occurrences of the event happen within the given time interval, then a ban is automatically added.")
		},{
			xtype: "textfield",
			name: "timeinterval",
			fieldLabel: _("Time interval"),
			allowBlank: false,
			maskRe: /[\d:]/,
			regex: /^\d{2}:\d{2}:\d{2}$/,
			regexText: _("This field must have the format hh:mm:ss"),
			maxLength: 8,
			value: "00:30:00",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("Specifies the time interval in hh:mm:ss in which the given number of occurrences must happen to add the ban.")
		},{
			xtype: "textfield",
			name: "expire",
			fieldLabel: _("Expire"),
			allowBlank: false,
			maskRe: /[\d:]/,
			regex: /^\d{2}:\d{2}:\d{2}$/,
			regexText: _("This field must have the format hh:mm:ss"),
			maxLength: 8,
			value: "00:10:00",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("Specifies the time in hh:mm:ss after which the ban expires.")
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
		title: _("FTP"),
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
		OMV.MessageBox.wait(null, _("Loading ..."));
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
		title: _("FTP"),
		stateId: "c9d06952-00da-11e1-aa29-00221568ca88",
		columns: [{
			header: _("Date & Time"),
			sortable: true,
			dataIndex: "rownum",
			id: "date",
			width: 35,
			renderer: function(val, cell, record, row, col, store) {
				return record.get("date");
			}
		},{
			header: _("Event"),
			sortable: true,
			dataIndex: "event",
			id: "event"
		}],
		rpcArgs: { "id": "proftpd" },
		rpcFields: [
			{ name: "rownum" },
			{ name: "ts" },
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
