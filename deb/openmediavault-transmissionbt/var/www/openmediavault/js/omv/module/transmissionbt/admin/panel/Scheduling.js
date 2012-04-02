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
 * @class OMV.Module.Services.TransmissionBT.Admin.SchedulingPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBT.Admin.SchedulingPanel = function(config) {
	var initialConfig = {
		title: _("Scheduling"),
		rpcService: "TransmissionBT",
		rpcGetMethod: "getScheduling",
		rpcSetMethod: "setScheduling"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBT.Admin.SchedulingPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBT.Admin.SchedulingPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{xtype: "fieldset",
			title: _("General"),
			defaults: {
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "alt-speed-time-enabled",
				fieldLabel: _("Scheduling"),
				checked: false,
				inputValue: 1,
				boxLabel: _("When enabled, this will toggle the Turtle Mode.")
			},{
				xtype: "fieldset",
				title: _("Time"),
				defaults: {
					labelSeparator: ""
				},
				items: [{
					xtype: "compositefield",
					name: "begin-time",
					fieldLabel: _("Begin"),
					width: 200,
					items: [{
						xtype: "combo",
						name: "begin-hour",
						mode: "local",
						store: Array.range(0, 23),
						allowBlank: false,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: 9,
						reset: function() {}
					},{
						xtype: "displayfield",
						value: ":"
					},{
						xtype: "combo",
						name: "begin-minute",
						mode: "local",
						store: Array.range(0, 59),
						allowBlank: false,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: 0,
						reset: function() {}
					}]
				},{
					xtype: "compositefield",
					name: "end-time",
					fieldLabel: _("End"),
					width: 200,
					items: [{
						xtype: "combo",
						name: "end-hour",
						mode: "local",
						store: Array.range(0, 23),
						allowBlank: false,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: 17,
						reset: function() {}
					},{
						xtype: "displayfield",
						value: ":"
					},{
						xtype: "combo",
						name: "end-minute",
						mode: "local",
						store: Array.range(0, 59),
						allowBlank: false,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: 0,
						reset: function() {}
					}]
				}]
			},{
				xtype: "fieldset",
				title: _("Days"),
				defaults: {
					labelSeparator: ""
				},
				items: [{xtype: "checkbox",
					name: "days-sunday",
					fieldLabel: _("Sunday"),
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-monday",
					fieldLabel: _("Monday"),
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-tuesday",
					fieldLabel: _("Tuesday"),
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-wednesday",
					fieldLabel: _("Wednesday"),
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-thursday",
					fieldLabel: _("Thursday"),
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-friday",
					fieldLabel: _("Friday"),
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-saturday",
					fieldLabel: _("Saturday"),
					checked: true,
					inputValue: 1
				}]
			}]
		},{
			xtype: "fieldset",
			title: _("Idle"),
			defaults: {
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "idle-seeding-limit-enabled",
				fieldLabel: _("Seeding Limit"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Stop seeding after being idle for N minutes.")
			},{
				xtype: "numberfield",
				name: "idle-seeding-limit",
				fieldLabel: _("Idle Minutes"),
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 30
			}]
		},{
			xtype: "fieldset",
			title: _("Ratio"),
			defaults: {
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "ratio-limit-enabled",
				fieldLabel: _("Ratio"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Transmission will only seed until ratio limit is reached.")
			},{
				xtype: "numberfield",
				name: "ratio-limit",
				fieldLabel: _("Ratio Limit"),
				allowDecimals: true,
				allowNegative: false,
				allowBlank: false,
				value: 2.0
			}]
		}];
	}
});