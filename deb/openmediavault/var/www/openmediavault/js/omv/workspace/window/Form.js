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
// require("js/omv/form/Panel.js")

/**
 * @ingroup webgui
 * A workspace window displaying a form panel.
 * @class OMV.workspace.window.Form
 * @derived OMV.workspace.window.Container
 */
Ext.define("OMV.workspace.window.Form", {
	extend: "OMV.workspace.window.Container",
	requires: [
		"OMV.form.Panel"
	],

	formClassName: "OMV.form.Panel",
	formConfig: {},
	formItems: [],

	getWindowItems: function() {
		var me = this;
		me.fp = me.initForm();
		return me.fp;
	},

	/**
	 * Initialize the form panel displayed in this window.
	 * @return The form panel object.
	 */
	initForm: function() {
		var me = this;
		return Ext.create(me.formClassName, Ext.apply({
			bodyPadding: "5 5 0",
			autoScroll: true,
			trackResetOnLoad: true,
			fieldDefaults: {
				readOnly: me.readOnly,
				anchor: "100%",
				labelSeparator: ""
			},
			items: me.getFormItems()
		}, me.getFormConfig()));
	},

	/**
	 * Returns additional form configuration options.
	 * @return The form panel configuration object.
	 */
	getFormConfig: function() {
		return this.formConfig;
	},

	/**
	 * Returns the items displayed in the property window form.
	 * This function must be overwritten by every derived class.
	 * @return An array of items displayed in the form panel.
	 */
	getFormItems: function() {
		return this.formItems;
	},

	/**
	 * Returns the form panel.
	 * @return The form panel object.
	 */
	getForm: function() {
		return this.fp;
	},

	/**
	 * Validate the form values.
	 * @return Returns TRUE if client-side validation on the form
	 *   is successful.
	 */
	isValid: function() {
		return this.getForm().isValid();
	},

	/**
	 * Clears all invalid field messages in this form.
	 */
	clearInvalid: function() {
		return this.getForm().clearInvalid();
	},

	/**
	 * Mark fields in this form invalid in bulk.
	 * @param errors Either an array in the form
	 *   [{id:'fieldId', msg:'The message'},...] or an object hash of
	 *   {id: msg, id2: msg2}
	 * @return The basic form panel.
	 */
	markInvalid: function(errors) {
		return this.getForm().markInvalid(errors);
	},

	/**
	 * Checks if any fields in this form have changed from their original
	 * values. If the values have been loaded into the form then these are
	 * the original ones.
	 * @return Returns TRUE if any fields in this form have changed from
	 *   their original values.
	 */
	isDirty: function() {
		return this.getForm().isDirty();
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form panel.
	 */
	setValues: function(values) {
		var me = this;
		var result = me.getForm().setValues(values);
		me.getForm().clearInvalid();
		return result;
	},

	/**
	 * Returns the fields in this form as an object with key/value pairs.
	 * @return An array of key/value pairs.
	 */
	getValues: function() {
		var values = this.getForm().getValues();
		return values;
	},

	/**
	 * Find a Ext.form.Field in this form panel.
	 * @param id The value to search for(specify either a id, dataIndex,
	 *   name or hiddenName).
	 * @returns The searched Ext.form.Field
	 */
	findField: function(id) {
		return this.getForm().findField(id);
	},

	/**
	 * Method that is called when the 'Reset' button is pressed. Reject
	 * all changes done in the form.
	 * @param this The window itself.
	 */
	onResetButton: function() {
		this.getForm().reset();
	}
});
