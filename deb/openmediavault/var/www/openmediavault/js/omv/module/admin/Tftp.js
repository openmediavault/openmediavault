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
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "tftp", {
	text: _("TFTP"),
	icon: "images/tftp.png"
});

/**
 * @class OMV.Module.Services.TFTP
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TFTP = function(config) {
	var initialConfig = {
		rpcService: "TFTP"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TFTP.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.TFTP, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.TFTP.superclass.initComponent.apply(
		  this, arguments);
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
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 0,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 69,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Port to listen on.")
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				hiddenName: "sharedfolderref",
				fieldLabel: _("Shared folder"),
				allowNone: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The location of the files to share.")
			},{
				xtype: "numberfield",
				name: "blocksize",
				fieldLabel: _("Blocksize"),
				minValue: 512,
				maxValue: 65464,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: true,
				value: 512,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Specifies the maximum permitted block size. The permitted range for this parameter is from 512 to 65464.")
			},{
				xtype: "numberfield",
				name: "retransmit",
				fieldLabel: _("Retry timeout"),
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 1000000,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Determine the default timeout, in microseconds, before the first packet is retransmitted.")
			},{
				xtype: "checkbox",
				name: "allownewfiles",
				fieldLabel: _("Allow new files"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Allow new files to be created."),
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("By default, only files that already exist are allowed to be uploaded.")
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				anchor: "100%"
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
				boxLabel: _("Advertise this service via mDNS/DNS-SD.")
			},{
				xtype: "textfield",
				name: "dnssdname",
				fieldLabel: _("Name"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The service name."),
				value: "%h - TFTP"
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("enable");
		var checked = field.checked;
		var fields = [ "sharedfolderref" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "tftp", {
	cls: OMV.Module.Services.TFTP
});
