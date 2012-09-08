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
// require("js/omv/PluginMgr.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/DiagPanel.js")
// require("js/omv/RrdGraphPanel.js")

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
OMV.NavigationPanelMgr.registerMenu("services", "nut", {
	text: _("UPS"),
	icon: "images/nut.png"
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
					check: function(comp, checked) {
						this._updateFormFields();
					},
					scope: this
				}
			},{
				xtype: "textfield",
				name: "upsname",
				fieldLabel: _("Identifier"),
				allowBlank: false,
				vtype: "upsname",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The name used to uniquely identify your UPS on this system.")
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
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("To get more information how to configure your UPS please check the Network UPS Tools <a href='http://www.networkupstools.org/docs/man/ups.conf.html' target='_blank'>documentation</a>."),
				anchor: "100%"
			},{
				xtype: "combo",
				name: "shutdownmode",
				hiddenName: "shutdownmode",
				fieldLabel: _("Shutdown mode"),
				mode: "local",
				store: new Ext.data.SimpleStore({
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
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Defines when the shutdown is initiated."),
				listeners: {
					select: function(combo, record, index) {
						this._updateFormFields();
					},
					scope: this
				}
			},{
				xtype: "numberfield",
				name: "shutdowntimer",
				fieldLabel: _("Shutdown timer"),
				minValue: 1,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 30,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The time in seconds until shutdown is initiated. If the UPS happens to come back before the time is up the shutdown is canceled.")
			}]
		},{
			xtype: "fieldset",
			title: _("Remote monitoring"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "remotemonitor",
				fieldLabel: _("Enable"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable remote monitoring of the local connected UPS."),
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "textfield",
				name: "remoteuser",
				fieldLabel: _("Username"),
				allowBlank: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Used for remote authentication.")
			},{
				xtype: "passwordfield",
				name: "remotepassword",
				fieldLabel: _("Password"),
				allowBlank: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Used for remote authentication.")
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var enable = this.findFormField("enable").checked;
		// Update shutdown mode settings
		var shutdownmode = this.findFormField("shutdownmode").getValue();
		var fields = [ "shutdowntimer" ];
		for (i = 0; i < fields.length; i++) {
			var c = this.findFormField(fields[i]);
			if (!Ext.isEmpty(c)) {
				var visible = (shutdownmode === "onbatt");
				c.allowBlank = !visible;
				c.setVisible(visible);
			}
		}
		// Update remote monitoring settings
		var remotemonitor = this.findFormField("remotemonitor").checked;
		fields = [ "remoteuser", "remotepassword" ];
		for (var i = 0; i < fields.length; i++) {
			var c = this.findFormField(fields[i]);
			if (!Ext.isEmpty(c)) {
				c.allowBlank = !(enable && remotemonitor);
				c.setReadOnly(!remotemonitor);
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "nut", {
	cls: OMV.Module.Services.NUT
});

/**
 * @class OMV.Module.Services.NUTDiagPanel
 * @derived Ext.TabPanel
 */
OMV.Module.Services.NUTDiagPanel = function(config) {
	var initialConfig = {
		title: _("UPS"),
		border: false,
		activeTab: 0,
		layoutOnTabChange: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NUTDiagPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.NUTDiagPanel, Ext.TabPanel, {
	initComponent : function() {
		this.items = [
			new OMV.Module.Services.NUTVariablesPanel,
			new OMV.Module.Services.NUTChargeRrdGraphPanel,
			new OMV.Module.Services.NUTLoadRrdGraphPanel,
			new OMV.Module.Services.NUTTemperatureRrdGraphPanel,
			new OMV.Module.Services.NUTVoltageRrdGraphPanel
		];
		OMV.Module.Services.NUTDiagPanel.superclass.initComponent.
		  apply(this, arguments);
	}
});
OMV.preg("sysinfo", "service", OMV.Module.Services.NUTDiagPanel);

/**
 * @class OMV.Module.Services.NUTVariablesPanel
 * @derived OMV.DiagPanel
 */
OMV.Module.Services.NUTVariablesPanel = function(config) {
	var initialConfig = {
		title: _("Variables"),
		layout: "fit",
		items: [{
			id: this.getId() + "-content",
			xtype: "textarea",
			readOnly: true,
			cls: "x-form-textarea-monospaced",
			disabledClass: ""
		}]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NUTVariablesPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.NUTVariablesPanel, OMV.DiagPanel, {
	doLoad : function() {
		OMV.MessageBox.wait(null, _("Loading ..."));
		OMV.Ajax.request(function(id, response, error) {
			  OMV.MessageBox.updateProgress(1);
			  OMV.MessageBox.hide();
			  if (error !== null) {
				  OMV.MessageBox.error(null, error);
			  } else {
				  var comp = this.getComponent(this.getId() + "-content");
				  if (!Ext.isEmpty(comp)) {
					  comp.setValue(response);
				  }
			  }
		  }, this, "Nut", "getStats");
	}
});

/**
 * @class OMV.Module.Services.NUTChargeRrdGraphPanel
 * @derived OMV.RrdGraphPanel
 */
OMV.Module.Services.NUTChargeRrdGraphPanel = function(config) {
	var initialConfig = {
		title: _("Charge"),
		rrdGraphName: "nut-charge"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NUTChargeRrdGraphPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.NUTChargeRrdGraphPanel, OMV.RrdGraphPanel, {
});

/**
 * @class OMV.Module.Services.NUTLoadRrdGraphPanel
 * @derived OMV.RrdGraphPanel
 */
OMV.Module.Services.NUTLoadRrdGraphPanel = function(config) {
	var initialConfig = {
		title: _("Load"),
		rrdGraphName: "nut-load"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NUTLoadRrdGraphPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.NUTLoadRrdGraphPanel, OMV.RrdGraphPanel, {
});

/**
 * @class OMV.Module.Services.NUTTemperatureRrdGraphPanel
 * @derived OMV.RrdGraphPanel
 */
OMV.Module.Services.NUTTemperatureRrdGraphPanel = function(config) {
	var initialConfig = {
		title: _("Temperature"),
		rrdGraphName: "nut-temperature"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NUTTemperatureRrdGraphPanel.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Services.NUTTemperatureRrdGraphPanel, OMV.RrdGraphPanel, {
});

/**
 * @class OMV.Module.Services.NUTVoltageRrdGraphPanel
 * @derived OMV.RrdGraphPanel
 */
OMV.Module.Services.NUTVoltageRrdGraphPanel = function(config) {
	var initialConfig = {
		title: _("Voltage"),
		rrdGraphName: "nut-voltage"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NUTVoltageRrdGraphPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.NUTVoltageRrdGraphPanel, OMV.RrdGraphPanel, {
});
