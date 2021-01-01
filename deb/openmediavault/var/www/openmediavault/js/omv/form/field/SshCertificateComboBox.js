/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
// require("js/omv/form/field/ComboBox.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/data/identifier/Empty.js")

/**
 * @ingroup webgui
 * @class OMV.form.SshCertificateComboBox
 * @derived OMV.form.field.ComboBox
 * Display all existing SSH certificates in a combobox control.
 * @param allowNone Set to TRUE to display the 'None' list entry.
 *   Defaults to FALSE.
 * @param noneText The text of the 'None' list entry.
 */
Ext.define("OMV.form.field.SshCertificateComboBox", {
	extend: "OMV.form.field.ComboBox",
	alias: [ "widget.sshcertificatecombo" ],
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.data.identifier.Empty"
	],

	allowNone: false,
	noneText: _("None"),

	emptyText: _("Select a SSH certificate ..."),
	editable: false,
	displayField: "comment",
	valueField: "uuid",
	forceSelection: true,
	forceEmptyValue: true,

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					identifier: "empty",
					idProperty: "uuid",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "CertificateMgmt",
						method: "getSshList"
					}
				},
				listeners: {
					scope: me,
					load: function(store, records, successful, operation, eOpts) {
						if (me.allowNone === false)
							return;
						// Push the 'None' entry to the beginning of
						// dropdown the list.
						store.insert(0, {
							uuid: "",
							comment: me.noneText
						});
					}
				}
			})
		});
		me.callParent(arguments);
	},

	getErrors: function(value) {
		var me = this, errors = me.callParent(arguments);
		if (me.allowNone === true) {
			if (!me.allowBlank && (value === me.noneText))
				errors.push(me.blankText);
		}
		return errors;
	}
});
