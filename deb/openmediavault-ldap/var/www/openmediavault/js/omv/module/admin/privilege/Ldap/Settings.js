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
 * @class OMV.module.admin.privilege.ldap.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.privilege.ldap.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "LDAP",
	rpcGetMethod: "getSettings",
	rpcSetMethod: "setSettings",

	getFormItems: function() {
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
				name: "host",
				fieldLabel: _("Host"),
				allowBlank: false,
				vtype: "domainnameIPv4",
				plugins: [{
					ptype: "fieldinfo",
					text: _("The FQDN or IP address of the server.")
				}]
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: "Port",
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the port to connect to.")
				}],
				value: 389
			},{
				xtype: "textfield",
				name: "base",
				fieldLabel: _("Base DN"),
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the base distinguished name (DN) to use as search base, e.g. 'dc=example,dc=net'.")
				}]
			},{
				xtype: "textfield",
				name: "rootbinddn",
				fieldLabel: _("Root Bind DN"),
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the distinguished name (DN) with which to bind to the directory server for lookups, e.g. 'cn=manager,dc=example,dc=net'.")
				}]
			},{
				xtype: "passwordfield",
				name: "rootbindpw",
				fieldLabel: _("Password"),
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the credentials with which to bind.")
				}]
			},{
				xtype: "textfield",
				name: "usersuffix",
				fieldLabel: _("Users suffix"),
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the user suffix, e.g. 'ou=Users'."),
				}],
				value: "ou=Users"
			},{
				xtype: "textfield",
				name: "groupsuffix",
				fieldLabel: _("Groups suffix"),
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the group suffix, e.g. 'ou=Groups'."),
				}],
				value: "ou=Groups"
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Please check the <a href='http://linux.die.net/man/5/nss_ldap' target='_blank'>manual page</a> for more details."),
				}]
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/privilege/ldap",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.privilege.ldap.Settings"
});
