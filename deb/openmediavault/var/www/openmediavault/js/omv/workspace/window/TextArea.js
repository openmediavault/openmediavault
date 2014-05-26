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
// require("js/omv/workspace/window/Container.js")

/**
 * @ingroup webgui
 * A workspace window used to display text.
 * @class OMV.workspace.window.TextArea
 * @derived OMV.workspace.window.Container
 * @param readOnly Mark the text area as readonly. Defaults to TRUE.
 */
Ext.define("OMV.workspace.window.TextArea", {
	extend: "OMV.workspace.window.Container",
	requires: [
		"Ext.form.field.TextArea"
	],

	readOnly: true,

	hideOkButton: true,
	hideCancelButton: true,
	hideCloseButton: false,
	hideResetButton: true,

	getWindowItems: function() {
		var me = this;
		return [{
			id: me.getId() + "-content",
			xtype: "textarea",
			readOnly: me.readOnly,
			cls: Ext.baseCSSPrefix + "form-textarea-monospaced",
			fieldStyle: {
				border: "0px"
			}
		}];
	},

	setValues: function(values) {
		var me = this;
		// Get the text area component and set the given content.
		var cmp = me.getComponent(me.getId() + "-content");
		if (Ext.isObject(cmp) && cmp.isFormField)
			cmp.setValue(values);
	}
});
