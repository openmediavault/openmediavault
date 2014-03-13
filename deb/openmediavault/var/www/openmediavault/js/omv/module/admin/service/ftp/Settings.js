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
// require("js/omv/workspace/form/Panel.js")

/**
 * @class OMV.module.admin.service.ftp.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.ftp.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "FTP",
	rpcGetMethod: "getSettings",
	rpcSetMethod: "setSettings",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: [
				"minpassiveports",
				"maxpassiveports"
			],
			conditions: [
				{ name: "usepassiveports", value: true }
			],
			properties: [
				"!readOnly",
				"!allowBlank"
			]
		},{
			name: [
				"maxuptransferrate",
				"maxdowntransferrate"
			],
			conditions: [
				{ name: "limittransferrate", value: true }
			],
			properties: [
				"!readOnly",
				"!allowBlank"
			]
		}]
	}],

	getFormItems: function() {
		var me = this;
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
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 21
			},{
				xtype: "numberfield",
				name: "maxclients",
				fieldLabel: _("Max. clients"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 5,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Maximum number of simultaneous clients.")
				}]
			},{
				xtype: "numberfield",
				name: "maxconnectionsperhost",
				fieldLabel: _("Max. connections per host"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 2,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Maximum number of connections per IP (0 = unlimited).")
				}]
			},{
				xtype: "numberfield",
				name: "maxloginattempts",
				fieldLabel: _("Max. login attempts"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 1,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Maximum number of allowed password attempts before disconnection.")
				}]
			},{
				xtype: "numberfield",
				name: "timeoutidle",
				fieldLabel: _("Timeout"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 1200,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Maximum idle time in seconds. Setting idle timeout to 0 disables the idle timer completely (clients can stay connected for ever, without sending data).")
				}]
			},{
				xtype: "checkbox",
				name: "anonymous",
				fieldLabel: _("Anonymous FTP"),
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
				plugins: [{
					ptype: "fieldinfo",
					text: _("The welcome message which will be displayed to the user when they initially login."),
				}]
			}]
		},{
			xtype: "fieldset",
			title: _("Advanced settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "rootlogin",
				fieldLabel: _("Permit root login"),
				boxLabel: _("Specifies whether it is allowed to login as superuser directly")
			},{
				xtype: "checkbox",
				name: "requirevalidshell",
				fieldLabel: _("Require valid shell"),
				boxLabel: _("Deny logins which do not have a valid shell"),
				checked: true
			},{
				xtype: "checkbox",
				name: "limittransferrate",
				fieldLabel: _("Bandwidth restriction"),
				boxLabel: _("Use the following bandwidth restriction:")
			},{
				xtype: "fieldcontainer",
				fieldLabel: "&nbsp;",
				layout: "anchor",
				fieldDefaults: {
					labelWith: 180,
					readOnly: me.readOnly,
					anchor: "100%"
				},
				items: [{
					xtype: "numberfield",
					name: "maxuptransferrate",
					fieldLabel: _("Maximum upload rate (KiB/s)"),
					minValue: 0,
					allowDecimals: false,
					allowBlank: true,
					readOnly: true,
					plugins: [{
						ptype: "fieldinfo",
						text: _("0 KiB/s means unlimited."),
					}],
					value: 0
				},{
					xtype: "numberfield",
					name: "maxdowntransferrate",
					fieldLabel: _("Maximum download rate (KiB/s)"),
					minValue: 0,
					allowDecimals: false,
					allowBlank: true,
					readOnly: true,
					plugins: [{
						ptype: "fieldinfo",
						text: _("0 KiB/s means unlimited."),
					}],
					value: 0
				}]
			},{
				xtype: "checkbox",
				name: "usepassiveports",
				fieldLabel: _("Passive FTP"),
				boxLabel: _("Use the following port range:")
			},{
				xtype: "fieldcontainer",
				fieldLabel: "&nbsp;",
				layout: "hbox",
				items: [{
					xtype: "numberfield",
					name: "minpassiveports",
					vtype: "port",
					minValue: 1025,
					maxValue: 65535,
					allowDecimals: false,
					allowBlank: true,
					readOnly: true,
					flex: 1,
					value: 49152,
				},{
					xtype: "displayfield",
					width: 14,
					value: "-",
					style: "text-align:center"
				},{
					xtype: "numberfield",
					name: "maxpassiveports",
					vtype: "port",
					minValue: 1025,
					maxValue: 65535,
					allowDecimals: false,
					allowBlank: true,
					readOnly: true,
					flex: 1,
					value: 65534
				}],
				plugins: [{
					ptype: "fieldinfo",
					text: _("In some cases you have to specify passive ports range to by-pass firewall limitations. Passive ports restricts the range of ports from which the server will select when sent the PASV command from a client. The server will randomly choose a number from within the specified range until an open port is found. The port range selected must be in the non-privileged range (eg. greater than or equal to 1024). It is strongly recommended that the chosen range be large enough to handle many simultaneous passive connections (for example, 49152-65534, the IANA-registered ephemeral port range).")
				}]
			},{
				xtype: "textfield",
				name: "masqueradeaddress",
				fieldLabel: _("Masquerade address"),
				vtype: "domainnameIP",
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("If your host is acting as a NAT gateway or port forwarder for the server, this option is useful in order to allow passive tranfers to work. You have to use your public address and opening the passive ports used on your firewall as well.")
				}]
			},{
				xtype: "numberfield",
				name: "dynmasqrefresh",
				fieldLabel: " ",
				minValue: 0,
				allowDecimals: false,
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the amount of time, in seconds, between checking and updating the masquerade address by resolving the IP address. Set this value to 0 to disable this option."),
				}],
				value: 0
			},{
				xtype: "checkbox",
				name: "allowforeignaddress",
				fieldLabel: _("FXP"),
				boxLabel: _("Enable FXP protocol"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("FXP allows transfers between two remote servers without any file data going to the client asking for the transfer.")
				}]
			},{
				xtype: "checkbox",
				name: "allowrestart",
				fieldLabel: _("Resume"),
				boxLabel: _("Allow clients to resume interrupted uploads and downloads")
			},{
				xtype: "checkbox",
				name: "identlookups",
				fieldLabel: _("Ident protocol"),
				boxLabel: _("Enable the ident protocol (RFC1413)"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("When a client initially connects to the server the ident protocol is used to attempt to identify the remote username.")
				}]
			},{
				xtype: "checkbox",
				name: "usereversedns",
				fieldLabel: _("Reverse DNS lookup"),
				boxLabel: _("Enable reverse DNS lookup"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("Enable reverse DNS lookup performed on the remote host's IP address for incoming active mode data connections and outgoing passive mode data connections.")
				}]
			},{
				xtype: "checkbox",
				name: "transferlog",
				fieldLabel: _("Transfer log"),
				boxLabel: _("Enable transfer log")
			},{
				xtype: "textarea",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Please check the <a href='http://www.proftpd.org/docs/directives/configuration_full.html' target='_blank'>manual page</a> for more details."),
				}]
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/ftp",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.ftp.Settings"
});
