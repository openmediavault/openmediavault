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
 * @class OMV.workspace.dashboard.Part
 * @derived Ext.dashboard.Part
 * @param title The title text to be used to display in the panel header.
 * @param icon Path to image for an icon.
 * @param viewXType The xtype of the component to be displayed in the view.
 * @param showAtFirstStartup Set to TRUE to display the widget by default
 *   if the dashboard panel is displayed the first time or the cookie has
 *   been cleared. Defaults to FALSE.
 */
Ext.define("OMV.workspace.dashboard.Part", {
	extend: "Ext.dashboard.Part",

	config: {
		icon: "",
		iconCls: "mdi mdi-widgets",
		title: "",
		viewXType: "",
		showAtFirstStartup: false
	},

	constructor: function(config) {
		var me = this;
		me.callParent([ config ]);
		me.setConfig("viewTemplate", me.buildViewTemplate());
	},

	buildViewTemplate: function() {
		var me = this;
		var icon = me.getIcon();
		var iconCls = me.getIconCls();
		if (!Ext.isEmpty(icon)) {
			iconCls = Ext.baseCSSPrefix + "workspace-dashboard-widget-icon"
		}
		return {
			frame: false,
			icon: icon,
			iconCls: iconCls,
			title: me.getTitle(),
			items: [{
				xtype: me.getViewXType()
			}]
		}
	},

	/**
	 * Helper function to generate a type ID based on the class name.
	 * This is used by the Ext.dashboard.Dashboard 'parts' config.
	 * @private
	 * @return The type ID.
	 */
	getType: function() {
		var me = this;
		var type = Ext.getClassName(me).toLowerCase();
		return type.replace(/\./g, "");
	}
});
