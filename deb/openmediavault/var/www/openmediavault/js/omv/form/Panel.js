/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
 * @class OMV.form.Panel
 * @derived Ext.form.Panel
 */
Ext.define("OMV.form.Panel", {
	extend: "Ext.form.Panel",

	autoScroll: true,
	frame: false,
	border: false,
	trackResetOnLoad: true,
	fieldDefaults: {
		anchor: "100%",
		labelSeparator: ""
	},

	/**
	 * Validate the form values.
	 * @return Returns true if client-side validation on the form is successful.
	 */
	isValid: function() {
		var me = this;
		var basicForm = me.getForm();
		return basicForm.isValid();
	},

	/**
	 * Mark fields in this form invalid in bulk.
	 * @param errors Either an array in the form
	 * [{id:'fieldId', msg:'The message'},...] or an object hash of
	 * {id: msg, id2: msg2}
	 * @return {BasicForm} this
	 */
	markInvalid: function(errors) {
		if(arguments.length === 2) {
			errors = [{
				id: arguments[0],
				msg: arguments[1]
			}];
		}
		var basicForm = this.getForm();
		return basicForm.markInvalid(errors);
	},

	/**
	 * Clears all invalid field messages in this form.
	 * @return {BasicForm} this
	 */
	clearInvalid: function() {
		var me = this;
		var basicForm = me.getForm();
		return basicForm.clearInvalid();
	},

	/**
	 * Checks if any fields in this form have changed from their original
	 * values. If the values have been loaded into the form then these are
	 * the original ones.
	 * @return Returns true if any fields in this form have changed from
	 * their original values.
	 */
	isDirty: function() {
		var me = this;
		var basicForm = me.getForm();
		return basicForm.isDirty();
	},

	/**
	 * Reset the form values.
	 */
	reset: function() {
		var me = this;
		var basicForm = me.getForm();
		basicForm.reset();
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form object.
	 */
	setValues: function(values) {
		var me = this;
		var basicForm = me.getForm();
		basicForm.setValues(values);
		return basicForm;
	},

	/**
	 * Retrieves the fields in the form as a set of key/value pairs.
	 * Note, this method collects type-specific data values.
	 */
	getValues: function() {
		var me = this;
		var basicForm = me.getForm();
		// Get all field values.
		var values = basicForm.getFieldValues();
		// Purge values that have 'submitValue' set to FALSE.
		basicForm.getFields().each(function(field) {
			if(!field.submitValue) {
				delete values[field.getName()];
			}
		});
		return values;
	},

	/**
	 * Find a specific Ext.form.field.Field in this form by id or name.
	 * @return The first matching field, or null if none was found.
	 */
	findField: function(id) {
		var me = this;
		var basicForm = me.getForm();
		return basicForm.findField(id);
	}
});
