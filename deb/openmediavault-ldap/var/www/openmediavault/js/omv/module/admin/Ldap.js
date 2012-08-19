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
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Privileges");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("privileges", "directoryservice", {
	text: _("Directory Service"),
	icon: "images/ldap.png",
	position: 5
});

Ext.ns("OMV.Module.Privileges.DirectoryService");

/**
 * @class OMV.Module.Privileges.DirectoryService.Settings
 * @derived OMV.FormPanelExt
 */
OMV.Module.Privileges.DirectoryService.Settings = function(config) {
	var initialConfig = {
		rpcService: "LDAP",
		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.DirectoryService.Settings.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.DirectoryService.Settings,
  OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Privileges.DirectoryService.Settings.superclass.
		  initComponent.apply(this, arguments);
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
				xtype: "textfield",
				name: "host",
				fieldLabel: _("Host"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The FQDN or IP address of the server.")
			},{
				xtype: "textfield",
				name: "base",
				fieldLabel: _("Base DN"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Specifies the base distinguished name (DN) to use as search base, e.g. 'dc=example,dc=net'.")
			},{
				xtype: "textfield",
				name: "rootbinddn",
				fieldLabel: _("Root Bind DN"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Specifies the distinguished name (DN) with which to bind to the directory server for lookups, e.g. 'cn=manager,dc=example,dc=net'.")
			},{
				xtype: "passwordfield",
				name: "rootbindpw",
				fieldLabel: _("Password"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Specifies the credentials with which to bind.")
			},{
				xtype: "textfield",
				name: "usersuffix",
				fieldLabel: _("Users suffix"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Specifies the user suffix, e.g. 'ou=Users'.")
			},{
				xtype: "textfield",
				name: "groupsuffix",
				fieldLabel: _("Groups suffix"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Specifies the group suffix, e.g. 'ou=Groups'.")
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "7",
					cols: "65"
				},
				anchor: "100%"
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "directoryservice", {
	cls: OMV.Module.Privileges.DirectoryService.Settings,
	title: _("LDAP"),
	position: 10
});
