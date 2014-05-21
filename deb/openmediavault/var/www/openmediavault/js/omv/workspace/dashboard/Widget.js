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
// require("js/omv/window/Window.js")

/**
 * Display the child nodes of a workspace category in a data view.
 * @class OMV.workspace.dashboard.Panel
 * @derived OMV.workspace.panel.Panel
 * @param refreshInterval The frequency in milliseconds in which the widget
 *   updates the shown content. Defaults to 0.
 * @param hideSettings Set to FALSE to show the settings button in the
 *   tool header. Defaults to TRUE.
 */
Ext.define("OMV.workspace.dashboard.Widget", {
	extend: "OMV.window.Window",
	uses: [
		"Ext.MessageBox"
	],

	isDashboardWidget: true,
	refreshInterval: 0,
	hideSettings: true,

	layout: "fit",
	collapsible: true,
	expandOnShow: false,
	constrainHeader: true,
	closable: false,
	width: 350,
	height: 200,
	cls: Ext.baseCSSPrefix + "workspace-dashboard-widget",

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		me.initTools();
		// Add the 'Settings' tool button.
		if (false === me.hideSettings) {
			me.addTool({
				type: "gear",
				tooltip: _("Settings"),
				scope: me,
				handler: me.onSettings
			});
		}
		// Add 'Refresh' toolbar icon.
		if (0 < me.refreshInterval) {
			me.addTool({
				type: "refresh",
				tooltip: _("Refresh"),
				scope: me,
				handler: me.onRefresh
			});
		}
		// Add the 'Trashcan' tool button.
		me.addTool({
			type: "unpin",
			tooltip: _("Remove"),
			handler: function() {
				Ext.MessageBox.show({
					title: _("Confirmation"),
					icon: Ext.MessageBox.QUESTION,
					msg: _("Do you really want to remove this widget?"),
					buttons: Ext.MessageBox.YESNO,
					scope: this,
					fn: function(answer) {
						if ("yes" !== answer)
							return;
						this.close();
					}
				});
			},
			scope: me
		});
	},

	afterRender: function() {
		var me = this;
		if ((me.refreshInterval > 0) && Ext.isEmpty(me.refreshTask)) {
			me.refreshTask = Ext.util.TaskManager.start({
				run: me.onRefresh,
				scope: me,
				interval: me.refreshInterval,
				fireOnStart: true
			});
		}
		me.callParent();
	},

	beforeDestroy: function() {
		var me = this;
		if (!Ext.isEmpty(me.refreshTask) && (me.refreshTask.isTask)) {
			Ext.util.TaskManager.stop(me.refreshTask);
			delete me.refreshTask;
		}
		me.callParent();
	},

	/**
	 * The function that is called when the 'Settings' icon in the
	 * widget header has been pressed.
	 */
	onSettings: Ext.emptyFn,

	/**
	 * The function that is called to reload the widget content.
	 */
	onRefresh: Ext.emptyFn
});
