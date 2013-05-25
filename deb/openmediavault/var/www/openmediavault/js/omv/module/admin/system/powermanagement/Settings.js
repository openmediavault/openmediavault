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
 * @class OMV.module.admin.system.powermanagement.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.system.powermanagement.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "PowerMgmt",

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("General Settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "cpufreq",
				fieldLabel: _("Monitoring"),
				checked: true,
				boxLabel: _("Specifies whether to monitor the system status and select the most appropriate CPU level.")
			},{
				xtype: "checkbox",
				name: "powerbtn",
				fieldLabel: _("Power button"),
				checked: false,
				boxLabel: _("Shutdown the system when pressing the power button.")
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/system/powermanagement",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.system.powermanagement.Settings"
});
