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

/**
 * @ingroup webgui
 * @class OMV.form.field.Password
 * @derived Ext.form.field.Text
 */
Ext.define("OMV.form.field.Password", {
	extend: "Ext.form.field.Text",
	alias: "widget.passwordfield",

	inputType: "password",
	triggers: {
		show: {
			cls: Ext.baseCSSPrefix + "form-password-trigger",
			handler: "onTrigger1Click"
		}
	},

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		me.on("afterrender", function() {
			// Add quick tip to the trigger button.
			var trigger = this.getTrigger("show");
			Ext.tip.QuickTipManager.register({
				target: trigger.getEl(),
				text: _("Show password")
			});
		}, me);
		me.on("beforedestroy", function() {
			// Remove the quick tip from the trigger button.
			var trigger = this.getTrigger("show");
			Ext.tip.QuickTipManager.unregister(trigger.getEl());
		}, me);
	},

	onTrigger1Click: function() {
		var me = this;
		if (me.disabled)
			return;
		var el = me.inputEl;
		if (!Ext.isIE) {
			// Firefox, Chrome, ... can change the field type dynamically.
			el.dom.type = (el.dom.type == "text") ? "password" : "text";
		} else {
			// Windows IE is not able to change the type dynamically, thus
			// a new HTML element must be created.
			var newEl = document.createElement("input");
			newEl.type = (el.dom.type == "text") ? "password" : "text";
			// Copy various attributes from the origin object.
			if (el.dom.value) newEl.value = el.dom.value;
			if (el.dom.name) newEl.name = el.dom.name;
			if (el.dom.id) newEl.id = el.dom.id;
			if (el.dom.className) newEl.className = el.dom.className;
			if (el.dom.readOnly) newEl.readOnly = el.dom.readOnly;
			if (el.dom.style.width) newEl.style.width = el.dom.style.width;
			el.replaceWith(newEl);
		}
	}
});
