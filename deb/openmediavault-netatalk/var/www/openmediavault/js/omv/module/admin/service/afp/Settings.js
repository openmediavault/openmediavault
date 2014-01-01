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

/**
 * @class OMV.module.admin.service.afp.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.afp.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "AFP",
 	rpcGetMethod: "getSettings",
	rpcSetMethod: "setSettings",

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
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true
			}]
		},{
			xtype: "fieldset",
			title: _("Home directories"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "homesenable",
				fieldLabel: _("Enable"),
				checked: false,
				boxLabel: _("Enable user home directories.")
			}]
		},{
			xtype: "fieldset",
			title: _("Advanced settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "allowclrtxt",
				fieldLabel: _("Allow plain passwords"),
				checked: false,
				boxLabel: _("Allow logins with passwords transmitted in the clear.")
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/afp",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.afp.Settings"
});
