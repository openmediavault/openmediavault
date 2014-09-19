/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
// require("js/omv/form/field/CertificateComboBox.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.owncloud.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.owncloud.Settings", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.form.field.CertificateComboBox",
		"OMV.form.field.SharedFolderComboBox"
	],

	rpcService: "OwnCloud",
 	rpcGetMethod: "getSettings",
	rpcSetMethod: "setSettings",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: [
				"sharedfolderref",
				"sslcertificateref"
			],
			conditions: [
				{ name: "enable", value: true }
			],
			properties: "!allowBlank"
		},{
			conditions: [
				{ name: "enable", value: true }
			],
			properties: function(valid, field) {
				this.setButtonDisabled("show", !valid);
			}
		}]
	}],

	getButtonItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		// Add 'Show' button to open the ownCloud web interface in
		// a new browser window.
		items.push({
			id: me.getId() + "-show",
			xtype: "button",
			text: _("Show"),
			icon: "images/search.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			disabled: true,
			scope: me,
			handler: function() {
				window.open("/owncloud", "_blank");
			}
		});
		return items;
	},

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
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 1,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 8443,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The port which is used to access the ownCloud web interface.")
				}]
			},{
				xtype: "certificatecombo",
				name: "sslcertificateref",
				fieldLabel: _("Certificate"),
				allowNone: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The SSL certificate.")
				}]
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				fieldLabel: _("Data directory"),
				allowNone: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The location where ownCloud stores its files.")
				}]
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/owncloud",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.owncloud.Settings"
});
