/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
// require("js/omv/form/CertificateComboBox.js")

Ext.ns("OMV.Module.System.GeneralSettings");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "generalsettings", {
	text: "General Settings",
	icon: "images/preferences.png",
	position: 10
});

/**
 * @class OMV.Module.System.GeneralSettings.WebUI
 * @derived OMV.FormPanelExt
 */
OMV.Module.System.GeneralSettings.WebUI = function(config) {
	var initialConfig = {
		rpcService: "System",
 		rpcGetMethod: "getWebUISettings",
		rpcSetMethod: "setWebUISettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.GeneralSettings.WebUI.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.GeneralSettings.WebUI, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.System.GeneralSettings.WebUI.superclass.initComponent.
		  apply(this, arguments);
		this.on("load", this._updateFormFields, this);
	},

	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "General settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "numberfield",
				name: "port",
				fieldLabel: "Port",
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 80
			},{
				xtype: "numberfield",
				name: "timeout",
				fieldLabel: "Session timeout",
				minValue: 1,
				maxValue: 30,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 5,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The session timeout time in minutes."
			}]
		},{
			xtype: "fieldset",
			title: "Secure connection",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enablessl",
				fieldLabel: "Enable SSL/TLS",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable secure connection.",
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "numberfield",
				name: "sslport",
				fieldLabel: "Port",
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 443
			},{
				xtype: "checkbox",
				name: "forcesslonly",
				fieldLabel: "Force SSL/TLS",
				checked: false,
				inputValue: 1,
				boxLabel: "Force secure connection only."
			},{
				xtype: "certificatecombo",
				name: "sslcertificateref",
				hiddenName: "sslcertificateref",
				fieldLabel: "Certificate",
				allowNone: true,
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The SSL certificate."
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("enablessl");
		var checked = field.checked;
		var fields = [ "sslport", "forcesslonly", "sslcertificateref" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "generalsettings", {
	cls: OMV.Module.System.GeneralSettings.WebUI,
	title: "Web Administration",
	position: 10
});
