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
// require("js/omv/form/field/Grid.js")

/**
 * @ingroup webgui
 * @class OMV.form.field.CheckboxGrid
 * @derived OMV.form.field.Grid
 * This field displayes a grid with a checkbox column. The checked records
 * will be retrieved via getValue and can be set via setValue.
 * @param valueField The underlying data value name to bind to this
 *   field. Required.
 * @param minSelections The number of minimal selections. Defaults to 0.
 * @param maxSelections The number of max. selections. Defaults to
 *   Number.MAX_VALUE.
 * @param minSelectionsText The error text to use when the min. selection
 *   count is invalid.
 * @param maxSelectionsText The error text to use when the max. selection
 *   count is invalid.
 */
Ext.define("OMV.form.field.CheckboxGrid", {
	extend: "OMV.form.field.Grid",
	alias: "widget.checkboxgridfield",
	requires: [
		"Ext.selection.CheckboxModel"
	],

	minSelections: 0,
	maxSelections: Number.MAX_VALUE,
	minSelectionsText: _("Minimum {0} selected item(s) required"),
	maxSelectionsText: _("Maximum {0} selected item(s) allowed"),

	constructor: function(config) {
		var me = this;
		me.callParent(arguments);
		me.addEvents(
			/**
			 * @event selectionchange
			 * Fires when the selection changes
			 * @param this OMV.form.field.CheckboxGrid
			 * @param model The grid selection model.
			 * @param selected The selected records.
			 * @param value The new field value.
			 */
			"selectionchange"
		);
	},

	initComponent: function() {
		var me = this;
		me.callParent();
		me.getGrid().on({
			scope: me,
			selectionchange: function(model, selected, eOpts) {
				var value = [];
				Ext.Array.each(selected, function(record) {
					value.push(record.get(this.valueField));
				}, this);
				this.mixins.field.setValue.call(this, value);
				this.fireEvent("selectionchange", this, model, selected,
				  value, eOpts);
			}
		});
	},

	getGridConfig: function() {
		var me = this;
		var config = me.callParent(arguments);
		Ext.apply(config, {
			selType: "checkboxmodel",
			selModel: {
				showHeaderCheckbox: true
			}
		});
		return config;
	},

	getStoreListeners: function() {
		var me = this;
		return {
			load: function(store, records, successful, eOpts) {
				this.syncSelections();
			}
		};
	},

	/**
	 * Helper method to select the given value in the grid.
	 * @private
	 */
	syncSelections: function() {
		var me = this;
		var selModel = me.getGrid().getSelectionModel();
		// Get the records to be selected.
		var records = [];
		Ext.Array.each(me.value, function(value) {
			var record = me.store.findRecord(me.valueField, value, 0,
			  false, true, true);
			if(null !== record)
				records.push(record);
		});
		if(!records.length)
			return;
		me.store.suspendEvents();
		selModel.select(records);
		me.store.resumeEvents();
	},

	setValue: function(value) {
		var me = this;
		if(me.useStringValue && me.delimiter && Ext.isString(value))
			value = value.split(me.delimiter);
		me.mixins.field.setValue.call(me, value);
		if(me.rendered) {
			me.syncSelections();
		} else {
			me.on({
				afterrender: Ext.Function.bind(me.syncSelections, me),
				single: true
			});
		}
	},

	getErrors: function(value) {
		var me = this;
		var errors = me.callParent(arguments);
		value = Ext.Array.from(value);
		if(value.length < me.minSelections) {
			errors.push(Ext.String.format(me.minSelectionsText,
			  me.minSelections));
		}
		if(value.length > me.maxSelections) {
			errors.push(Ext.String.format(me.maxSelectionsText,
			  me.maxSelections));
		}
		return errors;
	}
});
