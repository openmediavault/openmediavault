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

Ext.ns("OMV.grid");

/**
 * @class OMV.grid.GridPanel
 * @derived Ext.grid.GridPanel
 * Generic grid panel implementation.
 * @config disableLoadMaskOnLoad TRUE to disable the load mask after the first
 * load has been done.
 */
OMV.grid.GridPanel = function(config) {
	var initialConfig = {
		loadMask: true,
		disableLoadMaskOnLoad: false,
		autoScroll: true,
		stripeRows: true,
		columnLines: true,
		layout: "fit",
		border: false,
		stateful: true,
		viewConfig: {
			forceFit: true
		}
	};
	Ext.apply(initialConfig, config);
	OMV.grid.GridPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.grid.GridPanel, Ext.grid.GridPanel, {
	initComponent : function() {
		this.store.on("load", function(store, records, options) {
			if (this.disableLoadMaskOnLoad && Ext.isObject(this.loadMask)) {
				this.loadMask.disable();
			}
		}, this);
		OMV.grid.GridPanel.superclass.initComponent.apply(this, arguments);
	},

	/**
	 * Returns true if any record of this grid has been changed from its
	 * original value.
	 */
	isDirty : function() {
		return this.store.isDirty();
	},

	/**
	 * @method setValues
	 * Load values into the grids store.
	 * @param values The values to load into the grids store.
	 */
	setValues : function(values) {
		if (values != undefined) {
			this.store.loadData(values);
		}
	},

	/**
	 * @method getValues
	 * Returns the records of the grids store as object with key/value pairs.
	 * @return The records of the grids store as key/value pairs.
	 */
	getValues : function() {
		var values = [];
		var records = this.store.getRange();
		// Process all records in the store and apply the data to the
		// array to be returned.
		Ext.each(records, function(record) {
			var data = {};
			Ext.iterate(record.data, function(prop, value) {
				var m = record.fields.map[prop];
				if (m) {
					if (m.mapping) {
						var re = /[\[\.]/;
						if (re.test(m.mapping)) {
							// Split mapping string. The last string is the
							// attribute/property name.
							var sa = m.mapping.split(".");
							var name = sa.pop();
							var d = data;
							// Navigate to the attribute. On the way down
							// create the sub-array's if necessary.
							// Example:
							// m.mapping = xxx.yyy.zzz
							// 1. Create 'xxx' and 'yyy' sub-array
							// 2. Set the value of the attribute 'zzz'
							Ext.iterate(sa, function(a) {
								// Create array if necessary
								if (d[a] == undefined) {
									d[a] = {};
								}
								d = d[a];
							});
							// Set attribute value
							d[name] = value;
						} else {
							data[m.mapping] = value;
						}
					} else {
						data[m.name] = value;
					}
				}
			});
			values.push(data);
		});
		return values;
	}
});
