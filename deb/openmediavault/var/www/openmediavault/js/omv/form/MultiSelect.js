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

OMV.form.MultiSelect = function(config) {
	var initialConfig = {
		appendOnly: false,
		width: 100,
		height: 100,
		displayField: 0,
		valueField: 1,
		allowBlank: true,
		minSelections: 0,
		maxSelections: Number.MAX_VALUE,
		blankText: Ext.form.TextField.prototype.blankText,
		minSelectionsText: "Minimum {0} item(s) required",
		maxSelectionsText: "Maximum {0} item(s) allowed",
		delimiter: ",",
		defaultAutoCreate: {
			tag: "div"
		}
	};
	Ext.apply(initialConfig, config);
	OMV.form.MultiSelect.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.form.MultiSelect, Ext.form.Field, {
	initComponent: function(){
		OMV.form.MultiSelect.superclass.initComponent.call(this);
		if (Ext.isArray(this.store)){
			if (Ext.isArray(this.store[0])){
				this.store = new Ext.data.ArrayStore({
					fields: ['value','text'],
					data: this.store
				});
				this.valueField = 'value';
			} else {
				this.store = new Ext.data.ArrayStore({
					fields: ['text'],
					data: this.store,
					expandData: true
				});
				this.valueField = 'text';
			}
			this.displayField = 'text';
		} else {
			this.store = Ext.StoreMgr.lookup(this.store);
		}
		this.addEvents({
			'dblclick': true,
			'click': true,
			'change': true,
			'drop': true
		});
	},

	onRender: function(ct, position){
		OMV.form.MultiSelect.superclass.onRender.call(this, ct, position);
		var fs = this.fs = new Ext.form.FieldSet({
			cls: "x-form-fieldset-multiselect",
			bodyCssClass: "x-form-fieldset-multiselect-body",
			renderTo: this.el,
			title: this.legend,
			height: this.height,
			tbar: this.tbar,
			bodyStyle: 'overflow: auto;',
			anchor: "100%"
		});

		this.view = new Ext.ListView({
			cls: "x-form-fieldset-listview",
			overClass: "x-list-multiselect-over",
			selectedClass: "x-list-multiselect-selected",
			multiSelect: true,
			store: this.store,
			columns: [{ header: 'Value', width: 1, dataIndex: this.displayField }],
			hideHeaders: true
		});

		fs.add(this.view);

		this.view.on('click', this.onViewClick, this);
		this.view.on('beforeclick', this.onViewBeforeClick, this);
		this.view.on('dblclick', this.onViewDblClick, this);

		this.hiddenName = this.name || Ext.id();
		var hiddenTag = { tag: "input", type: "hidden", value: "", name: this.hiddenName };
		this.hiddenField = this.el.createChild(hiddenTag);
		this.hiddenField.dom.disabled = this.hiddenName != this.name;
		fs.doLayout();
	},

	onViewClick: function(vw, index, node, e) {
		this.fireEvent('change', this, this.getValue(), this.hiddenField.dom.value);
		this.hiddenField.dom.value = this.getValue();
		this.fireEvent('click', this, e);
		this.validate();
	},

	onViewBeforeClick: function(vw, index, node, e) {
		if (this.disabled) {return false;}
	},

	onViewDblClick : function(vw, index, node, e) {
		return this.fireEvent('dblclick', vw, index, node, e);
	},

	getValue: function(valueField){
		var returnArray = [];
		var selectionsArray = this.view.getSelectedIndexes();
		if (selectionsArray.length == 0) {return '';}
		for (var i=0; i<selectionsArray.length; i++) {
			returnArray.push(this.store.getAt(selectionsArray[i]).get((valueField != null) ? valueField : this.valueField));
		}
		return returnArray.join(this.delimiter);
	},

	setValue: function(values) {
		var index;
		var selections = [];
		this.view.clearSelections();
		this.hiddenField.dom.value = '';

		if (!values || (values == '')) { return; }

		if (!Ext.isArray(values)) { values = values.split(this.delimiter); }
		for (var i=0; i<values.length; i++) {
			index = this.view.store.indexOf(this.view.store.query(this.valueField,
				new RegExp('^' + values[i] + '$', "i")).itemAt(0));
			selections.push(index);
		}
		this.view.select(selections);
		this.hiddenField.dom.value = this.getValue();
		this.validate();
	},

	reset : function() {
		this.setValue('');
	},

	getRawValue: function(valueField) {
		var tmp = this.getValue(valueField);
		if (tmp.length) {
			tmp = tmp.split(this.delimiter);
		}
		else {
			tmp = [];
		}
		return tmp;
	},

	setRawValue: function(values){
		setValue(values);
	},

	validateValue : function(value){
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

	disable: function(){
		this.disabled = true;
		this.hiddenField.dom.disabled = true;
		this.fs.disable();
	},

	enable: function(){
		this.disabled = false;
		this.hiddenField.dom.disabled = false;
		this.fs.enable();
	},

	destroy: function(){
		Ext.destroy(this.fs);
		OMV.form.MultiSelect.superclass.destroy.call(this);
	}
});
Ext.reg('multiselect', OMV.form.MultiSelect);
