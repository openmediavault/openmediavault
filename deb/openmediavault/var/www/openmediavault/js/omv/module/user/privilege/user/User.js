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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/form/field/Password.js")

/**
 * @class OMV.module.user.privilege.user.User
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.user.privilege.user.User", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.workspace.form.Panel",
		"OMV.form.field.Password",
	],

	rpcService: "UserMgmt",
	rpcGetMethod: "getUserByContext",
	rpcSetMethod: "setUserByContext",
	hideResetButton: true,

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("User information"),
			fieldDefaults: {
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
			},{
				xtype: "textfield",
				name: "sshpubkey",
				fieldLabel: _("SSH public key"),
				allowBlank: true
			}]
		}];
	},

	isValid: function() {
		var me = this;
		if(!me.callParent(arguments))
			return false;
		var values = me.getValues();
		// Check the password
		var field = me.findField("passwordconf");
		if(values.password !== field.getValue()) {
			var msg = _("Passwords don't match");
			me.markInvalid([
				{ id: "password", msg: msg },
				{ id: "passwordconf", msg: msg }
			]);
			return false;
		}
		return true;
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form object.
	 */
	setValues: function(values) {
		var me = this;
		var result = null;
		// First clear all values
		me.reset();
		// Then set the form field values
		me.callParent(arguments);
		result = me.callParent(arguments);
		// Set form to read-only if user is not allowed to modify the user
		// account data
		if (values._readonly === true)
			this.setReadOnly(true);
		return result;
	}
});

OMV.WorkspaceManager.registerNode({
	id: "user",
	path: "/privilege",
	text: _("User"),
	icon16: "images/user.png",
	iconSvg: "images/user.svg",
	position: 10
});

OMV.WorkspaceManager.registerPanel({
	id: "user",
	path: "/privilege/user",
	text: _("User"),
	position: 10,
	className: "OMV.module.user.privilege.user.User"
});
