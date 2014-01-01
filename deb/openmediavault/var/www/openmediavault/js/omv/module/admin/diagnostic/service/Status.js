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
// require("js/omv/PluginManager.js")
// require("js/omv/workspace/tab/Panel.js")

/**
 * @class OMV.module.admin.diagnostic.service.Status
 * @derived OMV.workspace.tab.Panel
 */
Ext.define("OMV.module.admin.diagnostic.service.Status", {
	extend: "OMV.workspace.tab.Panel",

	initComponent: function() {
		var me = this;
		var plugins = OMV.PluginManager.get("diagnostic", "service");
		me.items = [];
		Ext.Array.each(plugins, function(plugin) {
			me.items.push(Ext.create(plugin.className, {
				title: plugin.text
			}));
		});
		me.callParent(arguments);
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "status",
	path: "/diagnostic/service",
	text: _("Services"),
	position: 10,
	className: "OMV.module.admin.diagnostic.service.Status"
});
