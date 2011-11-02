/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
OMV.NavigationPanelMgr.registerMenu("information", "support", {
	text: "Support",
	icon: "images/support.png",
	position: 20
});

/**
 * @class OMV.Module.Information.Support
 * @derived Ext.Panel
 */
OMV.Module.Information.Support = function(config) {
	var initialConfig = {
		border: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Information.Support.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Information.Support, Ext.Panel, {
	initComponent : function() {
		this.html = this.createBox(
		  "Project homapage<br/><a href='http://www.openmediavault.org' target='_blank'>http://www.openmediavault.org</a><br/><br/>" +
		  "FAQ<br/><a href='http://www.openmediavault.org/faq.html' target='_blank'>http://www.openmediavault.org/faq.html</a><br/><br/>" +
		  "Wiki<br/><a href='http://wiki.openmediavault.org' target='_blank'>http://wiki.openmediavault.org</a><br/><br/>" +
		  "Forums<br/><a href='http://forums.openmediavault.org' target='_blank'>http://forums.openmediavault.org</a><br/><br/>" +
		  "Mailing lists<br/><a href='http://lists.openmediavault.org' target='_blank'>http://lists.openmediavault.org</a><br/><br/>" +
		  "Bugtracker<br/><a href='http://bugtracker.openmediavault.org' target='_blank'>http://bugtracker.openmediavault.org</a><br/><br/>");
	  OMV.Module.Information.Support.superclass.initComponent.apply(this,
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
OMV.NavigationPanelMgr.registerPanel("information", "support", {
	cls: OMV.Module.Information.Support
});
