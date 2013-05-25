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
// require("js/omv/workspace/node/panel/Panel.js")

/**
 * @class OMV.module.public.info.donate.Donate
 * @derived OMV.workspace.node.panel.Panel
 */
Ext.define("OMV.module.public.Overview", {
	extend: "OMV.workspace.node.panel.Panel",
	requires: [
		"OMV.workspace.node.panel.Panel"
	]
});
/*
OMV.WorkspaceManager.registerPanel({
	id: "overview",
	path: "/",
	text: _("Overview"),
	icon16: "images/eye.png",
	position: 0,
	className: "OMV.module.public.Overview"
});
*/
