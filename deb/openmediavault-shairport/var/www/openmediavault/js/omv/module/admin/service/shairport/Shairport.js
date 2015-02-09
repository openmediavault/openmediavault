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

/**
 * @class OMV.module.admin.service.shairport.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.shairport.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "Shairport",
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
				xtype: "checkbox",
				name: "passwordrequired",
				fieldLabel: _("Authentication"),
				checked: false,
				boxLabel: _("A password is required to stream audio.")
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
	id: "shairport",
	path: "/service",
	text: _("Shairport"),
	icon16: "images/airplay.png",
	iconSvg: "images/airplay.svg"
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/shairport",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.shairport.Settings"
});
