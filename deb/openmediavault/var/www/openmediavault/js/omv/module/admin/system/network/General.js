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
 * @class OMV.module.admin.system.network.General
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.system.network.General", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "Network",
	rpcGetMethod: "getGeneralSettings",
	rpcSetMethod: "setGeneralSettings",

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("General settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "hostname",
				fieldLabel: _("Hostname"),
				vtype: "hostname",
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The hostname is a label that identifies the system to the network.")
				}]
			},{
				xtype: "textfield",
				name: "domainname",
				fieldLabel: _("Domain name"),
				vtype: "domainname",
				allowBlank: true
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "general",
	path: "/system/network",
	text: _("General"),
	position: 10,
	className: "OMV.module.admin.system.network.General"
});
