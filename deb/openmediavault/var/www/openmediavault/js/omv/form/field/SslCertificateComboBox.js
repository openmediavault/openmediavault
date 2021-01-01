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
// require("js/omv/workspace/window/TextArea.js")
// require("js/omv/util/Format.js")

/**
 * @ingroup webgui
 * @class OMV.form.SslCertificateComboBox
 * @derived OMV.form.field.ComboBox
 * Display all existing SSL certificates in a combobox control.
 * @param allowNone Set to TRUE to display the 'None' list entry.
 *   Defaults to FALSE.
 * @param noneText The text of the 'None' list entry.
 */
Ext.define("OMV.form.field.SslCertificateComboBox", {
	extend: "OMV.form.field.ComboBox",
	alias: [ "widget.sslcertificatecombo" ],
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.data.identifier.Empty"
	],
	uses: [
		"OMV.workspace.window.TextArea",
		"OMV.util.Format"
	],

	allowNone: false,
	noneText: _("None"),

	emptyText: _("Select a SSL certificate ..."),
	editable: false,
	displayField: "description",
	valueField: "uuid",
	forceSelection: true,
	forceEmptyValue: true,
	triggers: {
		search: {
			cls: Ext.baseCSSPrefix + "form-search-trigger",
			handler: "onTrigger2Click"
		}
	},

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
						{
							name: "description",
							type: "string",
							convert: function(value, record) {
								var validTo = record.get("validto");
								if (!Ext.isNumber(validTo)) {
									return record.get("comment");
								}
								validTo = OMV.util.Format.localeTime(
									validTo);
								return Ext.String.format(
									"{0} [{1}: {2}]",
									record.get("comment"),
									_("Valid to"),
									validTo);
							}
						}
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "CertificateMgmt",
						method: "getList"
					}
				},
				listeners: {
					scope: me,
					load: function(store, records, successful, operation, eOpts) {
						if (me.allowNone === false) {
							return;
						}
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
		me.on("afterrender", function() {
			// Add quick tip to the trigger button.
			var trigger = this.getTrigger("search");
			Ext.tip.QuickTipManager.register({
				target: trigger.getEl(),
				text: _("Show certificate")
			});
		}, me);
		me.on("beforedestroy", function() {
			// Remove the quick tip from the trigger button.
			var trigger = this.getTrigger("search");
			Ext.tip.QuickTipManager.unregister(trigger.getEl());
		}, me);
	},

	onTrigger1Click: function() {
		var me = this;
		me.onTriggerClick();
	},

	onTrigger2Click: function() {
		var me = this;
		if (me.disabled)
			return;
		var uuid = me.getValue();
		if (!Ext.isUuid(uuid))
			return;
		Ext.create("OMV.workspace.window.TextArea", {
			rpcService: "CertificateMgmt",
			rpcGetMethod: "getDetail",
			rpcGetParams: {
				uuid: uuid
			},
			title: _("Certificate details"),
			width: 600,
			height: 400
		}).show();
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
