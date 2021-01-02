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
		  "<b>Project homepage</b><br/><a href='https://www.openmediavault.org' target='_blank'>https://www.openmediavault.org</a><br/><br/>" +
		  "<b>Forum</b><br/><a href='https://forum.openmediavault.org' target='_blank'>https://forum.openmediavault.org</a><br/><br/>" +
		  "<b>Documentation</b><br/><a href='https://docs.openmediavault.org' target='_blank'>https://docs.openmediavault.org</a><br/><br/>" +
		  "<b>API documentation</b><br/><a href='https://apidocs.openmediavault.org' target='_blank'>https://apidocs.openmediavault.org</a><br/><br/>" +
		  "<b>Tracker</b><br/><a href='https://tracker.openmediavault.org' target='_blank'>https://tracker.openmediavault.org</a><br/><br/>");
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
	iconCls: "mdi mdi-help-circle-outline",
	position: 20
});

OMV.WorkspaceManager.registerPanel({
	id: "support",
	path: "/info/support",
	text: _("Support"),
	position: 10,
	className: "OMV.module.public.info.support.Support"
});
