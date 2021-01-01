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
 * @class OMV.workspace.dashboard.View
 * @derived Ext.panel.Panel
 * @param refreshInterval The frequency in milliseconds in which the widget
 *   updates the shown content. Defaults to 0.
 */
Ext.define("OMV.workspace.dashboard.View", {
	extend: "Ext.panel.Panel",

	config: {
		refreshInterval: 0
	},

	border: true,
	layout: "fit",
	height: 200,

	onBoxReady: function() {
		var me = this;
		var interval = me.getRefreshInterval();
		if ((interval > 0) && !Ext.isDefined(me.refreshTask) &&
			Ext.isFunction(me.doRefresh))
		{
			me.refreshTask = Ext.util.TaskManager.newTask({
				run: me.doRefresh,
				scope: me,
				interval: interval,
				fireOnStart: true
			});
			me.refreshTask.start();
		}
		me.callParent(arguments);
	},

	destroy: function() {
		var me = this;
		// Stop a running task?
		if (Ext.isDefined(me.refreshTask) && (me.refreshTask.isTask)) {
			me.refreshTask.destroy();
			me.refreshTask = null;
		}
		me.callParent();
	},

	/**
	 * The function that is called to reload the content.
	 */
	doRefresh: Ext.emptyFn
});
