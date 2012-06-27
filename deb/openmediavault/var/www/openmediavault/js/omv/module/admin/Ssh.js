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
// require("js/omv/DiagPanel.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "ssh", {
	text: _("SSH"),
	icon: "images/ssh.png"
});

/**
 * @class OMV.Module.Services.SSH
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.SSH = function(config) {
	var initialConfig = {
		rpcService: "SSH"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.SSH.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Services.SSH, OMV.FormPanelExt, {
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
				inputValue: 1
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 0,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 22
			},{
				xtype: "checkbox",
				name: "permitrootlogin",
				fieldLabel: _("Permit root login"),
				checked: true,
				inputValue: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Specifies whether it is allowed to login as superuser.")
			},{
				xtype: "checkbox",
				name: "passwordauthentication",
				fieldLabel: _("Password authentication"),
				checked: true,
				inputValue: 1,
				boxLabel: _("Enable keyboard-interactive authentication")
			},{
				xtype: "checkbox",
				name: "tcpforwarding",
				fieldLabel: _("TCP forwarding"),
				inputValue: 1,
				boxLabel: _("Permit to do SSH tunneling")
			},{
				xtype: "checkbox",
				name: "compression",
				fieldLabel: _("Compression"),
				inputValue: 1,
				boxLabel: _("Enable compression"),
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Compression is worth using if your connection is slow. The efficiency of the compression depends on the type of the file, and varies widely. Useful for internet transfer only.")
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				autoCreate: {
					tag: "textarea",
					autocomplete: "off",
					rows: "7",
					cols: "65"
				},
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Please check the <a href='http://www.openbsd.org/cgi-bin/man.cgi?query=sshd_config&sektion=5' target='_blank'>manual page</a> for more details."),
				anchor: "100%"
			}]
		},{
			xtype: "fieldset",
			title: _("DNS Service Discovery"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "dnssdenable",
				fieldLabel: _("Enable"),
				checked: true,
				inputValue: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Advertise this service via mDNS/DNS-SD.")
			},{
				xtype: "textfield",
				name: "dnssdname",
				fieldLabel: _("Name"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("The service name."),
				value: "%h - SSH"
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "ssh", {
	cls: OMV.Module.Services.SSH
});

OMV.Module.Services.SSHDiagPanel = function(config) {
	var initialConfig = {
		title: _("SSH"),
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
	OMV.Module.Services.SSHDiagPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.SSHDiagPanel, OMV.DiagPanel, {
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
		  }, this, "SSH", "getStats");
	}
});
OMV.preg("sysinfo", "service", OMV.Module.Services.SSHDiagPanel);
