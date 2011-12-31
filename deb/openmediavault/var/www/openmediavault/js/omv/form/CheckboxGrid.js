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

/**
 * @class OMV.form.CheckboxGrid
 * @derived Ext.grid.GridPanel
 * @param minSelections The number of minimal selections. Defaults to 0.
 * @param maxSelections The number of max. selections. Defaults to
 * Number.MAX_VALUE.
 */
OMV.form.CheckboxGrid = function(config) {
	var initialConfig = {
		cls: "x-checkboxgrid",
		isFormField: true,
		minSelections: 0,
		maxSelections: Number.MAX_VALUE,
		minSelectionsText: "Minimum {0} selected item(s) required",
		maxSelectionsText: "Maximum {0} selected item(s) allowed",
		readOnly: false,
		separator: ",",
		loadMask: true,
		invalidClass: "x-checkboxgrid-invalid",
		invalidText: "The value in this field is invalid",
		msgTarget: "qtip"
	};
	Ext.apply(initialConfig, config);
	OMV.form.CheckboxGrid.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * @event selectionchange
		 * Fires when the selection changes
		 */
		"selectionchange",
		/**
		 * @event invalid
		 * Fires after the field has been marked as invalid.
		 * @param this
		 * @param msg The validation message
		 */
		"invalid",
		/**
		 * @event valid
		 * Fires after the field has been validated with no errors.
		 * @param this
		 */
		"valid"
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

	afterRender : function(){
		OMV.form.CheckboxGrid.superclass.afterRender.call(this);
		this.initValue();
	},

	initValue : function(){
		if (this.value !== undefined){
			this.setValue(this.value);
		} else if (!Ext.isEmpty(this.el.dom.value) &&
		  this.el.dom.value != this.emptyText) {
			this.setValue(this.el.dom.value);
		}
		this.originalValue = this.getValue();
		if (this.hiddenField) {
            this.hiddenField.value = Ext.value(Ext.isDefined(
			  this.hiddenValue) ? this.hiddenValue : this.value, "");
        }
	},

	clearValue : function() {
		if (this.hiddenField) {
			this.hiddenField.value = "";
		}
		this.setRawValue("");
		this.value = "";
	},

	setRawValue : function(v) {
		return this.rendered ? (this.el.dom.value = (Ext.isEmpty(v) ?
		  "" : v)) : "";
	},

	setValue : function(values) {
		if (Ext.isEmpty(this.store))
			return this;
		if (!Ext.isArray(values)) {
			if (Ext.isEmpty(values))
				values = [];
			else
				values = values.split(this.separator);
		}
		this.value = values.join(this.separator);
		if (this.hiddenField) {
			this.hiddenField.value = Ext.value(this.value, "");
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
			return this;
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
		sm.clearSelections();
		if (values.length > 0) {
			var records = [];
			for (var i = 0; i < values.length; i++) {
				var index = this.store.findExact(this.valueField, values[i]);
				if (index > -1)
					records.push(this.store.getAt(index));
			}
			sm.selectRecords(records);
		}
		// Allow firing events
		sm.resumeEvents();
		// Reset selection model lock state
		if (locked === true) {
			sm.lock();
		}
		this.validate();
		return this;
	},

	getValue : function() {
		return Ext.isDefined(this.value) ? this.value : "";
	},

	getRawValue : function(){
		return this.getValue();
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
		var records = model.getSelections();
		if (records.length > 0) {
			var result = [];
			records.each(function(r) {
				result.push(r.get(this.valueField));
			}, this);
			this.value = result.join(this.separator);
		}
		if (this.hiddenField) {
			this.hiddenField.value = Ext.value(this.value, "");
		}
		this.validate();
		// Fire event to notify listeners.
		this.fireEvent("selectionchange", this, this.value);
	},

	setActiveError: function(msg, suppressEvent) {
		this.activeError = msg;
		if (suppressEvent !== true) this.fireEvent("invalid", this, msg);
	},

	unsetActiveError: function(suppressEvent) {
		delete this.activeError;
		if (suppressEvent !== true) this.fireEvent("valid", this);
	},

	getMessageHandler : function() {
		return Ext.form.MessageTargets[this.msgTarget];
	},

	clearInvalid : function() {
		// Don't remove the error icon if we're not rendered or marking is
		// prevented
		if (this.rendered && !this.preventMark) {
			this.el.removeClass(this.invalidClass);
			var mt = this.getMessageHandler();
			if (mt) {
				mt.clear(this);
			} else if (this.msgTarget) {
				this.el.removeClass(this.invalidClass);
				var t = Ext.getDom(this.msgTarget);
				if (t) {
					t.innerHTML = "";
					t.style.display = "none";
				}
			}
		}
		this.unsetActiveError();
	},

	markInvalid : function(msg) {
		// Don't set the error icon if we're not rendered or marking is
		// prevented
		if (this.rendered && !this.preventMark) {
			msg = msg || this.invalidText;
			var mt = this.getMessageHandler();
			if (mt) {
				mt.mark(this, msg);
			} else if (this.msgTarget) {
				this.el.addClass(this.invalidClass);
				var t = Ext.getDom(this.msgTarget);
				if (t) {
					t.innerHTML = msg;
					t.style.display = this.msgDisplay;
				}
			}
		}
		this.setActiveError(msg);
	},

	isDirty : function() {
		if (this.disabled || !this.rendered) {
			return false;
		}
		return String(this.getValue()) !== String(this.originalValue);
	},

	isValid : function(preventMark) {
		if (this.disabled){
			return true;
		}
		var restore = this.preventMark;
		this.preventMark = preventMark === true;
		var v = this.validateValue(this.processValue(this.getRawValue()));
		this.preventMark = restore;
		return v;
	},

	validate : function() {
		if (this.disabled || this.validateValue(this.processValue(
		  this.getRawValue()))){
			this.clearInvalid();
			return true;
		}
		return false;
	},

	processValue : function(value) {
		return value;
	},

	validateValue : function(value) {
		// Currently, we only show 1 error at a time for a field, so just use
		// the first one
		var error = this.getErrors(value)[0];
		if (error == undefined) {
			return true;
		} else {
			this.markInvalid(error);
			return false;
		}
	},

	getErrors : function(value) {
		var errors = [];
		if (!Ext.isArray(value)) {
			if (Ext.isEmpty(value))
				value = [];
			else
				value = value.split(this.separator);
		}
		if (value.length < this.minSelections) {
			errors.push(String.format(this.minSelectionsText,
			  this.minSelections));
		}
		if (value.length > this.maxSelections) {
			errors.push(String.format(this.maxSelectionsText,
			  this.maxSelections));
		}
		return errors;
	},

	reset : function() {
		this.setValue(this.originalValue);
		this.clearInvalid();
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
		return this.name || this.id || "";
	},

	onDestroy : function(){
		Ext.destroyMembers(this, "hiddenField");
		OMV.form.CheckboxGrid.superclass.onDestroy.call(this);
	}
});
Ext.reg('checkboxgrid', OMV.form.CheckboxGrid);
