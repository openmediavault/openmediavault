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
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "daapd", {
	text: _("iTunes/DAAP"),
	icon: "images/forkeddaapd.png"
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
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "textfield",
				name: "libraryname",
				fieldLabel: _("Library name"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The name of the library as displayed by the clients.")
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 0,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 3689,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Port to listen on.")
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				hiddenName: "sharedfolderref",
				fieldLabel: _("Shared folder"),
				allowNone: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The location of the media files to share.")
			},{
				xtype: "checkbox",
				name: "passwordrequired",
				fieldLabel: _("Authentication"),
				checked: false,
				inputValue: 1,
				boxLabel: _("A password is required to access the library."),
				listeners: {
					check: this._updateFormFields,
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
	_updateFormFields : function() {
		// Update 'password' field settings
		var field = this.findFormField("passwordrequired");
		var checked = field.checked;
		field = this.findFormField("password");
		if (!Ext.isEmpty(field)) {
			field.allowBlank = !checked;
			field.setReadOnly(!checked);
		}
		// Update 'sharedfolderref' field settings
		field = this.findFormField("enable");
		checked = field.checked;
		field = this.findFormField("sharedfolderref");
		if (!Ext.isEmpty(field)) {
			field.allowBlank = !checked;
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "daapd", {
	cls: OMV.Module.Services.ForkedDaapd
});
