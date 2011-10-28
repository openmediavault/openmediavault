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
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "transmissionbt", {
	text: "BitTorrent",
	icon: "images/transmissionbt.png"
});

OMV.Module.Services.TransmissionBT = function(config) {
	var initialConfig = {
		rpcService: "TransmissionBT"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBT.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBT, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.TransmissionBT.superclass.initComponent.apply(
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
				name: "peerport",
				fieldLabel: "Peer port",
				vtype: "port",
				minValue: 0,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 51413,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Port to listen for incoming peer connections."
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				hiddenName: "sharedfolderref",
				fieldLabel: "Shared folder",
				allowNone: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Make sure the group 'debian-transmission' has read/write access to the shared folder."
			},{
				xtype: "checkbox",
				name: "portforwardingenabled",
				fieldLabel: "Port forwarding",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable port forwarding via NAT-PMP or UPnP."
			},{
				xtype: "checkbox",
				name: "pexenabled",
				fieldLabel: "Peer exchange",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable peer exchange (PEX)."
			},{
				xtype: "checkbox",
				name: "dhtenabled",
				fieldLabel: "Distributed hash table (DHT).",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable distributed hash table."
			},{
				xtype: "combo",
				name: "encryption",
				hiddenName: "encryption",
				fieldLabel: "Encryption",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,"Off" ],
						[ 1,"Preferred" ],
						[ 2,"Forced" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The peer connection encryption mode."
			}]
		},{
			xtype: "fieldset",
			title: "Remote administration settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "rpcenabled",
				fieldLabel: "Enable",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable remote administration."
			},{
				xtype: "numberfield",
				name: "rpcport",
				fieldLabel: "Port",
				vtype: "port",
				minValue: 1024,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 9091,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Port to open and listen for RPC requests on."
			},{
				xtype: "checkbox",
				name: "rpcauthenticationrequired",
				fieldLabel: "Authentication",
				checked: true,
				inputValue: 1,
				boxLabel: "Require clients to authenticate themselves.",
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "textfield",
				name: "rpcusername",
				fieldLabel: "Username",
				allowBlank: false,
				vtype: "username",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Used for client authentication."
			},{
				xtype: "passwordfield",
				name: "rpcpassword",
				fieldLabel: "Password",
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Used for client authentication."
			}]
		},{
			xtype: "fieldset",
			title: "Blocklists",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "blocklistenabled",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1,
				boxLabel: "Use blocklists."
			},{
				xtype: "checkbox",
				name: "blocklistsyncenabled",
				fieldLabel: "Auto sync",
				checked: false,
				inputValue: 1,
				boxLabel: "Update blocklists automatically.",
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "combo",
				name: "blocklistsyncfrequency",
				hiddenName: "blocklistsyncfrequency",
				fieldLabel: "Sync frequency",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ "hourly","Hourly" ],
						[ "daily","Daily" ],
						[ "weekly","Weekly" ],
						[ "monthly","Monthly" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "daily"
			},{
				xtype: "textfield",
				name: "blocklisturl",
				fieldLabel: "URL",
				allowBlank: true,
				width: 300,
				value: "http://update.transmissionbt.com/level1.gz",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The URL of the blocklist."
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		// Update authentication settings
		var field = this.findFormField("rpcauthenticationrequired");
		var checked = field.checked;
		var fields = [ "rpcusername", "rpcpassword" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
		// Update blocklist settings
		field = this.findFormField("blocklistsyncenabled");
		checked = field.checked;
		fields = [ "blocklistsyncfrequency", "blocklisturl" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
		// Update 'sharedfolderref' field settings
		field = this.findFormField("enable");
		var checked = field.checked;
		field = this.findFormField("sharedfolderref");
		if (!Ext.isEmpty(field)) {
			field.allowBlank = !checked;
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "transmissionbt", {
	cls: OMV.Module.Services.TransmissionBT
});
