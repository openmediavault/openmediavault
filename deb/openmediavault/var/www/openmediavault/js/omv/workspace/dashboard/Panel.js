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

	layout: {
		type: "absolute"
	},
	stateful: true,
	cls: Ext.baseCSSPrefix + "workspace-dashboard",
	hideTopToolbar: false,
	hideRefreshButton: true,

	getTopToolbarItems: function(c) {
		var me = this;
		var items = me.callParent(arguments);
		Ext.Array.insert(items, 0, [{
			id: me.getId() + "-add",
			xtype: "combo",
			text: _("Add"),
			queryMode: "local",
			store: [],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			listeners: {
				scope: me,
				select: function(combo, records, eOpts) {
				}
			}
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
			  destroy: me.saveState
		  });
		// Add the widget to the parent panel.
		me.add(widget);
		// Save widget state if necessary.
		if (saveState)
			me.saveState();
	}
});
