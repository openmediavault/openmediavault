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
// require("js/omv/window/Window.js")
// require("js/omv/form/Panel.js")

/**
 * @ingroup webgui
 * @class OMV.window.Upload
 * @derived OMV.window.Window
 * @param service The name of the RPC service. Required.
 * @param method The name of the RPC method. Required.
 * @param params Additional RPC method parameters. Required.
 * @param title The dialog title.
 * @param waitMsg The displayed waiting message.
 */
Ext.define("OMV.window.Upload", {
	extend: "OMV.window.Window",
	requires: [
		"OMV.form.Panel",
	],

	url: "upload.php",
	title: _("Upload file"),
	waitMsg: _("Uploading file ..."),
	width: 450,
	layout: "fit",
	modal: true,
	buttonAlign: "center",

	constructor: function(config) {
		var me = this;
		me.callParent(arguments);
		me.addEvents(
			/**
			 * Fires after the installation has been finished successful.
			 * @param this The window object.
			 * @param response The response from the form submit action.
			 */
			"success"
		);
	},

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			buttons: [{
				text: _("OK"),
				handler: me.onOkButton,
				scope: me
			},{
				text: _("Cancel"),
				handler: me.onCancelButton,
				scope: me
			}],
			items: [ me.fp = Ext.create("OMV.form.Panel", {
				bodyPadding: "5 5 0",
				fileUpload: true,
				items: [{
					xtype: "filefield",
					name: "file",
					fieldLabel: _("File"),
					allowBlank: false
				}]
			}) ]
		});
		me.callParent(arguments);
	},

	/**
	 * Method that is called when the 'OK' button is pressed.
	 */
	onOkButton: function() {
		var me = this;
		var basicForm = me.fp.getForm();
		if(!basicForm.isValid())
			return;
		basicForm.submit({
			url: me.url,
			method: "POST",
			params: {
				service: me.service,
				method: me.method,
				params: me.params
			},
			waitMsg: me.waitMsg,
			scope: me,
			success: function(form, action) {
				me.close();
				me.fireEvent("success", me, action.result.responseText);
			},
			failure: function(form, action) {
				var msg = action.response.responseText;
				me.close();
				try {
					// Try to decode JSON data.
					msg = Ext.util.JSON.decode(action.response.responseText);
				} catch(e) {
					// Decoding JSON has been failed, assume response contains
					// plain text.
				}
				OMV.MessageBox.error(null, msg);
			}
		});
	},

	/**
	 * Method that is called when the 'Cancel' button is pressed.
	 */
	onCancelButton: function() {
		this.close();
	}
});
