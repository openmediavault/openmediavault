/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
// require("js/omv/workspace/tab/Panel.js")

// require("js/omv/ModuleManager.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/field/plugin/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Append vtypes
Ext.apply(Ext.form.VTypes, {
	upsname: function(v) {
		return /^[a-zA-Z0-9_-]+$/.test(v);
	},
	upsnameText: "Invalid identifier",
	upsnameMask: /[a-z0-9_-]/i
});

// Register the menu.
OMV.ModuleManager.registerMenu("services", "nut", {
	text: _("UPS"),
	icon16: "images/nut.png"
});

/**
 * @class OMV.Module.Services.NUT
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.NUT = function(config) {
	var initialConfig = {
		rpcService: "Nut"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NUT.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.NUT, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.NUT.superclass.initComponent.apply(this, arguments);
		this.on("load", this.updateFormFields, this);
	},

	getFormItems : function() {
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
				checked: false,
				listeners: {
					check: function(comp, checked) {
						this.updateFormFields();
					},
					scope: this
				}
			},{
				xtype: "textfield",
				name: "upsname",
				fieldLabel: _("Identifier"),
				allowBlank: false,
				vtype: "upsname",
				plugins: [{
					ptype: "fieldinfo",
					text: _("The name used to uniquely identify your UPS on this system.")
				}]
			},{
				xtype: "textfield",
				name: "comment",
				fieldLabel: _("Comment"),
				allowBlank: true,
				width: 400
			},{
				xtype: "textfield",
				name: "driverconf",
				fieldLabel: _("Driver configuration directives"),
				allowBlank: false,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "7",
					cols: "65"
				},
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
					fields: [ "value","text" ],
					data: [
						[ "fsd",_("UPS reaches low battery") ],
						[ "onbatt",_("UPS goes on battery") ]
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
				}],
				listeners: {
					select: function(combo, record, index) {
						this.updateFormFields();
					},
					scope: this
				}
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
				boxLabel: _("Enable remote monitoring of the local connected UPS."),
				listeners: {
					check: this.updateFormFields,
					scope: this
				}
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
		for(i = 0; i < fields.length; i++) {
			var c = this.findField(fields[i]);
			if(!Ext.isEmpty(c)) {
				var visible = (shutdownmode === "onbatt");
				c.allowBlank = !visible;
				c.setVisible(visible);
			}
		}
		// Update remote monitoring settings
		var remotemonitor = this.findField("remotemonitor").checked;
		fields = [ "remoteuser", "remotepassword" ];
		for(var i = 0; i < fields.length; i++) {
			var c = this.findField(fields[i]);
			if(!Ext.isEmpty(c)) {
				c.allowBlank = !(enable && remotemonitor);
				c.setReadOnly(!remotemonitor);
			}
		}
	}
});
OMV.ModuleManager.registerPanel("services", "nut", {
	cls: OMV.Module.Services.NUT
});
