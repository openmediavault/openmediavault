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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

/**
 * @class OMV.module.admin.diagnostic.log.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.diagnostic.log.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "Syslog",
	rpcGetMethod: "getSettings",
	rpcSetMethod: "setSettings",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: [
				"host",
				"port",
				"protocol"
			],
			conditions: [
				{ name: "enable", value: true }
			],
			properties: [
				"!allowBlank",
				"!readOnly"
			]
		}]
	}],

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("Remote syslog"),
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
				name: "host",
				fieldLabel: _("Host"),
				allowBlank: true,
				vtype: "domainnameIP",
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
				fieldLabel: _("Protocol"),
				emptyText: _("Select a protocol ..."),
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: [
						[ "tcp", "TCP" ],
						[ "udp", "UDP" ]
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
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/diagnostic/log",
	text: _("Settings"),
	position: 20,
	className: "OMV.module.admin.diagnostic.log.Settings"
});
