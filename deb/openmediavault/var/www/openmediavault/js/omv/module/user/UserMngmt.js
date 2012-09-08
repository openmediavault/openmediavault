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
// require("js/omv/NavigationPanel.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/PasswordField.js")

Ext.ns("OMV.Module.Privileges");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("privileges", "user", {
	text: _("User"),
	icon: "images/user.png"
});

/**
 * @class OMV.Module.Privileges.UserPropertyPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Privileges.UserPropertyPanel = function(config) {
	var initialConfig = {
		rpcService: "UserMgmt",
 		rpcGetMethod: "getSessionUser",
		rpcSetMethod: "setSessionUser",
		hideReset: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.UserPropertyPanel.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.UserPropertyPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("User information"),
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "name",
				fieldLabel: _("Name"),
				readOnly: true,
				submitValue: false
			},{
				xtype: "textfield",
				name: "comment",
				fieldLabel: _("Comment"),
				maxLength: 65
			},{
				xtype: "textfield",
				name: "email",
				fieldLabel: _("Email"),
				allowBlank: true,
				vtype: "email"
			},{
				xtype: "passwordfield",
				name: "password",
				fieldLabel: _("Password"),
				allowBlank: true
			},{
				xtype: "passwordfield",
				name: "passwordconf",
				fieldLabel: _("Confirm password"),
				allowBlank: true,
				submitValue: false
			}]
		}];
	},

	isValid : function() {
		var valid = OMV.Module.Privileges.UserPropertyPanel.superclass.
		  isValid.call(this);
		if (valid) {
			var values = this.getValues();
			// Check the password
			var field = this.findFormField("passwordconf");
			if (values.password !== field.getValue()) {
				var msg = _("Passwords don't match");
				this.markInvalid([
					{ id: "password", msg: msg },
					{ id: "passwordconf", msg: msg }
				]);
				valid = false;
			}
		}
		return valid;
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form object.
	 */
	setValues : function(values) {
		var result = null;
		// First clear all values
		this.reset();
		// Then set the form field values
		result = OMV.Module.Privileges.UserPropertyPanel.superclass.
		  setValues.call(this, values);
		// Set form to read-only if user is not allowed to modify the user
		// account data
		if (values._readOnly === true) {
			this.setReadOnly(true);
		}
		return result;
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "user", {
	cls: OMV.Module.Privileges.UserPropertyPanel
});
