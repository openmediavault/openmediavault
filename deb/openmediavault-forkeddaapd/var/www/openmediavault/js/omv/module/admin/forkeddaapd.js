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
// require("js/omv/ModuleManager.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/field/Password.js")
// require("js/omv/form/field/SharedFolderComboBox.js")
// require("js/omv/form/field/plugin/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.ModuleManager.registerMenu("services", "daapd", {
	text: _("iTunes/DAAP"),
	icon16: "images/forkeddaapd.png"
});

/**
 * @class OMV.Module.Services.ForkedDaapd
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.ForkedDaapd = function(config) {
	var initialConfig = {
		rpcService: "ForkedDaapd"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.ForkedDaapd.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Services.ForkedDaapd, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.ForkedDaapd.superclass.initComponent.apply(this,
		  arguments);
		this.on("load", this.updateFormFields, this);
	},

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
				listeners: {
					check: this.updateFormFields,
					scope: this
				}
			},{
				xtype: "textfield",
				name: "libraryname",
				fieldLabel: _("Library name"),
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The name of the library as displayed by the clients.")
				}]
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 0,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 3689,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Port to listen on.")
				}]
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				fieldLabel: _("Shared folder"),
				allowNone: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The location of the media files to share.")
				}]
			},{
				xtype: "checkbox",
				name: "passwordrequired",
				fieldLabel: _("Authentication"),
				checked: false,
				boxLabel: _("A password is required to access the library."),
				listeners: {
					check: this.updateFormFields,
					scope: this
				}
			},{
				xtype: "passwordfield",
				name: "password",
				fieldLabel: _("Password"),
				allowBlank: true
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	updateFormFields: function() {
		// Update 'password' field settings
		var field = this.findField("passwordrequired");
		var checked = field.checked;
		field = this.findField("password");
		if(!Ext.isEmpty(field)) {
			field.allowBlank = !checked;
			field.setReadOnly(!checked);
		}
		// Update 'sharedfolderref' field settings
		field = this.findField("enable");
		checked = field.checked;
		field = this.findField("sharedfolderref");
		if(!Ext.isEmpty(field)) {
			field.allowBlank = !checked;
		}
	}
});
OMV.ModuleManager.registerPanel("services", "daapd", {
	cls: OMV.Module.Services.ForkedDaapd
});
