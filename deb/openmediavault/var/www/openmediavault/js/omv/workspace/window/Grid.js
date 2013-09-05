/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
// require("js/omv/workspace/window/Container.js")
// require("js/omv/grid/Panel.js")

/**
 * @ingroup webgui
 * A workspace window displaying a grid panel.
 * @class OMV.workspace.window.Grid
 * @derived OMV.workspace.window.Container
 */
Ext.define("OMV.workspace.window.Grid", {
	extend: "OMV.workspace.window.Container",
	requires: [
		"OMV.grid.Panel"
	],

	gridClassName: "OMV.grid.Panel",
	gridConfig: {},
	autoLoadData: false, // Done by the store

	getWindowItems: function() {
		var me = this;
		me.gp = me.initGrid();
		return me.gp;
	},

	/**
	 * Initialize the tab panel displayed in this window.
	 * @return The tab panel object.
	 */
	initGrid: function() {
		var me = this;
		return Ext.create(me.gridClassName, Ext.apply({
			activeTab: 0,
			layoutOnTabChange: true,
			enableTabScroll: true,
			defaults: {
				readOnly: me.readOnly
			}
		}, me.getGridConfig(me)));
	},

	/**
	 * Returns additional grid configuration options.
	 * @param c This component object.
	 * @return The grid panel configuration object.
	 */
	getGridConfig: function(c) {
		return this.gridConfig;
	},

	/**
	 * Returns the tab panel.
	 * @return The tab panel object.
	 */
	getGrid: function() {
		return this.gp;
	},

	/**
	 * Checks if any data in this grid have changed from their original
	 * values.
	 * @return Returns TRUE if any data in this grid have changed from
	 *   their original values.
	 */
	isDirty: function() {
		var me = this;
		return me.getGrid().isDirty();
	},

	/**
	 * Loads an array of data straight into the grid.
	 * @param values The values to load into the grids store.
	 * @return None.
	 */
	setValues: function(values) {
		var me = this;
		var store = me.getGrid().getStore();
		store.loadData(values);
	},

	/**
	 * Get the data from the grid.
	 * @return The records from the grids store as key/value pairs.
	 */
	getValues: function() {
		var me = this;
		return me.getGrid().getValues();
	},

	/**
	 * Method that is called when the 'Reset' button is pressed. Reject
	 * all changes done in the grid.
	 * @param this The window itself.
	 */
	onResetButton: function() {
		var me = this;
		var store = me.getGrid().getStore();
		store.rejectChanges();
	}
});
