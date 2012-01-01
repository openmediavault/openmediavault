/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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

Ext.ns("OMV");

/**
 * @class OMV.NavigationPanel
 * @derived Ext.tree.TreePanel
 */
OMV.NavigationPanel = function(config) {
	var initialConfig = {
		rootVisible: false,
		root: {
			text: "root",
			id: "root",
			expanded: true,
			children: []
		}
	};
	Ext.apply(initialConfig, config);
	OMV.NavigationPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.NavigationPanel, Ext.tree.TreePanel, {
	initComponent : function() {
		var categories = OMV.NavigationPanelMgr.getCategories();
		categories.each(function(category) {
			var child = {
				position: category.position,
				text: category.text,
				cls: "folder",
				categoryId: category.id,
				expanded: true,
				hidden: true,
				singleClickExpand: true,
				children: []
			};
			category.items.each(function(menu) {
				var childnode = {
					text: menu.text,
					leaf: true,
					position: menu.position,
					categoryId: category.id,
					menuId: menu.id
				};
				if (!Ext.isEmpty(menu.icon)) {
					childnode.icon = menu.icon;
				}
				child.hidden = false;
				child.children.push(childnode);
			}, this);
			this.root.children.push(child);
		}, this);
		OMV.NavigationPanel.superclass.initComponent.apply(this, arguments);
		// Add tree sorter
		new Ext.tree.TreeSorter(this, {
			folderSort: true,
			dir: "asc",
			property: "position",
			sortType: function(prop) {
				return parseInt(prop);
			}
		});
		// Add event handler
		this.on("afterrender", function() {
			// Select the 'About' menu entry per default
			this.getRootNode().cascade(function(node) {
				if (node.attributes.categoryId === "diagnostics" &&
				  node.attributes.menuId === "sysinfo" &&
				  node.attributes.text === "System Information") {
					node.select();
					this.fireEvent("click", node);
					return false;
				}
			}, this);
		}, this);
	}
});

/**
 * @class OMV.NavigationPanelMgr
 */
OMV.NavigationPanelMgr = function() {
	var categories = new Ext.util.MixedCollection;
	return {
		/**
		 * Register a category, e.g. 'System' or 'Diagnostics'.
		 * @param key The key of the category, e.g. 'system' or 'storage'.
		 * @param properties A object containing the category properties.
		 * The field 'text' is displayed in the navigation tree and panel
		 * title, the field 'position' defines the position within the
		 * navigation tree (optional). It will be set to 100 if not defined.
		 * @return The category added.
		 */
		registerCategory : function(key, properties) {
			var category = {};
			Ext.apply(category, properties, {
				"id": key,
				"position": 100,
				"items": new Ext.util.MixedCollection
			});
			return categories.add(key, category);
		},

		/**
		 * Register a menu in the given category, e.g. 'System/Network'.
		 * @param categoryKey The key of the category, e.g. 'system'.
		 * @param menuKey The key of the menu, e.g. 'network'.
		 * @param properties A object containing the menu properties.
		 * The field 'text' is displayed in the navigation tree and panel
		 * title, the field 'position' defines the position within the
		 * navigation tree (optional). It will be set to 100 if not defined.
		 * The field 'icon' is the path to an icon to display in this menu.
		 * @return The menu added.
		 */
		registerMenu : function(categoryKey, menuKey, properties) {
			var category = categories.get(categoryKey);
			var menu = {};
			Ext.apply(menu, properties, {
				"id": menuKey,
				"icon": null,
				"position": 100,
				"items": new Ext.util.MixedCollection
			});
			return category.items.add(menuKey, menu);
		},

		/**
		 * Register a panel for the given category and menu,
		 * @param categoryKey The key of the category, e.g. 'system'.
		 * @param menuKey The key of the menu, e.g. 'network'.
		 * @param properties A object containing the menu properties.
		 * The field 'title' is displayed in the panel title, the field 'cls'
		 * is the name of the class to create and 'position' defines the
		 * position within the tab panel. It will be set to 10 if not defined.
		 * @return The panel added.
		 */
		registerPanel : function(categoryKey, menuKey, properties) {
			var category = categories.get(categoryKey);
			var menu = category.items.get(menuKey);
			var panel = {};
			Ext.apply(panel, properties, {
				"position": 10
			});
			return menu.items.add(panel.position, panel);
		},

		getCategories : function() {
			return categories;
		},

		getMenu : function(categoryKey, menuKey) {
			var category = categories.get(categoryKey);
			return category.items.get(menuKey);
		}
	};
}();

// Register default categories.
OMV.NavigationPanelMgr.registerCategory("system", {
	text: "System",
	position: 10
});
OMV.NavigationPanelMgr.registerCategory("storage", {
	text: "Storage",
	position: 20
});
OMV.NavigationPanelMgr.registerCategory("privileges", {
	text: "Access Right Management",
	position: 30
});
OMV.NavigationPanelMgr.registerCategory("services", {
	text: "Services",
	position: 40
});
OMV.NavigationPanelMgr.registerCategory("diagnostics", {
	text: "Diagnostics",
	position: 50
});
OMV.NavigationPanelMgr.registerCategory("information", {
	text: "Information",
	position: 60
});
