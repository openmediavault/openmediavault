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
 * @class OMV.Module.Services.TransmissionBT.Admin.QueuingPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBT.Admin.QueuingPanel = function(config) {
	var initialConfig = {
		title: _("Queuing"),
		rpcService: "TransmissionBT",
		rpcGetMethod: "getQueuing",
		rpcSetMethod: "setQueuing"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBT.Admin.QueuingPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBT.Admin.QueuingPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("General"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "queue-stalled-enabled",
				fieldLabel: _("Queue Stalled"),
				checked: true,
				inputValue: 1,
				boxLabel: _("Torrents that have not shared data for queue-stalled-minutes are treated as 'stalled' and are not counted against the queue-download-size and seed-queue-size limits.")
			},{
				xtype: "numberfield",
				name: "queue-stalled-minutes",
				fieldLabel: _("Stalled Minutes"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 30
			}]
		},{
			xtype: "fieldset",
			title: _("Download Queue"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "download-queue-enabled",
				fieldLabel: _("Download"),
				checked: true,
				inputValue: 1,
				boxLabel: _("Transmission will only download download-queue-size non-stalled torrents at once.")
			},{
				xtype: "numberfield",
				name: "download-queue-size",
				fieldLabel: _("Size"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 5
			}]
		},{
			xtype: "fieldset",
			title: _("Seed Queue"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "seed-queue-enabled",
				fieldLabel: _("Seed"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Transmission will only seed seed-queue-size non-stalled torrents at once.")
			},{
				xtype: "numberfield",
				name: "seed-queue-size",
				fieldLabel: _("Size"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 10
			}]
		}];
	}
});