/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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

Ext.ns("OMV.form");

OMV.form.CheckboxGrid = function(config) {
	var initialConfig = {
		isFormField: true,
		allowBlank: true,
		minSelections: 0,
		maxSelections: Number.MAX_VALUE,
		blankText: Ext.form.TextField.prototype.blankText,
		minSelectionsText: "Minimum {0} item(s) required",
		maxSelectionsText: "Maximum {0} item(s) allowed",
		readOnly: false,
		separator: ",",
		loadMask: true
	};
	Ext.apply(initialConfig, config);
	OMV.form.CheckboxGrid.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires when the selection changes
		 */
		"selectionchange"
	);
};
Ext.extend(OMV.form.CheckboxGrid, Ext.grid.GridPanel, {
	initComponent : function() {
		// Set selection model
		this.sm = new Ext.grid.CheckboxSelectionModel({
			singleSelect: false
		});
		// Append checkbox to column model
		this.colModel.columns.unshift(this.sm);
		OMV.form.CheckboxGrid.superclass.initComponent.call(this);
		// Register event handler
		var sm = this.getSelectionModel();
		sm.on("selectionchange", this.cbSelectionChange, this);
		sm.on("beforerowselect", this.cbBeforeRowSelect, this);
	},

	setValue : function(values) {
		if (Ext.isEmpty(this.store))
			return;
		if (!Ext.isArray(values)) {
			values = values.split(this.separator);
		}
		this.value = values.join(this.separator);
		if (this.hiddenField) {
			this.hiddenField.value = Ext.value(this.value, '');
		}
		// Sometimes the first load of a form loads a value into a control
		// before that control's store has been populated.
		// http://www.extjs.com/forum/showthread.php?p=364333
		if (!Ext.isDefined(this.store.totalLength)) {
			this.store.on("load", this.setValue.createDelegate(
			  this, arguments), null, { single: true });
			if (this.store.lastOptions === null) {
				this.store.load();
			}
			return;
		}
		// Get the selection model
		var sm = this.getSelectionModel();
		// Remember the locked state and unlock to grid temporary to be able
		// to modify the grid selection.
		var locked = sm.isLocked();
		sm.unlock();
		// Deny firing events
		sm.suspendEvents();
		// Clear selections
		sm.clearSelections(true);
		if (!Ext.isEmpty(values)) {
			var records = [];
			for (var i = 0; i < values.length; i++) {
				var index = this.store.findExact(this.valueField, values[i]);
				if (index > -1)
					records.push(this.store.getAt(index));
			}
			sm.selectRecords(records);
			this.validate();
		}
		// Allow firing events
		sm.resumeEvents();
		// Reset selection model lock state
		if (locked === true) {
			sm.lock();
		}
	},

	getValue : function() {
		return Ext.isDefined(this.value) ? this.value : '';
	},

	/**
	 * Creates hidden field to get BasicForm::getValues to work
	 */
	onRender : function() {
		OMV.form.CheckboxGrid.superclass.onRender.apply(this, arguments);
		if (true === this.isFormField) {
			this.hiddenField = this.body.createChild({
				tag: "input",
				type: "hidden",
				name: this.name || this.id,
				value: ""
			}, undefined, true);
		}
		this.setReadOnly(this.readOnly);
	},

	cbBeforeRowSelect : function(model, rowIndex, keepExisting, record) {
		if (this.disabled || (this.readOnly && model.isLocked())) {
			return false;
		}
		// Fire event and deny selection if a listener returns FALSE.
		if (this.fireEvent("beforerowselect", this, rowIndex, keepExisting,
		  record) === false) {
			return false;
		}
	},

	cbSelectionChange : function(model) {
		this.value = "";
		var records = this.getSelectionModel().getSelections();
		if (records.length > 0) {
			var result = [];
			for (var i = 0; i < records.length; i++) {
				result.push(records[i].get(this.valueField));
			}
			this.value = result.join(this.separator);
		}
		if (this.hiddenField) {
			this.hiddenField.value = Ext.value(this.value, '');
		}
		// Fire event to notify listeners.
		this.fireEvent("selectionchange", this, this.value);
	},

	// Form field compatibility methods
	clearInvalid : Ext.emptyFn,

	markInvalid : Ext.emptyFn,

	isDirty : function() {
		if (this.disabled || !this.rendered) {
			return false;
		}
		return String(this.getValue()) !== String(this.originalValue);
	},

	validate : function() {
		return true;
	},

	isValid : function() {
		if (value.length < 1) { // if it has no value
			if (this.allowBlank) {
				this.clearInvalid();
				return true;
			} else {
				this.markInvalid(this.blankText);
				return false;
			}
		}
		if (value.length < this.minSelections) {
			this.markInvalid(String.format(this.minSelectionsText,
			  this.minSelections));
			return false;
		}
		if (value.length > this.maxSelections) {
			this.markInvalid(String.format(this.maxSelectionsText,
			  this.maxSelections));
			return false;
		}
		return true;
	},

    reset : function() {
		this.setValue("");
    },

	setReadOnly : function(readOnly) {
		this.readOnly = readOnly;
		this.hiddenField.readOnly = readOnly;
		if (this.readOnly) {
			this.getSelectionModel().lock();
		} else {
			this.getSelectionModel().unlock();
		}
	},

	disable : function() {
		this.disabled = true;
		this.hiddenField.disabled = true;
	},

	enable : function() {
		this.disabled = false;
		this.hiddenField.disabled = false;
	},

	getName : function() {
		return this.name || this.id || '';
	}
});
Ext.reg('checkboxgrid', OMV.form.CheckboxGrid);
