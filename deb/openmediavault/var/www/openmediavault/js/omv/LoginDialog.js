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
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/LanguageComboBox.js")

Ext.ns("OMV");

OMV.LoginDialog = function(config) {
	var initialConfig = {
		title: _("Login"),
		layout: "fit",
		width: 320,
		closable: false,
		resizable: false,
		buttonAlign: "center"
	};
	Ext.apply(initialConfig, config);
	OMV.LoginDialog.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires when the 'Login' button has been pressed and the RPC
		 * has been executed.
		 */
		"login"
	);
};
Ext.extend(OMV.LoginDialog, OMV.Window, {
	initComponent : function() {
		this.form = new Ext.form.FormPanel({
			frame: false,
			border: false,
			autoWidth: true,
			autoHeight: true,
			bodyStyle: "padding: 5px",
			defaults: {
				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				fieldLabel: _("Username"),
				name: "username",
				blankText: _("Enter your username"),
				autoComplete: true
			},{
				xtype: "passwordfield",
				fieldLabel: _("Password"),
				name: "password",
				blankText: _("Enter your password"),
				autoComplete: true
			},{
				xtype: "languagecombo",
				fieldLabel: _("Language"),
				submitValue: false
			}]
		});
		Ext.apply(this, {
			buttons: [{
				id: this.getId() + "-login",
				text: _("Login"),
				handler: this.cbLoginBtnHdl,
				scope: this
			}],
			items: this.form
		});
		OMV.LoginDialog.superclass.initComponent.apply(this, arguments);
		this.on("show", function() {
			// Set focus to field 'Username'
			var field = this.form.getForm().findField("username");
			if (!Ext.isEmpty(field)) {
				field.focus(false, 500);
			}
		}, this);
	},

	/**
	 * @method onRender
	 */
	onRender : function(ct, position) {
		OMV.LoginDialog.superclass.onRender.apply(this, arguments);
		// Monitor keyboard activities
		this.map = new Ext.KeyMap(this.el, [{
			key: [ 10, 13 ],
			fn: this.cbLoginBtnHdl,
			scope: this
		}]);
	},

	/**
	 * @method cbLoginBtnHdl
	 * Method that is called when the 'Login' button is pressed.
	 */
	cbLoginBtnHdl : function() {
		// Get the username and password
		var basicForm = this.form.getForm();
		var values = basicForm.getValues();
		// Execute RPC
		OMV.Ajax.request(function(id, response, error) {
			if (error === null) {
				this.fireEvent("login", response);
			} else {
				OMV.MessageBox.error(null, error);
			}
		}, this, "Authentication", "login", values);
	}
});
