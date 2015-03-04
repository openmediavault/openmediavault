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

/**
 * @class OMV.workspace.dashboard.Widget
 * @derived Ext.panel.Panel
 * @param refreshInterval The frequency in milliseconds in which the widget
 *   updates the shown content. Defaults to 0.
 * @param hideSettings Set to FALSE to show the settings button in the
 *   tool header. Defaults to TRUE.
 * @param showAtFirstStartup Set to TRUE to display the widget by default
 *   if the dashboard panel is displayed the first time or the cookie has
 *   been cleared. Defaults to FALSE.
 */
Ext.define("OMV.workspace.dashboard.Widget", {
	extend: "Ext.panel.Panel",

	isDashboardWidget: true,

	refreshInterval: 0,
	hideSettings: true,
	showAtFirstStartup: false,

	header: false,
	border: false,
	layout: "fit",
	height: 200,

	onBoxReady: function() {
		var me = this;
		if ((me.refreshInterval > 0) && Ext.isEmpty(me.refreshTask)) {
			me.refreshTask = Ext.util.TaskManager.start({
				run: me.doRefresh,
				scope: me,
				interval: me.refreshInterval,
				fireOnStart: true
			});
		}
		me.callParent(arguments);
	},

	beforeDestroy: function() {
		var me = this;
		// Stop a running task?
		if (!Ext.isEmpty(me.refreshTask) && (me.refreshTask.isTask)) {
			Ext.util.TaskManager.stop(me.refreshTask);
			delete me.refreshTask;
		}
		me.callParent();
	},

	/**
	 * The function that is called to reload the widget content.
	 */
	doRefresh: Ext.emptyFn,

	getType: function() {
		var me = this;
		var type = me.alias[0];
		return type.replace(/\./g, "");
	}
});
