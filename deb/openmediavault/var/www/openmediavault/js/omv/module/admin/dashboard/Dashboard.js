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
// require("js/omv/workspace/dashboard/Panel.js")

/**
 * @class OMV.module.admin.system.general.AdminPasswd
 * @derived OMV.workspace.dashboard.Panel
 */
Ext.define("OMV.module.admin.dashboard.Dashboard", {
	extend: "OMV.workspace.dashboard.Panel",

	getWidgetClasses: function() {
		// Get the registered dashboard widgets classes.
		var classNames = Ext.ClassManager.getNamesByExpression(
		  "omv.widget.dashboard.*");
		return classNames;
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "dashboard",
	path: "/diagnostic",
	text: _("Dashboard"),
	position: 1,
	icon16: "images/grid.png",
	iconSvg: "images/grid.svg",
	className: "OMV.module.admin.dashboard.Dashboard"
});
