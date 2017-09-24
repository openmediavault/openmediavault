/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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
// require("js/omv/form/field/SslCertificateComboBox.js")

/**
 * @class OMV.module.admin.system.general.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.system.general.Settings", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.form.field.SslCertificateComboBox"
	],

	rpcService: "WebGui",
	rpcGetMethod: "getSettings",
	rpcSetMethod: "setSettings",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: [
				"sslport",
				"forcesslonly",
				"sslcertificateref"
			],
			conditions: [
				{ name: "enablessl", value: true }
			],
			properties: [
				"!allowBlank",
				"!readOnly"
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
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 80
			},{
				xtype: "combo",
				name: "timeout",
				fieldLabel: _("Session timeout"),
				queryMode: "local",
				store: [
					[ 0, _("Disabled") ],
					[ 1, _("1 min") ],
					[ 2, _("2 min") ],
					[ 3, _("3 min") ],
					[ 4, _("4 min") ],
					[ 5, _("5 min") ],
					[ 10, _("10 min") ],
					[ 15, _("15 min") ],
					[ 30, _("30 min") ],
					[ 60, _("60 min") ],
					[ 1440, _("1 d") ]
				],
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 5,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The session timeout time in minutes.")
				}]
			}]
		},{
			xtype: "fieldset",
			title: _("Secure connection"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enablessl",
				fieldLabel: _("Enable SSL/TLS"),
				checked: false,
				boxLabel: _("Enable secure connection.")
			},{
				xtype: "sslcertificatecombo",
				name: "sslcertificateref",
				fieldLabel: _("Certificate"),
				allowNone: true,
				allowBlank: true,
				readOnly: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The SSL certificate.")
				}]
			},{
				xtype: "numberfield",
				name: "sslport",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				readOnly: true,
				value: 443
			},{
				xtype: "checkbox",
				name: "forcesslonly",
				fieldLabel: _("Force SSL/TLS"),
				checked: false,
				readOnly: true,
				boxLabel: _("Force secure connection only.")
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/system/general",
	text: _("Web Administration"),
	position: 10,
	className: "OMV.module.admin.system.general.Settings"
});
