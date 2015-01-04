/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
 * @derived Ext.form.field.Trigger
 */
Ext.define("OMV.form.field.Password", {
	extend: "Ext.form.field.Trigger",
	alias: "widget.passwordfield",

	inputType: "password",
	triggerCls: "x-form-password-trigger",

	onRender: function() {
		var me = this;
		me.callParent(arguments);
		// Add tooltip to trigger button.
		var triggerEl = me.getTriggerButtonEl(0);
		Ext.tip.QuickTipManager.register({
			target: triggerEl.id,
			text: _("Show password")
		});
	},

	onTriggerClick: function() {
		if(this.disabled) {
			return;
		}
		var el = this.inputEl;
		if(!Ext.isIE) {
			// Firefox, Chrome, ... can change the field type dynamically.
			el.dom.type = (el.dom.type == "text") ? "password" : "text";
		} else {
			// Windows IE is not able to change the type dynamically, thus
			// a new HTML element must be created.
			var newEl = document.createElement("input");
			newEl.type = (el.dom.type == "text") ? "password" : "text";
			// Copy various attributes from the origin object.
			if(el.dom.value) newEl.value = el.dom.value;
			if(el.dom.name) newEl.name = el.dom.name;
			if(el.dom.id) newEl.id = el.dom.id;
			if(el.dom.className) newEl.className = el.dom.className;
			if(el.dom.readOnly) newEl.readOnly = el.dom.readOnly;
			if(el.dom.style.width) newEl.style.width = el.dom.style.width;
			el.replaceWith(newEl);
		}
	}
});
