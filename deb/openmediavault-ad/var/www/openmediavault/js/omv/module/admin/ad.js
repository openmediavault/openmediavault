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
// require("js/omv/NavigationPanel.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Privileges");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("privileges", "directoryservice", {
	text: _("Directory Service"),
	icon: "images/network.png",
	position: 5
});

Ext.ns("OMV.Module.Privileges.DirectoryService");

/**
 * @class OMV.Module.Privileges.DirectoryService.ADSettings
 * @derived OMV.FormPanelExt
 */
OMV.Module.Privileges.DirectoryService.ADSettings = function(config) {
	var initialConfig = {
		rpcService: "DirectoryService",
		rpcGetMethod: "getADSettings",
		rpcSetMethod: "setADSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.DirectoryService.ADSettings.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.DirectoryService.ADSettings,
  OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Privileges.DirectoryService.ADSettings.superclass.
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
				xtype: "textfield",
				name: "dcname",
				fieldLabel: _("Domain controller name"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("E.g. host.example.com")
			},{
				xtype: "textfield",
				name: "domainname",
				fieldLabel: _("Domain name"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("E.g. example.com")
			},{
				xtype: "textfield",
				name: "netbiosname",
				fieldLabel: _("Domain NetBIOS name"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("E.g. EXAMPLE")
			},{
				xtype: "textfield",
				name: "adminname",
				fieldLabel: _("Administrator name"),
				allowBlank: false
			},{
				xtype: "passwordfield",
				name: "adminpassword",
				fieldLabel: _("Administrator password"),
				allowBlank: false
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "directoryservice", {
	cls: OMV.Module.Privileges.DirectoryService.ADSettings,
	title: _("Active Directory"),
	position: 10
});
