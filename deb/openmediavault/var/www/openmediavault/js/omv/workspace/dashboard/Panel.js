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
// require("js/omv/workspace/panel/Panel.js")

/**
 * Display the child nodes of a workspace category in a data view.
 * @class OMV.workspace.dashboard.Panel
 * @derived OMV.workspace.panel.Panel
 */
Ext.define("OMV.workspace.dashboard.Panel", {
	extend: "OMV.workspace.panel.Panel",
	alias: "widget.workspace.dashboard.panel",
	requires: [
		"Ext.menu.Menu",
	],

	layout: {
		type: "absolute"
	},
	stateful: true,
	stateId: "b1710202-e0ea-11e3-ad39-0002b3a176b4",
	cls: Ext.baseCSSPrefix + "workspace-dashboard",
	hideTopToolbar: false,
	hideRefreshButton: true,

	getTopToolbarItems: function(c) {
		var me = this;
		var items = me.callParent(arguments);
		var menu = Ext.create("Ext.menu.Menu", {
			listeners: {
				scope: me,
				click: function(menu, item, e, eOpts) {
					// Create the dashboard widget and add it to the
					// dashboard panel. Display the widget settings
					// dialog if it has one.
					var widget = Ext.create(item.value);
					this.addWidget(widget, true);
					this.doLayout(true, false);
					widget.show();
					widget.onSettings();
				}
			}
		});
		// Get the registered widgets and initialize them.
		var classes = Ext.ClassManager.getNamesByExpression(
		  "omv.widget.diagnostic.dashboard.*");
		Ext.Array.each(classes, function(name) {
			var widget = Ext.create(name);
			if (!Ext.isObject(widget) || !widget.isDashboardWidget)
				return;
			menu.add({
				text: widget.title,
				value: name
			});
		});
		// Insert the combobox showing all registered dashboard widgets.
		Ext.Array.insert(items, 0, [{
			id: me.getId() + "-add",
			text: _("Add"),
			icon: "images/add.png",
			menu: menu
		}]);
		return items;
	},

	/**
	 * Add the widget to the dashboard.
	 * @param widget The widget to add.
	 * @param saveState Set to TRUE to save the widget state after it
	 *   has been added to the dashboard.
	 */
	addWidget: function(widget, saveState) {
		var me = this;
		if (Ext.isEmpty(widget) || !widget.isWindow)
			return;
		// Update the widget state for the following events:
		widget.on({
			  scope: me,
			  move: me.saveState,
			  resize: me.saveState,
			  destroy: me.saveState,
			  collapse: me.saveState,
			  expand: me.saveState
		  });
		// Add the widget to the parent panel.
		me.add(widget);
		// Save widget state if necessary.
		if (saveState)
			me.saveState();
	},

	/**
	 * Gets the current state of the dashboard panel. It contains an array
	 * of displayed dashboard widgets.
	 * @return The current state of the object.
	 */
	getState: function() {
		var me = this;
		var state = me.callParent(arguments);
		var widgetStates = [];
		Ext.Array.each(me.query(), function(item) {
			// Ignore various widget states.
			if (!item.rendered || item.destroying || !item.isDashboardWidget)
				return;
			// Get the widget state.
			var itemState = item.getState();
			// Append the dashboard widget config options which are used
			// to restore the widget if the dashboard is rendered.
			Ext.apply(itemState, {
				alias: Ext.isArray(item.alias) ? item.alias[0] : item.alias,
				collapsed: item.collapsed
			});
			widgetStates.push(itemState);
		});
		return Ext.apply(state || {}, {
			widgets: widgetStates
		});
	},

	/**
	 * Applies the state to the dashboard panel.
	 */
	applyState: function(state) {
		var me = this;
		me.callParent(arguments);
		if (!Ext.isDefined(state.widgets) || Ext.isEmpty(state.widgets))
			return;
		Ext.Array.each(state.widgets, function(widget) {
			var widget = Ext.create(widget.alias, widget);
			me.addWidget(widget);
		});
	},

	afterRender: function() {
		var me = this;
		me.callParent(arguments);
		// Show the dashboard widgets.
		Ext.Array.each(me.query(), function(item) {
			if (item.isDashboardWidget) {
				item.show();
			}
		});
	}
});
