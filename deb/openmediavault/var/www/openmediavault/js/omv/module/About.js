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
OMV.NavigationPanelMgr.registerMenu("information", "about", {
	text: "About",
	icon: "images/about.png",
	position: 30
});

/**
 * @class OMV.Module.Information.About
 * @derived Ext.Panel
 */
OMV.Module.Information.About = function(config) {
	var initialConfig = {
		border: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Information.About.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Information.About, Ext.Panel, {
	initComponent : function() {
		this.html = this.createBox("OpenMediaVault is Copyright © 2009-2011 by Volker Theile (volker.theile@openmediavault.org).<br/>" +
		  "All rights reserved.<br/><br/>" +
		  "OpenMediaVault is free software: you can redistribute it and/or modify " +
		  "it under the terms of the GNU General Public License v3 as published by " +
		  "the Free Software Foundation.<br/><br/>" +
		  "OpenMediaVault is distributed in the hope that it will be useful, " +
		  "but WITHOUT ANY WARRANTY; without even the implied warranty of " +
		  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the " +
		  "GNU General Public License for more details.<br/><br/>" +
		  "You should have received a copy of the GNU General Public License " +
		  "along with OpenMediaVault. If not, see &lt;<a href='http://www.gnu.org/licenses' " +
		  "target='_blank'>http://www.gnu.org/licenses</a>&gt;.");
	  OMV.Module.Information.About.superclass.initComponent.apply(this,
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
OMV.NavigationPanelMgr.registerPanel("information", "about", {
	cls: OMV.Module.Information.About
});
