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
// require("js/omv/MessageBox.js")
// require("js/omv/data/Connection.js")

Ext.ns("OMV.form");

/**
 * @class OMV.form.FormPanel
 * @derived Ext.form.FormPanel
 * Default form panel. This class includes various helper functions, e.g.
 * to load or submit form values via RPC.
 * @param rpcService The RPC service name.
 * @param rpcGetMethod The RPC method to get request the data.
 * @param rpcGetArgs Additional RPC arguments.
 * @param rpcSetMethod The RPC method to commit the data.
 */
OMV.form.FormPanel = function(config) {
	var initialConfig = {
		frame: true,
		border: false,
		layout: "form",
		defaults: {
			labelSeparator: ""
		}
	};
	Ext.applyEx(initialConfig, config);
	OMV.form.FormPanel.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires after the form content has been loaded successful.
		 */
		"load",
		/**
		 * Fires before the form is submitted.
		 */
		"beforesubmit",
		/**
		 * Fires after the form has been submitted successful.
		 */
		"submit"
	);
};
Ext.extend(OMV.form.FormPanel, Ext.form.FormPanel, {
	doLoad : function() {
		// Display waiting dialog
		OMV.MessageBox.wait(null, "Loading ...");
		// Execute RPC
		OMV.Ajax.request(this.cbLoadHdl, this, this.rpcService,
		  this.rpcGetMethod || "get", this.rpcGetArgs || null);
	},

	/**
	 * Handler that is called by the RPC initiated by 'doLoad'.
	 */
	cbLoadHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			var basicForm = this.getForm();
			this.setValues(response);
			basicForm.clearInvalid();
			this.fireEvent("load", this, response);
		} else {
			OMV.MessageBox.error(null, error);
		}
	},

	doReload : function() {
		this.doLoad();
	},

	doSubmit : function() {
		// Validate values
		if (!this.isValid()) {
			var basicForm = this.getForm();
			basicForm.markInvalid();
		} else {
			var values = this.getValues();
			if (this.fireEvent("beforesubmit", this, values) !== false) {
				// Display waiting dialog
				OMV.MessageBox.wait(null, "Saving ...");
				// Execute RPC
				OMV.Ajax.request(this.cbSubmitHdl, this, this.rpcService,
				  this.rpcSetMethod || "set", [ values ]);
			}
		}
	},

	/**
	 * Handler that is called by the RPC initiated by 'doSubmit'.
	 */
	cbSubmitHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			this.fireEvent("submit", this);
			OMV.MessageBox.success(null, "The changes have been applied " +
			  "successfully.");
		} else {
			OMV.MessageBox.error(null, error);
		}
	},

	/**
	 * Validate the form values.
	 * @return Returns true if client-side validation on the form is successful.
	 */
	isValid : function() {
		var basicForm = this.getForm();
		return basicForm.isValid();
	},

	/**
	 * Mark fields in this form invalid in bulk.
	 * @param errors Either an array in the form
	 * [{id:'fieldId', msg:'The message'},...] or an object hash of
	 * {id: msg, id2: msg2}
	 * @return {BasicForm} this
	 */
	markInvalid : function(errors) {
		var basicForm = this.getForm();
		return basicForm.markInvalid(errors);
	},

	/**
	 * Checks if any fields in this form have changed from their original
	 * values. If the values have been loaded into the form then these are
	 * the original ones.
	 * @return Returns true if any fields in this form have changed from
	 * their original values.
	 */
	isDirty : function() {
		var basicForm = this.getForm();
		return basicForm.isDirty();
	},

	/**
	 * Reset the form values.
	 */
	reset : function() {
		var basicForm = this.getForm();
		basicForm.reset();
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form object.
	 */
	setValues : function(values) {
		var basicForm = this.getForm();
		basicForm.setValues(values);
		return basicForm;
	},

	/**
	 * Returns the fields in this form as an object with key/value pairs.
	 */
	getValues : function() {
		var basicForm = this.getForm();
		var values = basicForm.getValuesEx();
		return values;
	},

	/**
	 * @method setButtonDisabled
	 * Convenience function for setting the given button disabled/enabled.
	 * @param name The name of the button which can be 'start', 'stop'
	 * or 'close'.
	 * @param disabled TRUE to disable the button, FALSE to enable.
	 * @return The button component, otherwise FALSE.
	 */
	setButtonDisabled : function(name, disabled) {
		var btnCtrl = Ext.getCmp(this.getId() + "-" + name);
		if (!Ext.isDefined(btnCtrl))
			return false;
		return btnCtrl.setDisabled(disabled);
	}
});
