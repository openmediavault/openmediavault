/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")

Ext.ns("OMV.form");

/**
 * @class OMV.form.SharedFolderComboBox
 * @derived Ext.form.ComboBox
 * Display all existing shared folders in a combobox control.
 * @param allowNone Set to TRUE to display the 'None' entry.
 */
OMV.form.SharedFolderComboBox = function(config) {
	var initialConfig = {
		emptyText: "Select a shared folder ...",
		allowBlank: false,
		allowNone: false,
		noneText: "None",
		editable: false,
		triggerAction: "all",
		displayField: "description",
		valueField: "uuid",
		store: new OMV.data.Store({
			remoteSort: false,
			proxy: new OMV.data.DataProxy("ShareMgmt", "getList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "description" },
					{ name: "name" },
					{ name: "mntentref" }
				]
			}),
			listeners: {
				"load": function(store, records, options) {
					if (this.allowNone === false)
						return;
					// Append the 'None' entry
					store.insert(0, new store.recordType({
						"uuid": "",
						"description": this.noneText
					  }));
				},
				scope: this
			}
		})
	};
	Ext.apply(initialConfig, config);
	OMV.form.SharedFolderComboBox.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.form.SharedFolderComboBox, Ext.form.ComboBox, {
	initComponent : function() {
		OMV.form.SharedFolderComboBox.superclass.initComponent.apply(
		  this, arguments);
		this.triggerConfig = {
			tag: "span",
			cls: "x-form-twin-triggers",
			cn: [{
				tag: "img",
				src: Ext.BLANK_IMAGE_URL,
				alt: "",
				cls: "x-form-trigger " + this.triggerClass
			},{
				tag: "img",
				src: Ext.BLANK_IMAGE_URL,
				cls: "x-form-trigger x-form-add-trigger",
				"ext:qtip": "Add"
			},{
				tag: "img",
				src: Ext.BLANK_IMAGE_URL,
				alt: "",
				cls: "x-form-trigger x-form-search-trigger",
				"ext:qtip": "Show privileges"
			}]
		};
	},

	/**
	 * Override the default function to handle multiple triggers
	 * @private
	 */
	initTrigger : function() {
		var ts = this.trigger.select(".x-form-trigger", true);
		var triggerField = this;
		ts.each (function(t, all, index) {
			var triggerIndex = "Trigger" + (index + 1);
			t.hide = function() {
				var w = triggerField.wrap.getWidth();
				this.dom.style.display = "none";
				triggerField.el.setWidth(w - triggerField.trigger.getWidth());
				this["hidden" + triggerIndex] = true;
			};
			t.show = function() {
				var w = triggerField.wrap.getWidth();
				this.dom.style.display = "";
				triggerField.el.setWidth(w - triggerField.trigger.getWidth());
				this["hidden" + triggerIndex] = false;
			};
			if (this["hide" + triggerIndex]) {
				t.dom.style.display = "none";
				this["hidden" + triggerIndex] = true;
			}
			this.mon(t, "click", this["on" + triggerIndex + "Click"], this,
				{ preventDefault: true });
			t.addClassOnOver("x-form-trigger-over");
			t.addClassOnClick("x-form-trigger-click");
		}, this);
		this.triggers = ts.elements;
	},

	/**
	 * Override the default function to handle multiple triggers
	 * @private
	 */
	getTrigger : function(index) {
		return this.triggers[index];
	},

	/**
	 * Override the default function to handle multiple triggers
	 * @private
	 */
	getTriggerWidth : function() {
		var tw = 0;
		Ext.each (this.triggers, function(t, index) {
			var triggerIndex = "Trigger" + (index + 1),
				w = t.getWidth();
			if (w === 0 && !this["hidden" + triggerIndex]) {
				tw += this.defaultTriggerWidth;
			} else {
				tw += w;
			}
		}, this);
		return tw;
	},

	/**
	 * Show the dropdown list.
	 * @param {EventObject} e
	 */
	onTrigger1Click : function(e) {
		this.onTriggerClick();
	},

	/**
	 * Display the dialog to create a new shared folder dialog.
	 * @param {EventObject} e
	 */
	onTrigger2Click : function(e) {
		if (this.readOnly || this.disabled) {
			return;
		}
		var wnd = new OMV.Module.Privileges.SharedFolderPropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function(dlg, data) {
					// Reload the combobox store to display and select the new
					// created shared folder
					var lastOptions = this.store.lastOptions;
					Ext.apply(lastOptions, {
						callback: function(records, options, success) {
							if (success) {
								var record;
								records.each(function(r) {
									// Compare the shared folder name and the
									// UUID of the used filesystem to identify
									// it explicit
									var name = r.get("name");
									var mntentref = r.get("mntentref");
									if ((name === data.name) &&
									  (mntentref === data.mntentref)) {
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
				},
				scope: this
			}
		});
		wnd.show();
	},

	/**
	 * Display the shares privileges dialog.
	 * @param {EventObject} e
	 */
	onTrigger3Click : function(e) {
		if (this.disabled) {
			return;
		}
		var uuid = this.getValue();
		if (!Ext.isEmpty(uuid)) {
			var wnd = new OMV.Module.Privileges.PrivilegesPropertyDialog({
				uuid: uuid,
				readOnly: this.readOnly
			});
			wnd.show();
		}
	},

	getErrors : function(value) {
		var errors = OMV.form.SharedFolderComboBox.superclass.getErrors.apply(
		  this, arguments);
		if (this.allowNone === true) {
			if (!this.allowBlank && (value === this.noneText)) {
				errors.push(this.blankText);
			}
		}
		return errors;
	}
});
Ext.reg('sharedfoldercombo', OMV.form.SharedFolderComboBox);
