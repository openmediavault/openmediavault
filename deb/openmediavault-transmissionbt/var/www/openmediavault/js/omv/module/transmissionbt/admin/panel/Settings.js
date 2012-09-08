/**
 * Created by JetBrains PhpStorm.
 * User: mbeck
 * Date: 28.11.11
 * Time: 20:39
 * To change this template use File | Settings | File Templates.
 */

// require("js/omv/FormPanelExt.js")
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services.TransmissionBT.Admin");

/**
 * @class OMV.Module.Services.TransmissionBT.Admin.SettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBT.Admin.SettingsPanel = function(config) {
	var initialConfig = {
		title: _("Settings"),
		rpcService: "TransmissionBT",
		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBT.Admin.SettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBT.Admin.SettingsPanel, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.TransmissionBT.Admin.SettingsPanel.superclass.
		  initComponent.apply(this, arguments);
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
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				hiddenName: "sharedfolderref",
				fieldLabel: _("Shared folder"),
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Make sure the group 'debian-transmission' has read/write access to the shared folder.")
			},{
				xtype: "checkbox",
				name: "pexenabled",
				fieldLabel: _("Peer exchange"),
				checked: true,
				inputValue: 1,
				boxLabel: _("Enable peer exchange (PEX).")
			},{
				xtype: "checkbox",
				name: "dhtenabled",
				fieldLabel: _("Distributed hash table (DHT)."),
				checked: true,
				inputValue: 1,
				boxLabel: _("Enable distributed hash table.")
			},{
				xtype: "checkbox",
				name: "lpd-enabled",
				fieldLabel: _("Local Peer Discovery (LPD)."),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable local peer discovery.")
			},{
				xtype: "checkbox",
				name: "utp-enabled",
				fieldLabel: _("Micro Transport Protocol (&micro;TP)."),
				checked: true,
				inputValue: 1,
				boxLabel: _("Enable micro transport protocol.")
			},{
				xtype: "combo",
				name: "encryption",
				hiddenName: "encryption",
				fieldLabel: _("Encryption"),
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
				infoText: _("The peer connection encryption mode.")
			},{
				xtype: "combo",
				name: "message-level",
				hiddenName: "message-level",
				fieldLabel: _("Message Level"),
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,_("None") ],
						[ 1,_("Error") ],
						[ 2,_("Info") ],
						[ 3,_("Debug") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 2,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Set verbosity of transmission messages.")
			},{
				xtype: "checkbox",
				name: "lazy-bitfield-enabled",
				fieldLabel: _("Lazy Bitfield"),
				checked: true,
				inputValue: 1,
				boxLabel: _("May help get around some ISP filtering.")
			},{
				xtype: "checkbox",
				name: "scrape-paused-torrents-enabled",
				fieldLabel: _("Scrape paused torrents."),
				checked: true,
				inputValue: 1,
				boxLabel: _("Enable paused torrent scraping.")
			},{
				xtype: "numberfield",
				name: "umask",
				fieldLabel: _("Umask"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 0,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Sets transmission's file mode creation mask.")
			},{
				xtype: "numberfield",
				name: "cache-size-mb",
				fieldLabel: _("Cache Size"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 4,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Cache size (in Mb) to reduce the number of disk reads and writes.")
			}]
		},{
			xtype: "fieldset",
			title: _("RPC/WebUI Settings"),
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "numberfield",
				name: "rpcport",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1024,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 9091,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Port to open and listen for RPC/Web requests on.")
			},{
				xtype: "textfield",
				name: "rpcurl",
				fieldLabel: _("Uri"),
				vtype: "transmissionbturi",
				allowBlank: false,
				value: 'transmission',
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Url to access RPC (http://localhost/&lt;URI&gt;/(rpc|web).")
			},{
				xtype: "checkbox",
				name: "rpcauthenticationrequired",
				fieldLabel: _("Authentication"),
				checked: true,
				inputValue: 1,
				boxLabel: _("Require clients to authenticate themselves."),
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "textfield",
				name: "rpcusername",
				fieldLabel: _("Username"),
				allowBlank: false,
				vtype: "username",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Used for client authentication.")
			},{
				xtype: "passwordfield",
				name: "rpcpassword",
				fieldLabel: _("Password"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Used for client authentication.")
			}]
		},{
			xtype: "fieldset",
			title: _("Blocklists"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "blocklistenabled",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Use blocklists.")
			},{
				xtype: "checkbox",
				name: "blocklistsyncenabled",
				fieldLabel: _("Auto sync"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Update blocklists automatically."),
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "combo",
				name: "blocklistsyncfrequency",
				hiddenName: "blocklistsyncfrequency",
				fieldLabel: _("Sync frequency"),
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ "hourly",_("Hourly") ],
						[ "daily",_("Daily") ],
						[ "weekly",_("Weekly") ],
						[ "monthly",_("Monthly") ]
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
				fieldLabel: _("URL"),
				allowBlank: true,
				width: 300,
				value: "http://update.transmissionbt.com/level1.gz",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The URL of the blocklist.")
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
		checked = field.checked;
		field = this.findFormField("sharedfolderref");
		if (!Ext.isEmpty(field)) {
			field.allowBlank = !checked;
		}
	}
});



Ext.apply(Ext.form.VTypes, {

    transmissionbturi: function(v) {
        return /^[a-z0-9]+$/i.test(v);
    },
    transmissionbturiText: "Invalid Uri.",
    transmissionbturiMask: /[a-z0-9\-_]/i

});
