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
// require("js/omv/workspace/form/Panel.js")

/**
 * @class OMV.module.admin.system.update.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.system.update.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "Apt",
	rpcGetMethod: "getSettings",
	rpcSetMethod: "setSettings",

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("Install updates from"),
			fieldDefaults: {
				labelSeparator: "",
				hideLabel: true
			},
			items: [{
				xtype: "checkbox",
				name: "proposed",
				checked: false,
				boxLabel: _("Pre-released updates.")
			},{
				xtype: "checkbox",
				name: "partner",
				checked: false,
				boxLabel: _("Community-maintained updates.")
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/system/update",
	text: _("Settings"),
	position: 20,
	className: "OMV.module.admin.system.update.Settings"
});
