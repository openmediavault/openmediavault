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
// require("js/omv/grid/Panel.js")

/**
 * @ingroup webgui
 * @class OMV.form.field.Grid
 * This field displayes a grid control. The content of the grid is set via
 * the setValue method and can be retrieved with the getValue method.
 * @derived Ext.form.FieldContainer
 * @param allowBlank Set to FALSE to require at least one item in the grid
 *   to be selected, TRUE to allow no selection. Defaults to TRUE.
 * @param delimiter The string used to delimit the selected values.
 * @param useStringValue TRUE to return the field value always as string.
 *   The delimiter is used to concatenate the values. Defaults to FALSE.
 *   The setValue method splits the value before processing in case it is
 *   a string.
 * @param blankText The error text to display if the allowBlank validation
 *   fails.
 * @param readOnly Set to TRUE to mark the field as readOnly.
 * @param gridClass The grid class name. Defaults to OMV.grid.Panel.
 * @param gridConfig The grid configuration.
 */
Ext.define("OMV.form.field.Grid", {
	extend: "Ext.form.FieldContainer",
	alias: "widget.gridfield",
	requires: [
		"Ext.form.field.Base",
		"OMV.grid.Panel"
	],
	mixins: {
		bindable: "Ext.util.Bindable",
		field: "Ext.form.field.Field"
	},

	allowBlank: true,
	blankText: _("This field is required"),
	delimiter: ",",
	useStringValue: false,
	readOnly: false,
	gridClass: "OMV.grid.Panel",
	gridConfig: {},

	height: 150,
	layout: "anchor",
	border: false,
	fieldBodyCls: Ext.baseCSSPrefix + "form-gridfield-body",

	initComponent: function() {
		var me = this;
		// Create the grid object.
		me.grid = Ext.create(me.gridClass, Ext.apply(
		  me.getGridConfig(), me.gridConfig));
		// Note, in some scenarious the grid has it's own store and the
		// field's store is not defined. In this case simply use the
		// grid's store.
		if(!Ext.isDefined(me.store))
			me.store = me.grid.store;
		me.items = [ me.grid ];
		me.callParent();
		// Bind the store to this instance.
		me.bindStore(me.store, true);
	},

	getStoreListeners: function() {
		var me = this;
		var syncValue = function(store) {
			var records = store.getRange();
			me.mixins.field.setValue.call(me, me.convertData(records));
		};
		return {
			datachanged: syncValue,
			refresh: syncValue,
			add: syncValue,
			bulkremove: syncValue,
			update: syncValue,
			clear: syncValue
		};
	},

	getGridConfig: function() {
		var me = this;
		var config = {
			anchor: "100% 100%",
			border: true,
			disabled: me.readOnly
		};
		// Only apply the store if it's defined. This is because in some
		// scenarious the grid may already has its own store defined.
		if(Ext.isDefined(me.store)) {
			Ext.apply(config, {
				store: me.store
			});
		}
		return config;
	},

	getGrid: function() {
		return this.grid;
	},

	/**
	 * The function that is called whenever the store has been modified.
	 * It is used to convert the data as itis stored in the store into the
	 * structure that is required by the form field.
	 * @param records An array of all records from the store.
	 * @return The value as it should be stored by the form field.
	 */
	convertData: function(records) {
		var value = [];
		Ext.Array.each(records, function(record) {
			value.push(record.getData());
		});
		return value;
	},

	setValue: function(value) {
		var me = this;
		if(me.useStringValue && me.delimiter && Ext.isString(value))
			value = value.split(me.delimiter);
		me.mixins.field.setValue.apply(me, value);
		me.store.loadData(value);
	},

	getModelData: function() {
		var me = this;
		if(me.useStringValue)
			return me.getSubmitData();
		return me.mixins.field.getModelData.apply(me, arguments);
	},

	getSubmitData: function() {
		var me = this;
		var data = null;
		if(!me.disabled && me.submitValue && !me.isFileUpload()) {
			var value = me.getValue();
			data = {};
			data[me.getName()] = Ext.isString(me.delimiter) ?
			  value.join(me.delimiter) : value;
		}
		return data;
	},

	setReadOnly: function(readOnly) {
		var me = this;
		me.readOnly = readOnly;
		me.getGrid().setDisabled(readOnly);
	},

	setDisabled: function(disabled) {
		var me = this;
		me.disabled = disabled;
		me.getGrid().setDisabled(disabled);
	},

	getErrors: function(value) {
		var me = this;
		var errors = [];
		value = Ext.Array.from(value);
		if(!me.allowBlank && (value.length < 1)) {
			errors.push(me.blankText);
		}
		return errors;
	},

	isValid: function() {
		var me = this;
		return me.disabled || me.validateValue(me.getValue());
	}
}, function() {
	this.borrow(Ext.form.field.Base, [
		"setError",
		"validateValue",
		"markInvalid",
		"clearInvalid"
	]);
});
