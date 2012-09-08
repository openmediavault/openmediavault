/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
// require("js/omv/NavigationPanel.js")

Ext.ns("OMV.Module.Information");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("information", "donate", {
	text: _("Donate"),
	icon: "images/donate.png",
	position: 10
});

/**
 * @class OMV.Module.Information.Donate
 * @derived Ext.Panel
 */
OMV.Module.Information.Donate = function(config) {
	var initialConfig = {
		border: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Information.Donate.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Information.Donate, Ext.Panel, {
	initComponent : function() {
		this.html = this.createBox("OpenMediaVault is free, but costs money and time to produce, support and distribute. This gift to the developer would demonstrate your appreciation of this software and help its future development." +
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
	  OMV.Module.Information.Donate.superclass.initComponent.apply(this,
		arguments);
	},

	createBox : function(msg) {
		return [
			'<div class="x-box-aboutbox">',
			'<div class="x-box-tl"><div class="x-box-tr"><div class="x-box-tc"></div></div></div>',
			'<div class="x-box-ml"><div class="x-box-mr"><div class="x-box-mc"><h3 class="x-icon-text"></h3>', msg, '</div></div></div>',
			'<div class="x-box-bl"><div class="x-box-br"><div class="x-box-bc"></div></div></div>',
			'</div>'
		].join('');
	}
});
OMV.NavigationPanelMgr.registerPanel("information", "donate", {
	cls: OMV.Module.Information.Donate
});
