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

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "snmp", {
	text: _("SNMP"),
	icon: "images/snmp.png"
});

/**
 * @class OMV.Module.Services.SNMP
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.SNMP = function(config) {
	var initialConfig = {
		rpcService: "SNMP"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.SNMP.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Services.SNMP, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.SNMP.superclass.initComponent.apply(this,
		  arguments);
		this.on("load", this._updateFormFields, this);
	},

	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("General settings"),
			defaults: {
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
				name: "syslocation",
				fieldLabel: _("Location"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Location information, e.g. physical location of this system.")
			},{
				xtype: "textfield",
				name: "syscontact",
				fieldLabel: _("Contact"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Contact information, e.g. name or email address of the person responsible for this system.")
			},{
				xtype: "combo",
				name: "version",
				hiddenName: "version",
				fieldLabel: _("Version"),
				mode: "local",
				store: [
					[ "2c",_("SNMP version 1/2c") ],
					[ "3",_("SNMP version 3") ]
				],
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "2c",
				listeners: {
					select: function(combo, record, index) {
						this._updateFormFields();
					},
					scope: this
				}
			},{
				xtype: "textfield",
				name: "community",
				fieldLabel: _("Community"),
				allowBlank: false,
				value: "public"
			},{
				xtype: "textfield",
				name: "username",
				fieldLabel: _("Username"),
				hidden: true,
				allowBlank: true
			},{
				xtype: "combo",
				name: "securitylevel",
				hiddenName: "securitylevel",
				fieldLabel: _("Security level"),
				mode: "local",
				store: [
					[ "noauth",_("No authentication and no privacy") ],
					[ "auth",_("Authentication and no privacy") ],
					[ "priv",_("Authentication and privacy") ]
				],
				hidden: true,
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "noauth",
				listeners: {
					select: function(combo, record, index) {
						this._updateFormFields();
					},
					scope: this
				}
			},{
				xtype: "combo",
				name: "authtype",
				hiddenName: "authtype",
				fieldLabel: _("Authentication type"),
				mode: "local",
				store: [
					[ "MD5","MD5" ],
					[ "SHA","SHA" ]
				],
				hidden: true,
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "MD5"
			},{
				xtype: "passwordfield",
				name: "authpassphrase",
				fieldLabel: _("Authentication passphrase"),
				hidden: true,
				allowBlank: true,
				minLength: 8
			},{
				xtype: "combo",
				name: "privtype",
				hiddenName: "privtype",
				fieldLabel: _("Privacy type"),
				mode: "local",
				store: [
					[ "DES","DES" ],
					[ "AES","AES" ]
				],
				hidden: true,
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "DES"
			},{
				xtype: "passwordfield",
				name: "privpassphrase",
				fieldLabel: _("Privacy passphrase"),
				hidden: true,
				allowBlank: true,
				minLength: 8
			}]
		},{
			xtype: "fieldset",
			title: _("Advanced settings"),
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "3",
					cols: "65"
				},
				anchor: "100%"
			}]
		},{
			xtype: "fieldset",
			title: _("Traps"),
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "trapenable",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1,
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "textfield",
				name: "trapcommunity",
				fieldLabel: _("Community"),
				allowBlank: true
			},{
				xtype: "textfield",
				name: "traphost",
				fieldLabel: _("Host"),
				allowBlank: true
			},{
				xtype: "numberfield",
				name: "trapport",
				fieldLabel: _("Port"),
				allowBlank: true,
				allowDecimals: false,
				allowNegative: false,
				minValue: 1,
				maxValue: 65535,
				vtype: "port"
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		// Update trap settings
		var field = this.findFormField("trapenable");
		var checked = field.checked;
		var fields = [ "trapcommunity", "traphost", "trapport" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
		// Update version settings
		field = this.findFormField("version");
		var version = field.getValue();
		fields = [ "community" ];
		for (i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				var visible = (version === "2c");
				field.allowBlank = !visible;
				field.setVisible(visible);
			}
		}
		field = this.findFormField("securitylevel");
		var securityLevel = field.getValue();
		fields = [ "securitylevel", "username", "authtype", "authpassphrase",
		  "privtype", "privpassphrase" ];
		for (i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				var visible = false;;
				switch (fields[i]) {
				case "securitylevel":
				case "username":
					visible = (version === "3");
					break;
				case "authtype":
				case "authpassphrase":
					visible = (version === "3") &&
					  ((securityLevel === "auth") ||
					  (securityLevel === "priv"));
					break;
				case "privtype":
				case "privpassphrase":
					visible = (version === "3") &&
					  (securityLevel === "priv");
					break;
				}
				field.allowBlank = !visible;
				field.setVisible(visible);
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "snmp", {
	cls: OMV.Module.Services.SNMP
});
