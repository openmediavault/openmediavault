/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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

// Append vtypes
Ext.apply(Ext.form.field.VTypes, {
	upsname: function(v) {
		return /^[a-zA-Z0-9_-]+$/.test(v);
	},
	upsnameText: "Invalid identifier",
	upsnameMask: /[a-z0-9_-]/i
});

/**
 * @class OMV.module.admin.service.nut.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.nut.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "Nut",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: [
				"remoteuser",
				"remotepassword"
			],
			conditions: [
				{ name: "enable", value: true },
				{ name: "remotemonitor", value: true },
				{ name: "mode", value: "standalone"}
			],
			properties: [
				"!allowBlank",
				"!readOnly"
			]
		},{
			name: "shutdowntimer",
			conditions: [
				{ name: "shutdownmode", value: "onbatt" }
			],
			properties: [
				"!allowBlank",
				"!readOnly"
			]
		},{
			name: [
				"netclienthostname",
				"netclientusername",
				"netclientpassword",
				"powervalue",
			],
			conditions: [
				{ name: "mode", value: "standalone"},
			],
			properties: [
				"allowBlank",
				"!show"
			]
		},{
			name: "driverconf",
			conditions: [
				{ name: "mode", value: "netclient" }
			],
			properties: [
				"allowBlank",
				"!show"
			]
		},{
			name: "remotemonitor",
			conditions: [
				{ name: "mode", value: "netclient" }
			],
			properties: [
				"readOnly",
				"!checked"
			]
		}]
	}],

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("General settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false
			},{
				xtype: "combo",
				name: "mode",
				fieldLabel: _("Mode"),
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: [
						[ "netclient", _("Netclient") ],
						[ "standalone", _("Standalone") ],
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "standalone",
				plugins: [{
					ptype: "fieldinfo",
					text: _("'Standalone' for local UPS, 'Netclient' for remote UPS on a NUT server."),
				}]
			},{
				xtype: "textfield",
				name: "upsname",
				fieldLabel: _("Identifier"),
				allowBlank: false,
				vtype: "upsname",
				plugins: [{
					ptype: "fieldinfo",
					text: _("The name used to uniquely identify your UPS on local or remote system.")
				}]
			},{
				xtype: "textfield",
				name: "comment",
				fieldLabel: _("Comment"),
				allowBlank: true
			},{
				xtype: "textfield",
				name: "netclienthostname",
				fieldLabel: _("Netclient hostname"),
				allowBlank: false
			},{
				xtype: "textfield",
				name: "netclientusername",
				fieldLabel: _("Netclient username"),
				allowBlank: false
			},{
				xtype: "textfield",
				name: "netclientpassword",
				fieldLabel: _("Netclient password"),
				allowBlank: false
			},{
				xtype: "numberfield",
				name: "powervalue",
				fieldLabel: _("Powervalue"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 1,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The number of power supplies that the UPS feeds on this system (usually 1), refer to <a href='https://networkupstools.org/docs/man/upsmon.conf.html' target='_blank'>upsmon.conf(5)</a> for more information.")
				}]
			},{
				xtype: "textarea",
				name: "driverconf",
				fieldLabel: _("Driver configuration directives"),
				allowBlank: false,
				minHeight: 150,
				plugins: [{
					ptype: "fieldinfo",
					text: _("To get more information how to configure your UPS please check the Network UPS Tools <a href='http://www.networkupstools.org/docs/man/ups.conf.html' target='_blank'>documentation</a>."),
				}]
			},{
				xtype: "combo",
				name: "shutdownmode",
				fieldLabel: _("Shutdown mode"),
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: [
						[ "fsd", _("UPS reaches low battery") ],
						[ "onbatt", _("UPS goes on battery") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "onbatt",
				plugins: [{
					ptype: "fieldinfo",
					text: _("Defines when the shutdown is initiated."),
				}]
			},{
				xtype: "numberfield",
				name: "shutdowntimer",
				fieldLabel: _("Shutdown timer"),
				minValue: 1,
				allowDecimals: false,
				allowBlank: false,
				value: 30,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The time in seconds until shutdown is initiated. If the UPS happens to come back before the time is up the shutdown is canceled.")
				}]
			}]
		},{
			xtype: "fieldset",
			title: _("Remote monitoring"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "remotemonitor",
				fieldLabel: _("Enable"),
				checked: false,
				boxLabel: _("Enable remote monitoring of the local connected UPS.")
			},{
				xtype: "textfield",
				name: "remoteuser",
				fieldLabel: _("Username"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Used for remote authentication.")
				}]
			},{
				xtype: "passwordfield",
				name: "remotepassword",
				fieldLabel: _("Password"),
				allowBlank: true,
				autoComplete: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Used for remote authentication.")
				}]
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	updateFormFields: function() {
		var enable = this.findField("enable").checked;
		// Update shutdown mode settings
		var shutdownmode = this.findField("shutdownmode").getValue();
		var fields = [ "shutdowntimer" ];
		for(var i = 0; i < fields.length; i++) {
			var c = this.findField(fields[i]);
			if(!Ext.isEmpty(c)) {
				var visible = (shutdownmode === "onbatt");
				c.allowBlank = !visible;
				c.setVisible(visible);
			}
		}
		// Update remote monitoring settings
		var remotemonitor = this.findField("remotemonitor").checked;
		var standalone = (this.findField("mode") === "standalone");
		fields = [ "remoteuser", "remotepassword" ];
		for(var i = 0; i < fields.length; i++) {
			var c = this.findField(fields[i]);
			if(!Ext.isEmpty(c)) {
				c.allowBlank = !(enable && remotemonitor && standalone);
				c.setReadOnly(!remotemonitor);
			}
		}
	}
});

OMV.WorkspaceManager.registerNode({
	id: "nut",
	path: "/service",
	text: _("UPS"),
	iconCls: "mdi mdi-car-battery"
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/nut",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.nut.Settings"
});
