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
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @ingroup webgui
 * @class OMV.form.field.SharedFolderComboBox
 * @derived Ext.form.field.ComboBox
 * Display all existing shared folders in a combobox control.
 * @param allowNone Set to TRUE to display the 'None' list entry.
 *   Defaults to FALSE.
 * @param noneText The text of the 'None' list entry.
 */
Ext.define("OMV.form.field.SharedFolderComboBox", {
	extend: "Ext.form.field.ComboBox",
	alias: "widget.sharedfoldercombo",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
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
	triggerAction: "all",
	displayField: "description",
	valueField: "uuid",
	trigger2Cls: Ext.baseCSSPrefix + "form-add-trigger",
	trigger3Cls: Ext.baseCSSPrefix + "form-search-trigger",

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
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
					load: function(store, records, options) {
						if(me.allowNone === false)
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
	},

	onRender: function() {
		var me = this;
		me.callParent(arguments);
		// Add tooltip to trigger button.
		var trigger2El = me.getTriggerButtonEl(me.trigger2Cls);
		Ext.tip.QuickTipManager.register({
			target: trigger2El.id,
			text: _("Add")
		});
		var trigger3El = me.getTriggerButtonEl(me.trigger3Cls);
		Ext.tip.QuickTipManager.register({
			target: trigger3El.id,
			text: _("Show privileges")
		});
	},

	onTrigger1Click: function() {
		var me = this;
		me.onTriggerClick();
	},

	onTrigger2Click: function() {
		var me = this;
		if(me.readOnly || me.disabled)
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
						scope: me,
						callback: function(records, operation, success) {
							if(success) {
								var record;
								records.each(function(r) {
									// Compare the shared folder name and the
									// UUID of the used filesystem to identify
									// it explicit
									var name = r.get("name");
									var mntentref = r.get("mntentref");
									if((name === data.name) &&
									  (mntentref === data.mntentref)) {
										record = r;
										return false;
									}
								});
								if(record) {
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
		if(me.disabled)
			return;
		var uuid = me.getValue();
		if(Ext.isEmpty(uuid))
			return;
		Ext.create("OMV.module.admin.privilege.sharedfolder.Privileges", {
			uuid: uuid,
			readOnly: me.readOnly
		}).show();
	},

	getErrors: function(value) {
		var me = this, errors = me.callParent(arguments);
		if(me.allowNone === true) {
			if(!me.allowBlank && (value === me.noneText)) {
				errors.push(me.blankText);
			}
		}
		return errors;
	}
});
