/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
// require("js/omv/workspace/dashboard/Dashboard.js")

/**
 * @class OMV.module.admin.system.general.AdminPasswd
 * @derived OMV.workspace.dashboard.Dashboard
 */
Ext.define("OMV.module.admin.dashboard.Dashboard", {
	extend: "OMV.workspace.dashboard.Dashboard",

	getPartAliases: function() {
		// Get the registered dashboard widgets aliases.
		var aliases = Ext.ClassManager.getAliasesByExpression(
		  "part.module.admin.dashboard.part.*");
		return aliases;
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "dashboard",
	path: "/diagnostic",
	text: _("Dashboard"),
	position: 1,
	iconCls: "mdi mdi-view-dashboard",
	className: "OMV.module.admin.dashboard.Dashboard"
});
