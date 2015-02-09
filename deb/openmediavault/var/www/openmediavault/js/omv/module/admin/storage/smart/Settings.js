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
 * @class OMV.module.admin.storage.smart.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.storage.smart.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "Smart",
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
				xtype: "numberfield",
				name: "interval",
				fieldLabel: _("Check interval"),
				minValue: 10,
				allowDecimals: false,
				allowBlank: false,
				value: 1800,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Sets the interval between disk checks to N seconds.")
				}]
			},{
				xtype: "combo",
				name: "powermode",
				fieldLabel: _("Power mode"),
				queryMode: "local",
				store: [
					[ "never", _("Never") ],
					[ "sleep", _("Sleep") ],
					[ "standby", _("Standby") ],
					[ "idle", _("Idle") ]
				],
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "standby",
				plugins: [{
					ptype: "fieldinfo",
					text: _("Prevent a disk from being spun-up when it is periodically polled.<br/><ul><li>Never - Poll (check) the device regardless of its power mode. This may cause a disk which is spun-down to be spun-up when it is checked.</li><li>Sleep - Check the device unless it is in SLEEP mode.</li><li>Standby - Check the device unless it is in SLEEP or STANDBY mode. In these modes most disks are not spinning, so if you want to prevent a disk from spinning up each poll, this is probably what you want.</li><li>Idle - Check the device unless it is in SLEEP, STANDBY or IDLE mode. In the IDLE state, most disks are still spinning, so this is probably not what you want.</li></ul>")
				}]
			
			}]
		},{
			xtype: "fieldset",
			title: _("Temperature monitoring"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "numberfield",
				name: "tempdiff",
				fieldLabel: _("Difference"),
				width: 40,
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 0,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Report if the temperature had changed by at least N degrees Celsius since last report. Set to 0 to disable this report.")
				}]
			},{
				xtype: "numberfield",
				name: "tempinfo",
				fieldLabel: _("Informal"),
				width: 40,
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 0,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Report if the temperature is greater or equal than N degrees Celsius. Set to 0 to disable this report.")
				}]
			},{
				xtype: "numberfield",
				name: "tempcrit",
				fieldLabel: _("Critical"),
				width: 40,
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 0,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Report if the temperature is greater or equal than N degrees Celsius. Set to 0 to disable this report.")
				}]
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/storage/smart",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.storage.smart.Settings"
});
