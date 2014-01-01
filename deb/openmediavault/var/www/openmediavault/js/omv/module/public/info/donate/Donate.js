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
 * @class OMV.module.public.info.donate.Donate
 * @derived OMV.workspace.panel.Panel
 */
Ext.define("OMV.module.public.info.donate.Donate", {
	extend: "OMV.workspace.panel.Panel",

	initComponent: function() {
		var me = this;
		me.html = me.createBox("OpenMediaVault is free, but costs money and time to produce, support and distribute. This gift to the developer would demonstrate your appreciation of this software and help its future development." +
		  "<br/>" + 
		  "To help OpenMediaVault in a monetary way, you can show your appreciation with a donation via PayPal." +
		  "<br/><br/>" +
		  "<form action='https://www.paypal.com/cgi-bin/webscr' method='post' target='_blank'>" +
		  "<input type='hidden' name='cmd' value='_s-xclick'>" +
		  "<input type='hidden' name='hosted_button_id' value='95MF5UQ66PW2E'>" +
		  "<div class='x-text-center'>" +
		  "<input type='image' src='images/donate-btn.gif' border='0' name='submit' alt='PayPal - The safer, easier way to pay online!'>" +
		  "</div>" +
		  "</form>");
		me.callParent(arguments);
	},

	createBox: function(msg) {
		return [ '<div class="x-box-aboutbox">', msg, '</div>' ].join('');
	}
});

OMV.WorkspaceManager.registerNode({
	id: "donate",
	path: "/info",
	text: _("Donate"),
	icon16: "images/donate.png",
	iconSvg: "images/donate.svg",
	position: 10
});

OMV.WorkspaceManager.registerPanel({
	id: "donate",
	path: "/info/donate",
	text: _("Donate"),
	position: 10,
	className: "OMV.module.public.info.donate.Donate"
});
