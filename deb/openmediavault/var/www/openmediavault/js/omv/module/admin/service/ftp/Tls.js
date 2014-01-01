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
// require("js/omv/form/field/CertificateComboBox.js")

/**
 * @class OMV.module.admin.service.ftp.Tls
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.ftp.Tls", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.form.field.CertificateComboBox"
	],

	rpcService: "FTP",
	rpcGetMethod: "getModTLSSettings",
	rpcSetMethod: "setModTLSSettings",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: "sslcertificateref",
			conditions: [
				{ name: "enable", value: true }
			],
			properties: [
				"!allowBlank",
				"!readOnly"
			]
		}]
	}],

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
				checked: false,
				boxLabel: _("Enable SSL/TLS connections")
			},{
				xtype: "certificatecombo",
				name: "sslcertificateref",
				fieldLabel: _("Certificate"),
				allowBlank: false,
				allowNone: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The SSL certificate.")
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
				name: "required",
				fieldLabel: _("Required"),
				checked: false,
				boxLabel: _("This option requires clients to use FTP over TLS when talking to this server.")
			},{
				xtype: "checkbox",
				name: "nocertrequest",
				fieldLabel: _("No certificate request"),
				checked: false,
				boxLabel: _("This option causes the server to not send a certificate request during a SSL handshake.")
			},{
				xtype: "checkbox",
				name: "nosessionreuserequired",
				fieldLabel: _("No session reuse required"),
				checked: false,
				boxLabel: _("The requirement that the SSL session from the control connection is reused for data connections is not required.")
			},{
				xtype: "checkbox",
				name: "useimplicitssl",
				fieldLabel: _("Implicit SSL"),
				checked: false,
				boxLabel: _("This option will handle all connections as if they are SSL connections implicitly.")
			},{
				xtype: "textarea",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Please check the <a href='http://www.proftpd.org/docs/contrib/mod_tls.html' target='_blank'>manual page</a> for more details."),
				}]
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "tls",
	path: "/service/ftp",
	text: _("SSL/TLS"),
	position: 20,
	className: "OMV.module.admin.service.ftp.Tls"
});
