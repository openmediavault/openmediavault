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
// require("js/omv/data/DataProxy.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "tftp", {
	text: "TFTP",
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
			title: "General settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1,
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: "Port",
				vtype: "port",
				minValue: 0,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 69,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Port to listen on."
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				hiddenName: "sharedfolderref",
				fieldLabel: "Shared folder",
				allowNone: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The location of the files to share. Make sure " +
				  "the user 'tftp' has the required permission for the " +
				  "shared folder. Note, users must join the group 'tftp' " +
				  "to access the files."
			},{
				xtype: "numberfield",
				name: "blocksize",
				fieldLabel: "Blocksize",
				minValue: 512,
				maxValue: 65464,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: true,
				value: 512,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Specifies the maximum permitted block size. The permitted range for this parameter is from 512 to 65464."
			},{
				xtype: "numberfield",
				name: "retransmit",
				fieldLabel: "Retry timeout",
				minValue: 0,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 1000000,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Determine the default timeout, in microseconds, before the first packet is retransmitted."
			},{
				xtype: "checkbox",
				name: "allownewfiles",
				fieldLabel: "Allow new files",
				checked: false,
				inputValue: 1,
				boxLabel: "Allow new files to be created.",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "By default, only files that already exist are allowed to be uploaded."
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: "Extra options",
				allowBlank: true
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
