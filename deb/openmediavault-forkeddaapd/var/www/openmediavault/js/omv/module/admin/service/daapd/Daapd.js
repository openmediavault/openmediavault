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
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.daapd.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.daapd.Settings", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.form.field.SharedFolderComboBox"
	],

	rpcService: "ForkedDaapd",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: "password",
			conditions: [
				{ name: "passwordrequired", value: true }
			],
			properties: [
				"!allowBlank",
				"!readOnly"
			]
		},{
			name: "sharedfolderref",
			conditions: [
				{ name: "enable", value: true }
			],
			properties: "!allowBlank"
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
				xtype: "textfield",
				name: "libraryname",
				fieldLabel: _("Library name"),
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The name of the library as displayed by the clients.")
				}]
			},{
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 0,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 3689,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Port to listen on.")
				}]
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				fieldLabel: _("Shared folder"),
				allowNone: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The location of the media files to share.")
				}]
			},{
				xtype: "checkbox",
				name: "passwordrequired",
				fieldLabel: _("Authentication"),
				checked: false,
				boxLabel: _("A password is required to access the library.")
			},{
				xtype: "passwordfield",
				name: "password",
				fieldLabel: _("Password"),
				allowBlank: true
			}]
		}];
	}
});

OMV.WorkspaceManager.registerNode({
	id: "daapd",
	path: "/service",
	text: _("iTunes/DAAP"),
	icon16: "images/forkeddaapd.png",
	iconSvg: "images/forkeddaapd.svg"
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/daapd",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.daapd.Settings"
});
