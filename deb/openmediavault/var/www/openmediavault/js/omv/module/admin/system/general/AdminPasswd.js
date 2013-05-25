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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/form/field/Password.js")

/**
 * @class OMV.module.admin.system.general.AdminPasswd
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.system.general.AdminPasswd", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.form.field.Password",
	],

	rpcService: "WebGui",
	rpcSetMethod: "setPassword",

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("Administrator password"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
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

	isValid: function() {
		var me = this;
		if(!me.callParent(arguments))
			return false;
		var values = me.getValues();
		// Check the password.
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

	doLoad: function() {
		var me = this;
		// Do not display any text in password fields by default.
		me.setValues({
			password: "",
			passwordconf: ""
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "adminpasswd",
	path: "/system/general",
	text: _("Web Administrator Password"),
	position: 20,
	className: "OMV.module.admin.system.general.AdminPasswd"
});
