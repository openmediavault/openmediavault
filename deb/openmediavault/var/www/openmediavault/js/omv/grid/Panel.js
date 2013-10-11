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
		autoScroll: true,
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
		me.store.on("load", function(store, records, options) {
			if(me.disableLoadMaskOnLoad) {
				if(Ext.isObject(me.view)) {
					if(Ext.isObject(me.view.loadMask)) {
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
		// Reload grid content when panel is activated?
		if(me.reloadOnActivate) {
			me.on("activate", function() {
				this.doReload();
			}, me);
		}
		// Auto-reload the grid content?
		if(me.autoReload) {
			var fnActivateTask = function() {
				if(Ext.isEmpty(me.reloadTask)) {
					me.reloadTask = Ext.util.TaskManager.start({
						run: me.doReload,
						scope: me,
						interval: me.reloadInterval,
						fireOnStart: true
					});
				}
			};
			var fnDeactivateTask = function() {
				if(!Ext.isEmpty(me.reloadTask)) {
					Ext.util.TaskManager.stop(me.reloadTask);
					delete me.reloadTask;
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
	 * Returns an array of the currently selected records.
	 * @return The selected records.
	 */
	getSelection: function() {
		var selModel = this.getSelectionModel();
		var records = selModel.getSelection();
		return records;
	},

	/**
	 * Get the selected record. If more rows are selected the first
	 * one is returned.
	 * @return The selected record.
	 */
	getSelected: function() {
		var records = this.getSelection();
		return records[0];
	},

	/**
	 * Returns true if any record of this grid has been changed from its
	 * original value.
	 */
	isDirty: function() {
		return this.store.isDirty();
	},

	/**
	 * Loads an array of data straight into the grid.
	 * @param values The values to load into the grids store.
	 * @return None.
	 */
	setValues: function(values) {
		var me = this;
		var store = me.getStore();
		if(Ext.isDefined(values))
			store.loadData(values);
	},

	/**
	 * Get the records from the grids store as key/value pairs.
	 * @return The records from the grids store as key/value pairs.
	 */
	getValues: function() {
		var me = this;
		var values = [];
		var store = me.getStore();
		var records = store.getRange();
		Ext.Array.each(records, function(record) {
			values.push(record.getData());
		});
		return values;
	},

	/**
	 * Load the grid content.
	 */
	doLoad: function() {
		var me = this;
		me.store.load();
	},

	/**
	 * Reload the grid content.
	 */
	doReload: function() {
		var me = this;
		me.store.reload();
	}
});
