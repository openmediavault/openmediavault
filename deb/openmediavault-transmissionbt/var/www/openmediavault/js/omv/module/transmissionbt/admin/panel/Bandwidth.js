/**
 * Created by JetBrains PhpStorm.
 * User: mbeck
 * Date: 28.11.11
 * Time: 20:42
 * To change this template use File | Settings | File Templates.
 */

// require("js/omv/FormPanelExt.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services.TransmissionBT.Admin");

/**
 * @class OMV.Module.Services.TransmissionBT.Admin.BandwidthPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBT.Admin.BandwidthPanel = function(config) {
	var initialConfig = {
		title: _("Bandwidth"),
		rpcService: "TransmissionBT",
		rpcGetMethod: "getBandwidth",
		rpcSetMethod: "setBandwidth"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBT.Admin.BandwidthPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBT.Admin.BandwidthPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("Speed"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "speed-limit-down-enabled",
				fieldLabel: _("Limit Download"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable download limit.")
			},{
				xtype: "numberfield",
				name: "speed-limit-down",
				fieldLabel: _("Download"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 100,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Limit download speed. Value is kb/s.")
			},{
				xtype: "checkbox",
				name: "speed-limit-up-enabled",
				fieldLabel: _("Limit Upload"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable upload limit.")
			},{
				xtype: "numberfield",
				name: "speed-limit-up",
				fieldLabel: _("Upload"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 100,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Limit upload speed. Value is kb/s.")
			},{
				xtype: "numberfield",
				name: "upload-slots-per-torrent",
				fieldLabel: _("Upload slots"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 14
			}]
		},{
			xtype: "fieldset",
			title: _("Turtle Mode"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "alt-speed-enabled",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable Turtle Mode.")
			},{
				xtype: "numberfield",
				name: "alt-speed-down",
				fieldLabel: _("Download"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 50,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Turtle Mode download speed. Value is kb/s.")
			},{
				xtype: "numberfield",
				name: "alt-speed-up",
				fieldLabel: _("Upload"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 50,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Turtle Mode upload speed. Value is kb/s.")
			}]
		}];
	}
});