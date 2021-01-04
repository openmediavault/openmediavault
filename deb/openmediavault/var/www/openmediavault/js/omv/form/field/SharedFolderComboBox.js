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
 * @class OMV.form.field.SharedFolderComboBox
 * @derived OMV.form.field.ComboBox
 * Display all existing shared folders in a combobox control.
 * @param allowNone Set to TRUE to display the 'None' list entry.
 *   Defaults to FALSE.
 * @param noneText The text of the 'None' list entry.
 */
Ext.define("OMV.form.field.SharedFolderComboBox", {
	extend: "OMV.form.field.ComboBox",
	alias: "widget.sharedfoldercombo",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.data.identifier.Empty"
	],
	uses: [
		"OMV.module.admin.privilege.sharedfolder.SharedFolder",
		"OMV.module.admin.privilege.sharedfolder.Privileges"
	],

	allowNone: false,
	noneText: _("None"),

	emptyText: _("Select a shared folder ..."),
	allowBlank: false,
	editable: false,
	forceSelection: true,
	forceEmptyValue: true,
	triggerAction: "all",
	displayField: "description",
	valueField: "uuid",
	triggers: {
		add: {
			cls: Ext.baseCSSPrefix + "form-add-trigger",
			handler: "onTrigger2Click"
		},
		show: {
			cls: Ext.baseCSSPrefix + "form-search-trigger",
			handler: "onTrigger3Click"
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
						{ name: "description", type: "string" },
						{ name: "name", type: "string" },
						{ name: "mntentref", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "ShareMgmt",
						method: "enumerateSharedFolders"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "name"
				}],
				listeners: {
					scope: me,
					load: function(store, records, successful, operation, eOpts) {
						if (me.allowNone === false)
							return;
						// Append the 'None' entry.
						store.insert(0, {
							uuid: "",
							description: me.noneText
						});
					}
				}
			})
		});
		me.callParent(arguments);
		me.on("afterrender", function() {
			// Add quick tip to the trigger button.
			var trigger = this.getTrigger("add");
			Ext.tip.QuickTipManager.register({
				target: trigger.getEl(),
				text: _("Add")
			});
			trigger = this.getTrigger("show");
			Ext.tip.QuickTipManager.register({
				target: trigger.getEl(),
				text: _("Show privileges")
			});
		}, me);
		me.on("beforedestroy", function() {
			// Remove the quick tip from the trigger button.
			[ "add", "show" ].forEach(function(id) {
				var trigger = this.getTrigger(id);
				Ext.tip.QuickTipManager.unregister(trigger.getEl());
			}, this);
		}, me);
	},

	onTrigger1Click: function() {
		var me = this;
		me.onTriggerClick();
	},

	onTrigger2Click: function() {
		var me = this;
		if (me.readOnly || me.disabled)
			return;
		Ext.create("OMV.module.admin.privilege.sharedfolder.SharedFolder", {
			title: _("Add shared folder"),
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				scope: me,
				submit: function(dlg, data) {
					// Reload the combobox store to display and select the new
					// created shared folder
					var lastOptions = this.store.lastOptions;
					Ext.apply(lastOptions, {
						scope: me, // lgtm [js/overwritten-property]
						callback: function(records, operation, success) {
							if (success) {
								var record;
								Ext.Array.each(records, function(r) {
									// Compare the shared folder name and the
									// UUID of the used file system to identify
									// it explicit
									var name = r.get("name");
									var mntentref = r.get("mntentref");
									if ((name === data.name) && (mntentref ===
									  data.mntentref)) {
										record = r;
										return false;
									}
								});
								if (record) {
									this.setValue(record.get(this.valueField));
								}
							}
						},
						scope: this
					});
					this.store.reload(lastOptions);
				}
			}
		}).show();
	},

	onTrigger3Click: function() {
		var me = this;
		if (me.disabled)
			return;
		var uuid = me.getValue();
		if (Ext.isEmpty(uuid))
			return;
		Ext.create("OMV.module.admin.privilege.sharedfolder.Privileges", {
			uuid: uuid,
			readOnly: me.readOnly
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
