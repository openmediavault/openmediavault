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
 * @class OMV.grid.Panel
 * @derived Ext.grid.Panel
 * Enhanced grid panel implementation.
 * @param disableLoadMaskOnLoad TRUE to disable the load mask after the
 *   first load has been done.
 * @param maskBody TRUE to mask the document body while loading data.
 *   Defaults to FALSE.
 * @param reloadOnActivate TRUE to automatically reload the grid content
 *   when the component has been visually activated. Defaults to FALSE.
  * @param autoReload TRUE to reload the grid content automatically every n
 *   milliseconds. Defaults to FALSE.
 * @param reloadInterval The frequency in milliseconds with which the grid
 *   content should be reloaded. Defaults to 10 seconds.
 */
Ext.define("OMV.grid.Panel", {
	extend: "Ext.grid.Panel",

	allowDeselect: true,
	columnLines: true,
	rowLines: false,
	forceFit: false,
	viewConfig: {
		markDirty: false,
		scrollable: true,
		stripeRows: true
	},

	maskBody: false,
	disableLoadMaskOnLoad: false,
	reloadOnActivate: false,
	autoReload: false,
	reloadInterval: 10000, // 10 seconds

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		// Mask document body? Let the grid view create the mask object,
		// simply override the target object. The mask object will be
		// created automatically before the grid is rendered, see
		// Ext.view.AbstractView::onRender() for more details.
		var view = me.getView();
		if((true === view.loadMask) && (true === me.maskBody)) {
			view.loadMask = {
				target: OMV.viewport
			};
		}
		// Disable mask after data has been loaded the first time?
		if (me.disableLoadMaskOnLoad) {
			me.store.on({
				single: true,
				load: function(store, records, successful, operation, eOpts) {
					if (Ext.isObject(me.view)) {
						if (Ext.isObject(me.view.loadMask)) {
							me.view.loadMask.bindStore(null);
							delete me.view.loadMask;
						} else {
							me.view.loadMask = false;
						}
					} else {
						Ext.apply(me.viewConfig, {
							loadMask: false
						});
					}
				}
			});
		}
		// Reload grid content when panel is activated?
		if(me.reloadOnActivate) {
			me.on("activate", function() {
				this.doReload();
			}, me);
		}
		// Auto-reload the grid content?
		if(me.autoReload) {
			var fnActivateTask = function() {
				if (Ext.isEmpty(me.reloadTask)) {
					me.reloadTask = Ext.util.TaskManager.newTask({
						run: me.doReload,
						scope: me,
						interval: me.reloadInterval,
						fireOnStart: true
					});
					me.reloadTask.start();
				}
			};
			var fnDeactivateTask = function() {
				if (!Ext.isEmpty(me.reloadTask) && (me.reloadTask.isTask)) {
					me.reloadTask.destroy();
					me.reloadTask = null;
				}
			}
			me.on({
				render: fnActivateTask,
				beforedestroy: fnDeactivateTask,
				activate: fnActivateTask,
				deactivate: fnDeactivateTask,
				scope: me
			});
		}
	},

	/**
	 * Get the grid's selected record. If more rows are selected the first
	 * one is returned.
	 * @return The selected record, otherwise NULL.
	 */
	getSelected: function() {
		var me = this;
		var records = me.getSelection();
		if (!Ext.isArray(records))
			return null;
		return records[0];
	},

	/**
	 * Returns true if any record of this grid has been changed from its
	 * original value.
	 */
	isDirty: function() {
		var me = this;
		return me.getStore().isDirty();
	},

	/**
	 * Loads an array of data straight into the grid.
	 * @param values The values to load into the grids store.
	 * @return void
	 */
	setValues: function(values) {
		var me = this;
		if (Ext.isDefined(values))
			me.getStore().loadData(values);
	},

	/**
	 * Gets all values for each record in this store and returns an object
	 * containing the current record values as described in the store's model.
	 * @return An array of object hash containing all the values.
	 */
	getValues: function() {
		var me = this;
		var values = me.getStore().getModelData();
		return values;
	},

	/**
	 * Load the grid content.
	 * @return void
	 */
	doLoad: function() {
		var me = this;
		me.getStore().load();
	},

	/**
	 * Reload the grid content.
	 * @return void
	 */
	doReload: function() {
		var me = this;
		// Do not reload the store if it is currently performing a
		// load operation.
		if (me.getStore().isLoading())
			return;
		me.getStore().reload();
	}
});
