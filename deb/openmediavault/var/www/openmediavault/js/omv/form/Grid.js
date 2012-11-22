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
// require("js/omv/grid/TBarGridPanel.js")

Ext.ns("OMV.form");

/**
 * @class OMV.form.Grid
 * @derived OMV.grid.TBarGridPanel
 * A grid panel that can be displayed within a form panel and is recognized
 * as form field.
 */
OMV.form.Grid = function(config) {
	var initialConfig = {
		mode: "local",
		isFormField: true,
		frame: true
	};
	Ext.apply(initialConfig, config);
	OMV.form.Grid.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.form.Grid, OMV.grid.TBarGridPanel, {
	/**
	 * This function must be implemented to recognise this control as
	 * form field.
	 */
	setValue : function(values) {
		return OMV.form.Grid.superclass.setValues.call(this, values);
	},

	/**
	 * This function must be implemented to recognise this control as
	 * form field.
	 */
	getValue : function() {
		return OMV.form.Grid.superclass.getValues.call(this, arguments);
	},

	/**
	 * This function must be implemented to recognise this control as
	 * form field.
	 */
	getName : function() {
		return this.name;
	},

	/**
	 * This function must be implemented to recognise this control as
	 * form field.
	 */
	markInvalid : function(msg) {
		// Empty
	},

	/**
	 * This function must be implemented to recognise this control as
	 * form field.
	 */
	clearInvalid : function() {
		// Empty
	},

	/**
	 * Validate all values in this grid.
	 * @return True if the values are valid, otherwise false.
	 */
	validate : function() {
		return true;
	},

	/**
	 * Returns whether or not the field value is currently valid by
	 * validating the processed value of the field.
	 */
	isValid : function() {
		return true;
	},

	/**
	 * Returns true if the value of this Field has been changed from its
	 * original value. Will return false if the field is disabled or has not
	 * been rendered yet.
	 */
	isDirty : function() {
		if (this.disabled || !this.rendered) {
			return false;
		}
		return OMV.form.Grid.superclass.isDirty.call(this, arguments);
	},

	/**
	 * Resets the current field value to the originally loaded value.
	 */
	reset : function() {
		// Empty
	}
});
