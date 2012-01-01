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
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")

Ext.ns("OMV.form");

/**
 * @class OMV.form.CertificateComboBox
 * @derived Ext.form.ComboBox
 * Display all existing certificates in a combobox control.
 * @param allowNone Set to TRUE to display the 'None' entry.
 */
OMV.form.CertificateComboBox = function(config) {
	var initialConfig = {
		emptyText: "Select a SSL certificate ...",
		allowBlank: false,
		allowNone: false,
		noneText: "None",
		editable: false,
		triggerAction: "all",
		displayField: "comment",
		valueField: "uuid",
		store: new OMV.data.Store({
			remoteSort: false,
			proxy: new OMV.data.DataProxy("CertificateMgmt", "getList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "comment" }
				]
			}),
			listeners: {
				"load": function(store, records, options) {
					if (this.allowNone === false)
						return;
					// Append the 'None' entry
					store.insert(0, new store.recordType({
						"uuid": "",
						"comment": this.noneText
					  }));
				},
				scope: this
			}
		})
	};
	Ext.apply(initialConfig, config);
	OMV.form.CertificateComboBox.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.form.CertificateComboBox, Ext.form.ComboBox, {
	initComponent : function() {
		// Display tooltip for each item
		this.tpl = '<tpl for=".">' +
		  '<div class="x-combo-list-item" ext:qtip="{' +
		  this.displayField + '}">{' + this.displayField + '}</div></tpl>';
		OMV.form.CertificateComboBox.superclass.initComponent.apply(
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
				alt: "",
				cls: "x-form-trigger x-form-search-trigger",
				"ext:qtip": "Show certificate"
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
	 * Display the SSL certificate details dialog.
	 * @param {EventObject} e
	 */
	onTrigger2Click : function(e) {
		if (this.disabled) {
			return;
		}
		var uuid = this.getValue();
		if (!Ext.isEmpty(uuid)) {
			var wnd = new OMV.Module.System.CertificateDetailDialog({
				rpcGetParams: [ uuid ]
			});
			wnd.show();
		}
	},

	getErrors : function(value) {
		var errors = OMV.form.CertificateComboBox.superclass.getErrors.apply(
		  this, arguments);
		if (this.allowNone === true) {
			if (!this.allowBlank && (value === this.noneText)) {
				errors.push(this.blankText);
			}
		}
		return errors;
	}
});
Ext.reg('certificatecombo', OMV.form.CertificateComboBox);
