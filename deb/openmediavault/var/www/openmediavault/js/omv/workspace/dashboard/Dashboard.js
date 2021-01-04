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
 * Display the child nodes of a workspace category in a data view.
 * @class OMV.workspace.dashboard.Dashboard
 * @derived Ext.dashboard.Dashboard
 */
Ext.define("OMV.workspace.dashboard.Dashboard", {
	extend: "Ext.dashboard.Dashboard",
	requires: [
		"Ext.menu.Menu"
	],

	border: false,
	stateful: true,
	stateId: "68f8e3e8-c288-11e4-98d4-0002b3a176b4",
	plugins: "responsive",
	responsiveConfig: {
		// On phone only one column is used.
		phone: {
			columnWidths: [ 1.0 ],
			maxColumns: 1
		}
	},
	columnWidths: [
		0.50,
		0.50
	],

	initComponent: function() {
		var me = this;
		// Initialize the top toolbar.
		me.dockedItems = [];
		me.dockedItems.push(me.topToolbar = Ext.widget({
			xtype: "toolbar",
			dock: "top",
			items: me.getTopToolbarItems(me)
		}));
		// Setup the dashboard widgets that can be displayed.
		var aliases = me.getPartAliases();
		var parts = {};
		Ext.Array.each(aliases, function(alias) {
			var part = Ext.create(alias);
			if (!Ext.isObject(part) || !part.isPart)
				return;
			var type = part.getType();
			parts[type] = alias.replace(/^part\./, "");
		}, me);
		me.setParts(parts);
		me.callParent(arguments);
	},

	getPartAliases: function() {
		return [];
	},

	getTopToolbarItems: function(c) {
		var me = this;
		var items = [];
		// Get the registered dashboard widget aliases and fill up the
		// menu which displayes the available dashboard widgets.
		var aliases = me.getPartAliases();
		// Create the menu items.
		var menuItems = [];
		Ext.Array.each(aliases, function(alias) {
			var part = Ext.create(alias);
			if (!Ext.isObject(part) || !part.isPart)
				return;
			var menuItem = {
				text: part.getTitle(),
				type: part.getType()
			};
			if (!Ext.isEmpty(part.getIcon())) {
				Ext.apply(menuItem, {
					icon: part.getIcon(),
					iconCls: Ext.baseCSSPrefix + "menu-item-icon-16x16"
				});
			} else if (!Ext.isEmpty(part.getIconCls())) {
				Ext.apply(menuItem, {
					iconCls: part.getIconCls()
				});
			}
			menuItems.push(menuItem);
		});
		Ext.Array.sort(menuItems, function(a, b) {
			return a.text > b.text ? 1 : (a.text < b.text ? -1 : 0);
		});
		// Insert the combobox showing all registered dashboard widgets.
		Ext.Array.insert(items, 0, [{
			id: me.getId() + "-add",
			text: _("Add"),
			iconCls: "x-fa fa-plus",
			menu: Ext.create("Ext.menu.Menu", {
				items: menuItems,
				listeners: {
					scope: me,
					click: function(menu, item, e, eOpts) {
						this.addNew(item.type);
					}
				}
			})
		}]);
		return items;
	},

	/**
	 * Initializes the state of the object upon construction.
	 */
	initState: function() {
		var me = this;
		me.callParent(arguments);
		// Check if the dashboard is displayed the first time. If this
		// is the case, then automatically display some widgets by default.
		var id = me.getStateId();
		var state = Ext.state.Manager.get(id);
		if (!(Ext.isDefined(state) && Ext.isObject(state))) {
			// Get all registered dashboard widgets.
			var aliases = me.getPartAliases();
			Ext.Array.each(aliases, function(alias) {
				var part = Ext.create(alias);
				if (!Ext.isObject(part) || !part.isPart)
					return;
				// Show the widget at startup (e.g. displaying the UI the
				// first time or clearing the cookie)?
				if (!part.getShowAtFirstStartup())
					return;
				this.addNew(part.getType());
			}, me);
		}
	}
});
