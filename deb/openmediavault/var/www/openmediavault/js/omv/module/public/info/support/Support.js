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
// require("js/omv/workspace/panel/Panel.js")

/**
 * @class OMV.module.public.info.support.Support
 * @derived OMV.workspace.panel.Panel
 */
Ext.define("OMV.module.public.info.support.Support", {
	extend: "OMV.workspace.panel.Panel",

	initComponent: function() {
		var me = this;
		me.html = "<form style='overflow: auto; height: 100%;'>";
		me.html += me.createBox(
		  "<b>Project homepage</b><br/><a href='http://www.openmediavault.org' target='_blank'>http://www.openmediavault.org</a><br/><br/>" +
		  "<b>Wiki</b><br/><a href='http://wiki.openmediavault.org' target='_blank'>http://wiki.openmediavault.org</a><br/><br/>" +
		  "<b>Forums</b><br/><a href='http://forums.openmediavault.org' target='_blank'>http://forums.openmediavault.org</a><br/><br/>" +
		  "<b>Documentation</b><br/><a href='http://docs.openmediavault.org' target='_blank'>http://docs.openmediavault.org</a><br/><br/>" +
		  "<b>Bugtracker</b><br/><a href='http://bugtracker.openmediavault.org' target='_blank'>http://bugtracker.openmediavault.org</a><br/><br/>" +
		  "<b>Contributors</b><br/><a href='http://wiki.openmediavault.org/index.php?title=Contributors' target='_blank'>http://wiki.openmediavault.org/index.php?title=Contributors</a><br/>");
		me.html += "</form>";
		me.callParent(arguments);
	},

	createBox: function(msg) {
		return [ '<div class="x-box-aboutbox">', msg, '</div>' ].join('');
	}
});

OMV.WorkspaceManager.registerNode({
	id: "support",
	path: "/info",
	text: _("Support"),
	icon16: "images/help.png",
	iconSvg: "images/help.svg",
	position: 20
});

OMV.WorkspaceManager.registerPanel({
	id: "support",
	path: "/info/support",
	text: _("Support"),
	position: 10,
	className: "OMV.module.public.info.support.Support"
});
