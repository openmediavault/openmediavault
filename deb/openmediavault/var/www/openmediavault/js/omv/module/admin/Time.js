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
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/FormPanelExt.js")

Ext.ns("OMV.Module.System");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "time", {
	text: _("Date & Time"),
	icon: "images/time.png",
	position: 20
});

/**
 * @class OMV.Module.System.Time
 * @derived OMV.FormPanelExt
 */
OMV.Module.System.Time = function(config) {
	var initialConfig = {
		rpcService: "System",
 		rpcGetMethod: "getTimeSettings",
		rpcSetMethod: "setTimeSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Time.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.System.Time, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.System.Time.superclass.initComponent.apply(this,
		  arguments);
		this.on("load", this._updateFormFields, this);
		this.on("submit", function() {
			  this.doLoad(); // Reload the form
		  }, this);
	},

	getFormItems : function() {
		var dtNow = new Date(); // Display local time per default
		return [{
			xtype: "fieldset",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "displayfield",
				name: "now",
				fieldLabel: _("Current time"),
				reset: function() {
					// Workaround to prevent this field from getting
					// empty when pressing the 'Reset' button.
				},
				anchor: "100%"
			}]
		},{
			xtype: "fieldset",
			title: _("Settings"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "combo",
				name: "timezone",
				hiddenName: "timezone",
				fieldLabel: _("Time zone"),
				mode: "local",
				store: new OMV.data.Store({
					autoLoad: true,
					remoteSort: false,
					proxy: new OMV.data.DataProxy({
						"service": "System",
						"method": "getTimeZoneList",
						"appendPagingParams": false
					}),
					reader: new Ext.data.JsonReader({
						idProperty: "value",
						totalProperty: "total",
						root: "data",
						fields: [ "value" ]
					})
				}),
				displayField: "value",
				valueField: "value",
				allowBlank: false,
				typeAhead: true,
				forceSelection: true,
				triggerAction: "all",
				value: "UTC"
			},{
				xtype: "checkbox",
				name: "ntpenable",
				fieldLabel: _("Use NTP server"),
				checked: false,
				inputValue: 1,
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "container",
				fieldLabel: "&nbsp;",
				items: [{
					layout: "column",
					defaults: {
						layout: "form"
					},
					items: [{
						defaults: {
							labelSeparator: ""
						},
						items: [{
							xtype: "textfield",
							name: "ntptimeservers",
							fieldLabel: _("Time servers"),
							allowBlank: true,
							readOnly: true,
							value: "pool.ntp.org"
/*
						},{
							xtype: "button",
							id: this.getId() + "-ntpupdate",
							hideLabel: true,
							disabled: true,
							text: _("Update now"),
							scope: this,
							handler: function() {
								OMV.Ajax.request(function(id, response, error) {
									  if (error !== null) {
										  OMV.MessageBox.error(null, error);
									  } else {
										  this.doReload();
									  }
								  }, this, "System", "setNtpDate");
							}
*/
						}]
					}]
				}]
			},{
				xtype: "container",
				fieldLabel: _("Manual"),
				items: [{
					layout: "column",
					defaults: {
						layout: "form"
					},
					items: [{
						defaults: {
							labelSeparator: ""
						},
						items: [{
							xtype: "datefield",
							name: "date",
							hiddenName: "date",
							fieldLabel: _("Date"),
							width: 100,
							value: dtNow,
							submitValue: false
						},{
							xtype: "compositefield",
							name: "manualtime",
							fieldLabel: _("Time"),
							width: 200,
							items: [{
								xtype: "combo",
								name: "hour",
								mode: "local",
								store: Array.range(0, 23),
								allowBlank: false,
								editable: false,
								triggerAction: "all",
								width: 50,
								value: dtNow.getHours(),
								submitValue: false,
								reset: function() {
									var dtNow = new Date();
									this.setValue(dtNow.getHours());
								}
							},{
								xtype: "displayfield",
								value: ":"
							},{
								xtype: "combo",
								name: "minute",
								mode: "local",
								store: Array.range(0, 59),
								allowBlank: false,
								editable: false,
								triggerAction: "all",
								width: 50,
								value: dtNow.getMinutes(),
								submitValue: false,
								reset: function() {
									var dtNow = new Date();
									this.setValue(dtNow.getMinutes());
								}
							},{
								xtype: "displayfield",
								value: ":"
							},{
								xtype: "combo",
								name: "second",
								mode: "local",
								store: Array.range(0, 59),
								allowBlank: false,
								editable: false,
								triggerAction: "all",
								width: 50,
								value: dtNow.getSeconds(),
								submitValue: false,
								reset: function() {
									var dtNow = new Date();
									this.setValue(dtNow.getSeconds());
								}
							}]
						},{
							xtype: "button",
							id: this.getId() + "-manualupdate",
							hideLabel: true,
							text: _("Update now"),
							scope: this,
							handler: function() {
								var values = {};
								var fields = ["date","hour","minute","second"];
								for (var i = 0; i < fields.length; i++) {
									var field = this.findFormField(fields[i]);
									values[fields[i]] = field.getValue();
								}
								var date = new Date(values.date);
								date.setHours(values.hour, values.minute,
								  values.second);
								OMV.Ajax.request(function(id, response, error) {
									  if (error !== null) {
										  OMV.MessageBox.error(null, error);
									  } else {
										  this.doReload();
									  }
								  }, this, "System", "setDate",
								  { "timestamp": date.format("U") });
							}
						}]
					}]
				}]
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var fields, enable;

		var field = this.findFormField("ntpenable");
		var ntpenable = field.checked;

		// Process 'Manual' fields
		fields = [ "date", "hour", "minute", "second", "updatenow" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.setReadOnly(ntpenable);
			}
		}
		field = Ext.getCmp(this.getId() + "-manualupdate");
		if (!Ext.isEmpty(field)) {
			field.setDisabled(ntpenable);
		}

		// Process 'NTP' fields
		fields = [ "ntptimeservers" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.setReadOnly(!ntpenable);
				if (field.xtype === "textfield") {
					field.allowBlank = !ntpenable;
				}
			}
		}
		field = Ext.getCmp(this.getId() + "-ntpupdate");
		if (!Ext.isEmpty(field)) {
			field.setDisabled(!ntpenable);
		}
	},

	setValues : function(values) {
		var dtNow = new Date.parseDate(values.date.ISO8601, "c");
		Ext.apply(values, {
			now: values.date.local,
			date: dtNow.format(Ext.form.DateField.prototype.format),
			hour: dtNow.getHours(),
			minute: dtNow.getMinutes(),
			second: dtNow.getSeconds()
		});
		return OMV.Module.System.Time.superclass.setValues.call(this, values);
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "time", {
	cls: OMV.Module.System.Time
});
