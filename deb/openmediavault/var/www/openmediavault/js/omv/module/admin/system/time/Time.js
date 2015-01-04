/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/data/reader/RpcArray.js")

/**
 * @class OMV.module.admin.system.time.Time
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.system.time.Time", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.proxy.Rpc",
		"OMV.data.reader.RpcArray"
	],

	rpcService: "System",
	rpcGetMethod: "getTimeSettings",
	rpcSetMethod: "setTimeSettings",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: [
				"date",
				"hour",
				"minute",
				"second",
				"ntptimeservers"
			],
			conditions: [
				{ name: "ntpenable", value: true }
			],
			properties: "readOnly"
		},{
			name: "manualupdate",
			conditions: [
				{ name: "ntpenable", value: true }
			],
			properties: "disabled"
		},{
			name: "ntptimeservers",
			conditions: [
				{ name: "ntpenable", value: true }
			],
			properties: [ "allowBlank", "!readOnly" ]
		}]
	}],

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		me.on("submit", function() {
			me.doReload(); // Reload the form content.
		}, me);
	},

	getFormItems: function() {
		var me = this;
		var dtNow = new Date(); // Display local time per default
		return [{
			xtype: "fieldset",
			title: _("Info"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "displayfield",
				name: "now",
				fieldLabel: _("Current time"),
				reset: function() {
					// Workaround to prevent this field from getting
					// empty when pressing the 'Reset' button.
				}
			}]
		},{
			xtype: "fieldset",
			title: _("Settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "combo",
				name: "timezone",
				fieldLabel: _("Time zone"),
				store: Ext.create("OMV.data.Store", {
					autoLoad: true,
					fields: [
						{ name: "value", type: "string" }
					],
					proxy: {
						type: "rpc",
						reader: "rpcarray",
						appendSortParams: false,
						rpcData: {
							service: "System",
							method: "getTimeZoneList"
						}
					}
				}),
				queryMode: "local",
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
				checked: false
			},{
				xtype: "textfield",
				name: "ntptimeservers",
				fieldLabel: _("Time servers"),
				vtype: "domainnameIP",
				allowBlank: true,
				readOnly: true,
				value: "pool.ntp.org"
			},{
				xtype: "fieldcontainer",
				fieldLabel: _("Manual"),
				layout: "anchor",
				fieldDefaults: {
					anchor: "100%",
					labelSeparator: ""
				},
				items: [{
					xtype: "datefield",
					name: "date",
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
						queryMode: "local",
						store: Ext.Array.range(0, 23),
						readOnly: true,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: dtNow.getHours(),
						submitValue: false,
						reset: function() {
							var me = this;
							var dtNow = new Date();
							me.setValue(dtNow.getHours());
						}
					},{
						xtype: "displayfield",
						value: ":"
					},{
						xtype: "combo",
						name: "minute",
						queryMode: "local",
						store: Ext.Array.range(0, 59),
						readOnly: true,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: dtNow.getMinutes(),
						submitValue: false,
						reset: function() {
							var me = this;
							var dtNow = new Date();
							me.setValue(dtNow.getMinutes());
						}
					},{
						xtype: "displayfield",
						value: ":"
					},{
						xtype: "combo",
						name: "second",
						queryMode: "local",
						store: Ext.Array.range(0, 59),
						readOnly: true,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: dtNow.getSeconds(),
						submitValue: false,
						reset: function() {
							var me = this;
							var dtNow = new Date();
							me.setValue(dtNow.getSeconds());
						}
					}]
				},{
					xtype: "button",
					name: "manualupdate",
					disabled: true,
					text: _("Update now"),
					scope: me,
					handler: function() {
						var values = {};
						var fields = [ "date", "hour", "minute",
						  "second" ];
						Ext.Array.each(fields, function(name) {
							var field = this.findField(name);
							values[name] = field.getValue();
						}, this);
						var date = new Date(values.date);
						date.setHours(values.hour, values.minute,
						  values.second);
						// Execute RPC.
						OMV.Rpc.request({
							scope: this,
							callback: function(id, success, response) {
								this.doReload();
							},
							relayErrors: false,
							rpcData: {
								service: "System",
								method: "setDate",
								params: {
									timestamp: date.toUnixTimestamp()
								}
							}
						});
					}
				}]
			}]
		}];
	},

	setValues: function(values) {
		var me = this;
		var dtNow = Ext.Date.parse(values.date.ISO8601, "c");
		Ext.apply(values, {
			now: values.date.local,
			date: Ext.Date.format(dtNow, Ext.form.field.Date.prototype.format),
			hour: dtNow.getHours(),
			minute: dtNow.getMinutes(),
			second: dtNow.getSeconds()
		});
		return me.callParent([ values ]);
	}
});

OMV.WorkspaceManager.registerNode({
	id: "time",
	path: "/system",
	text: _("Date & Time"),
	icon16: "images/clock.png",
	iconSvg: "images/clock.svg",
	position: 20
});

OMV.WorkspaceManager.registerPanel({
	id: "time",
	path: "/system/time",
	text: _("Date & Time"),
	position: 10,
	className: "OMV.module.admin.system.time.Time"
});
