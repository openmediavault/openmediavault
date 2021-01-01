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
// require("js/omv/workspace/window/Container.js")

/**
 * @ingroup webgui
 * A workspace window used to display text. The displayed text is not
 * editable by default.
 * @class OMV.workspace.window.TextArea
 * @derived OMV.workspace.window.Container
 * @param textAreaConfig A config object that will be applied to the
 *   textarea field.
 */
Ext.define("OMV.workspace.window.TextArea", {
	extend: "OMV.workspace.window.Container",
	requires: [
		"Ext.form.field.TextArea"
	],

	textAreaConfig: {
		editable: false
	},

	border: false,

	hideOkButton: true,
	hideCancelButton: true,
	hideCloseButton: false,
	hideResetButton: true,

	getWindowItems: function() {
		var me = this;
		me.ta = Ext.create(Ext.apply({
			xtype: "textarea",
			cls: Ext.baseCSSPrefix + "form-textarea-monospaced"
		}, me.getTextAreaConfig(me)));
		return me.ta;
	},

	/**
	 * Returns additional textarea configuration options.
	 * @param c This component object.
	 * @return The textarea configuration object.
	 */
	getTextAreaConfig: function(c) {
		return this.textAreaConfig;
	},

	/**
	 * Returns the textarea field.
	 * @return The textarea field object.
	 */
	getTextArea: function() {
		return this.ta;
	},

	getValues: function() {
		var me = this;
		return me.getTextArea().getValue();
	},

	setValues: function(values) {
		var me = this;
		return me.getTextArea().setValue(values);
	},

	/**
	 * Checks if any fields in this window have changed from their original
	 * values. If the values have been loaded into the window then these are
	 * the original ones.
	 * @return Returns TRUE if any fields in this window have changed from
	 *   their original values.
	 */
	isDirty: function() {
		var me = this;
		if (true === me.readOnly)
			return false;
		return me.getTextArea().isDirty();
	},

	/**
	 * Validate the text area values.
	 * @return Returns TRUE if client-side validation on the tab
	 *   is successful.
	 */
	isValid: function() {
		var me = this;
		return me.getTextArea().isValid();
	}
});
