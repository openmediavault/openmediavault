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
// require("js/omv/form/CertificateComboBox.js")
// require("js/omv/form/PasswordField.js")

Ext.ns("OMV.Module.System.WebGUI");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "generalsettings", {
	text: _("General Settings"),
	icon: "images/preferences.png",
	position: 10
});

/**
 * @class OMV.Module.System.WebGUI.Settings
 * @derived OMV.FormPanelExt
 */
OMV.Module.System.WebGUI.Settings = function(config) {
	var initialConfig = {
		rpcService: "WebGUI",
 		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.WebGUI.Settings.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.WebGUI.Settings, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.System.WebGUI.Settings.superclass.initComponent.
		  apply(this, arguments);
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
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
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
				fieldLabel: _("Session timeout"),
				minValue: 1,
				maxValue: 30,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 5,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The session timeout time in minutes.")
			}]
		},{
			xtype: "fieldset",
			title: _("Secure connection"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enablessl",
				fieldLabel: _("Enable SSL/TLS"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable secure connection."),
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "numberfield",
				name: "sslport",
				fieldLabel: _("Port"),
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
				fieldLabel: _("Force SSL/TLS"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Force secure connection only.")
			},{
				xtype: "certificatecombo",
				name: "sslcertificateref",
				hiddenName: "sslcertificateref",
				fieldLabel: _("Certificate"),
				allowNone: true,
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The SSL certificate.")
			}]
		},{
			xtype: "fieldset",
			title: _("DNS Service Discovery"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "dnssdenable",
				fieldLabel: _("Enable"),
				checked: true,
				inputValue: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Advertise this service via mDNS/DNS-SD.")
			},{
				xtype: "textfield",
				name: "dnssdname",
				fieldLabel: _("Name"),
				allowBlank: false,
				width: 180,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The service name."),
				value: "%h - Web administration"
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
	cls: OMV.Module.System.WebGUI.Settings,
	title: _("Web Administration"),
	position: 10
});

/**
 * @class OMV.Module.System.WebGUI.AdminPasswd
 * @derived OMV.FormPanelExt
 */
OMV.Module.System.WebGUI.AdminPasswd = function(config) {
	var initialConfig = {
		rpcService: "WebGUI",
		rpcSetMethod: "setPassword"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.WebGUI.AdminPasswd.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.WebGUI.AdminPasswd, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("Administrator password"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "passwordfield",
				name: "password",
				fieldLabel: _("Password"),
				allowBlank: true
			},{
				xtype: "passwordfield",
				name: "passwordconf",
				fieldLabel: _("Confirm password"),
				allowBlank: true,
				submitValue: false
			}]
		}];
	},

	cbOkBtnHdl : function() {
		// Only submit values if form values have been changed
		if (!this.isDirty())
			return;
		OMV.Module.System.WebGUI.AdminPasswd.superclass.
		  cbOkBtnHdl.call(this);
	},

	isValid : function() {
		var valid = OMV.Module.System.WebGUI.AdminPasswd.superclass.
		  isValid.call(this);
		if (valid) {
			var values = this.getValues();
			// Check the password
			var field = this.findFormField("passwordconf");
			if (values.password !== field.getValue()) {
				var msg = _("Passwords don't match");
				this.markInvalid([
					{ id: "password", msg: msg },
					{ id: "passwordconf", msg: msg }
				]);
				valid = false;
			}
		}
		return valid;
	},

	doLoad : function() {
		// Empty form fields
		this.setValues({
			password: "",
			passwordconf: ""
		});
	},

	doSubmit : function() {
		// Validate values
		if (!this.isValid()) {
			var basicForm = this.getForm();
			basicForm.markInvalid();
		} else {
			var values = this.getValues();
			// Display waiting dialog
			OMV.MessageBox.wait(null, _("Saving ..."));
			// Execute RPC
			OMV.Ajax.request(this.cbSubmitHdl, this, this.rpcService,
			  this.rpcSetMethod, { "password": values.password });
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "generalsettings", {
	cls: OMV.Module.System.WebGUI.AdminPasswd,
	title: _("Web Administrator Password"),
	position: 20
});
