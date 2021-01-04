/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
// require("js/omv/form/field/Password.js")

/**
 * @class OMV.module.admin.system.network.Proxy
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.system.network.Proxy", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.form.field.Password",
	],

	rpcService: "Network",
	rpcGetMethod: "getProxy",
	rpcSetMethod: "setProxy",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: "httphost",
			conditions: [
				{ name: "httpenable", value: true }
			],
			properties: "!allowBlank"
		},{
			name: "httpshost",
			conditions: [
				{ name: "httpsenable", value: true }
			],
			properties: "!allowBlank"
		},{
			name: "ftphost",
			conditions: [
				{ name: "ftpenable", value: true }
			],
			properties: "!allowBlank"
		}]
	}],

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("HTTP-Proxy"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "httpenable",
				fieldLabel: _("Enable"),
				checked: false
			},{
				xtype: "textfield",
				fieldLabel: _("Host"),
				name: "httphost",
				vtype: "domainnameIP",
				allowBlank: true
			},{
				xtype: "numberfield",
				name: "httpport",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 8080
			},{
				xtype: "textfield",
				fieldLabel: _("Username"),
				name: "httpusername",
				vtype: "username",
				allowBlank: true
			},{
				xtype: "passwordfield",
				fieldLabel: _("Password"),
				name: "httppassword",
				allowBlank: true,
				autoComplete: false
			}]
		},{
			xtype: "fieldset",
			title: _("HTTPS-Proxy"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "httpsenable",
				fieldLabel: _("Enable"),
				checked: false
			},{
				xtype: "textfield",
				fieldLabel: _("Host"),
				name: "httpshost",
				vtype: "domainnameIP",
				allowBlank: true
			},{
				xtype: "numberfield",
				name: "httpsport",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 8080
			},{
				xtype: "textfield",
				fieldLabel: _("Username"),
				name: "httpsusername",
				vtype: "username",
				allowBlank: true
			},{
				xtype: "passwordfield",
				fieldLabel: _("Password"),
				name: "httpspassword",
				allowBlank: true,
				autoComplete: false
			}]
		},{
			xtype: "fieldset",
			title: _("FTP-Proxy"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "ftpenable",
				fieldLabel: _("Enable"),
				checked: false
			},{
				xtype: "textfield",
				fieldLabel: _("Host"),
				name: "ftphost",
				vtype: "domainnameIP",
				allowBlank: true
			},{
				xtype: "numberfield",
				name: "ftpport",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 8080
			},{
				xtype: "textfield",
				fieldLabel: _("Username"),
				name: "ftpusername",
				vtype: "username",
				allowBlank: true
			},{
				xtype: "passwordfield",
				fieldLabel: _("Password"),
				name: "ftppassword",
				allowBlank: true,
				autoComplete: false
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "proxy",
	path: "/system/network",
	text: _("Proxy"),
	position: 30,
	className: "OMV.module.admin.system.network.Proxy"
});
