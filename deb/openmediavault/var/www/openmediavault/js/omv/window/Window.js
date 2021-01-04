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

/**
 * @ingroup webgui
 * @class OMV.window.Window
 * @derived Ext.window.Window
 * A specialized panel intended for use as an application window.
 */
Ext.define("OMV.window.Window", {
	extend: "Ext.window.Window",

	config: {
		helpId: null,
		enableResponsiveness: true
	},

	cls: OMV.baseCSSPrefix + "window",
	stateful: true,

	initComponent: function() {
		var me = this;
		me.initResponsiveConfig();
		me.callParent(arguments);
	},

	initResponsiveConfig: function() {
		var me = this;
		if (!me.getEnableResponsiveness())
			return;
		Ext.apply(me, {
			responsiveConfig: {
				"phone || tablet || touch": {
					maximized: true,
					maximizable: true,
					stateful: false
				}
			}
		});
		if (!Ext.isDefined(me.plugins))
			me.plugins = [];
		if (!Ext.isArray(me.plugins))
			me.plugins = [me.plugins];
		Ext.Array.push(me.plugins, "responsive");
	},

	getStateId: function() {
		var me = this;
		var stateId = md5(Ext.getClassName(me));
		return stateId;
	}
});
