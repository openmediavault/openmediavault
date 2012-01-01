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
// require("js/omv/Window.js")
// require("js/omv/MessageBox.js")
// require("js/omv/data/Connection.js")

Ext.ns("OMV");

/**
 * @class OMV.FormPanelDialog
 * @derived OMV.Window
 * @param rpcService The RPC service name.
 * @param rpcGetMethod The RPC method to get the data.
 * @param rpcGetParams The RPC method parameters.
 * @param rpcSetMethod The RPC method to commit the data.
 * @param success The function that should be called in case of a successful
 * data commit.
 * @param scope The scope to be used when callig the success function.
 * @param hideOk True to hide the 'OK' button. Defaults to false.
 * @param hideCancel True to hide the 'Cancel' button. Defaults to false.
 * @param hideClose True to hide the 'Close' button. Defaults to true.
 * @param hideReset True to hide the 'Reset' button. Defaults to true.
 * @param mode The mode how to retrieve the data displayed in the property
 * dialog. This can be 'local' or 'remote' which means the data is requested
 * via RPC. Defaults to 'remote'.
 * @param readOnly True if the property values are read-only. The 'OK' and
 * 'Reset' buttons will be disabled in this case. Defaults to false.
 */
OMV.FormPanelDialog = function(config) {
	var initialConfig = {
		width: 400,
		height: 200,
		layout: "fit",
		modal: true,
		border: false,
		mode: "remote",
		hideOk: false, // Hide the 'Ok' button
		hideReset: false, // Hide the 'Reset' button
		hideCancel: false, // Hide the 'Cancel' button
		hideClose: true, // Hide the 'Close' button
		buttonAlign: "center",
		readOnly: false
	};
	Ext.apply(initialConfig, config);
	OMV.FormPanelDialog.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires after the form content has been loaded successful.
		 */
		"load",
		/**
		 * Fires after the submission has been finished successful.
		 */
		"submit"
	);
};
Ext.extend(OMV.FormPanelDialog, OMV.Window, {
	initComponent : function() {
		// Initialize the embedded form
		this.form = this.initForm();
		Ext.apply(this, {
			buttons: [{
				id: this.getId() + "-ok",
				text: "OK",
				hidden: this.hideOk,
				disabled: this.readOnly,
				handler: this.cbOkBtnHdl,
				scope: this
			},{
				id: this.getId() + "-reset",
				text: "Reset",
				hidden: this.hideReset,
				disabled: this.readOnly,
				handler: this.cbResetBtnHdl,
				scope: this
			},{
				id: this.getId() + "-cancel",
				text: "Cancel",
				hidden: this.hideCancel,
				handler: this.cbCancelBtnHdl,
				scope: this
			},{
				id: this.getId() + "-close",
				text: "Close",
				hidden: this.hideClose,
				handler: this.cbCloseBtnHdl,
				scope: this
			}],
			items: this.form
		});
		OMV.FormPanelDialog.superclass.initComponent.apply(this, arguments);
		// Register event handler
		if ((this.mode === "remote") && Ext.isDefined(this.rpcGetMethod)) {
			this.on("render", this.doLoad, this, { delay: 10 });
		}
	},

	/**
	 * Initialize the property window form panel.
	 */
	initForm : function() {
		var config = Ext.applyEx({
			frame: true,
			border: false,
			trackResetOnLoad: true,
			defaults: {
				anchor: "100%",
				labelSeparator: "",
				readOnly: this.readOnly
			}
		}, this.getFormConfig());
		config.items = this.getFormItems();
		return new Ext.form.FormPanel(config);
	},

	/**
	 * Returns additional form configuration options.
	 */
	getFormConfig : function() {
		return {};
	},

	/**
	 * Returns the items displayed in the property window form.
	 * This function must be overwritten by every derived class.
	 */
	getFormItems : function() {
		return [];
	},

	/**
	 * Validate the form values.
	 * @return Returns true if client-side validation on the form is successful.
	 */
	isValid : function() {
		var basicForm = this.form.getForm();
		return basicForm.isValid();
	},

	/**
	 * Checks if any fields in this form have changed from their original
	 * values. If the values have been loaded into the form then these are
	 * the original ones.
	 * @return Returns true if any fields in this form have changed from
	 * their original values.
	 */
	isDirty : function() {
		var basicForm = this.form.getForm();
		return basicForm.isDirty();
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form object.
	 */
	setValues : function(values) {
		var basicForm = this.form.getForm();
		basicForm.setValues(values);
		basicForm.clearInvalid();
		return basicForm;
	},

	/**
	 * Returns the fields in this form as an object with key/value pairs.
	 */
	getValues : function() {
		var basicForm = this.form.getForm();
		var values = basicForm.getValuesEx();
		return values;
	},

	/**
	 * Find a Ext.form.Field in this form.
	 * @param id The value to search for (specify either a id, dataIndex,
	 * name or hiddenName).
	 * @returns The searched Ext.form.Field
	 */
	findFormField : function(id) {
		var basicForm = this.form.getForm();
		return basicForm.findField(id);
	},

	doLoad : function() {
		// Display waiting dialog
		OMV.MessageBox.wait(null, "Loading ...");
		// Execute RPC
		OMV.Ajax.request(this.cbLoadHdl, this, this.rpcService,
		  this.rpcGetMethod, this.rpcGetParams);
	},

	cbLoadHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			this.setValues(response);
			this.fireEvent("load", this, response);
		} else {
			OMV.MessageBox.error(null, error);
		}
	},

	doSubmit : function() {
		// Get the form values
		var values = this.getValues();
		if (this.mode === "remote") {
			// Display waiting dialog
			OMV.MessageBox.wait(null, "Saving ...");
			// Execute RPC
			OMV.Ajax.request(this.cbSubmitHdl, this, this.rpcService,
			  this.rpcSetMethod, [ values ]);
		} else {
			this.fireEvent("submit", this, values);
			this.close();
		}
	},

	cbSubmitHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			var values = this.getValues();
			this.fireEvent("submit", this, values);
			this.close();
		} else {
			OMV.MessageBox.error(null, error);
		}
	},

	/**
	 * @method cbOkBtnHdl
	 * Method that is called when the 'OK' button is pressed.
	 */
	cbOkBtnHdl : function() {
		// Quit immediatelly if the property values have not been modified
		if (!this.isDirty()) {
			this.close();
			return;
		}
		// Validate values
		if (!this.isValid()) {
			// Do not close the property dialog. The invalid fields are marked
			// automatically.
		} else {
			this.doSubmit();
		}
	},

	/**
	 * @method cbCancelBtnHdl
	 * Method that is called when the 'Cancel' button is pressed.
	 */
	cbCancelBtnHdl : function() {
		this.close();
	},

	/**
	 * @method cbCloseBtnHdl
	 * Method that is called when the 'Close' button is pressed.
	 */
	cbCloseBtnHdl : function() {
		this.close();
	},

	/**
	 * @method cbResetBtnHdl
	 * Method that is called when the 'Reset' button is pressed.
	 */
	cbResetBtnHdl : function() {
		var basicForm = this.form.getForm();
		basicForm.reset();
	}
});
