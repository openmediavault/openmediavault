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
				xtype: "combo",
				name: "powerbtn",
				fieldLabel: _("Power button"),
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: [
						[ "nothing", _("Nothing") ],
						[ "shutdown", _("Shutdown") ],
						[ "standby", _("Standby") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "nothing",
				plugins: [{
					ptype: "fieldinfo",
					text: _("The action to be done when pressing the power button.")
				}]
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
