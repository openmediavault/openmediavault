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

Ext.ns("OMV");

/**
 * @class OMV.UploadDialog
 * @config title The dialog title.
 * @config service The name of the RPC service.
 * @config method The name of the RPC method.
 * @config params Additional RPC method parameters.
 * @config waitMsg The displayed waiting message.
 */
OMV.UploadDialog = function(config) {
	var initialConfig = {
		url: "upload.php",
		title: _("Upload file"),
		waitMsg: _("Uploading file ..."),
		width: 450,
		autoHeight: true,
		layout: "fit",
		modal: true,
		border: false,
		buttonAlign: "center",
		buttons: [{
			text: _("OK"),
			handler: this.cbOkBtnHdl.createDelegate(this),
			scope: this
		},{
			text: _("Cancel"),
			handler: this.cbCancelBtnHdl.createDelegate(this),
			scope: this
		}]
	};
	Ext.apply(initialConfig, config);
	OMV.UploadDialog.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires after the installation has been finished successful.
		 */
		"success"
	);
};
Ext.extend(OMV.UploadDialog, OMV.Window, {
	initComponent : function() {
		this.form = new Ext.form.FormPanel({
			frame: true,
			border: false,
			layout: "form",
			defaults: {
				anchor: "100%",
				labelSeparator: ""
			},
			autoHeight: true,
			fileUpload: true,
			items: [{
				xtype: "hidden",
				name: "service",
				value: this.service
			},{
				xtype: "hidden",
				name: "method",
				value: this.method
			},{
				xtype: "hidden",
				name: "params",
				value: this.params
			},{
				xtype: "textfield",
				name: "file",
				fieldLabel: _("File"),
				allowBlank: false,
				inputType: "file"
			}]
		});
		this.items = this.form;
		OMV.UploadDialog.superclass.initComponent.apply(this, arguments);
	},

	/**
	 * @method cbOkBtnHdl
	 * Method that is called when the 'OK' button is pressed.
	 */
	cbOkBtnHdl : function() {
		var basicForm = this.form.getForm();
		if (!basicForm.isValid()) {
			return;
		}
		basicForm.submit({
			url: this.url,
			method: "POST",
			waitMsg: this.waitMsg,
			success: function(form, action) {
				this.fireEvent("success", this, action.result.responseText);
				this.close();
			},
			failure: function(form, action) {
				var msg = action.response.responseText;
				try {
					// Try to decode JSON data.
					msg = Ext.util.JSON.decode(action.response.responseText);
				} catch(e) {
					// Decoding JSON has been failed, assume response contains
					// plain text.
				}
				OMV.MessageBox.error(null, msg);
			},
			scope: this
		});
	},

	/**
	 * @method cbCancelBtnHdl
	 * Method that is called when the 'Cancel' button is pressed.
	 */
	cbCancelBtnHdl : function() {
		this.close();
	}
});
