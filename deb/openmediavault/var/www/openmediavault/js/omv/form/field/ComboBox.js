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
 * @class OMV.form.field.ComboBox
 * @derived Ext.form.field.ComboBox
 * @param emptyValue The value that is returned when no selection is done.
 *   Defaults to empty string.
 * @param forceEmptyValue Set to TRUE to force the submittion of the
 *   emtpyValue to the server when the form is submitted and no item is
 *   selected. Defaults to FALSE.
 */
Ext.define("OMV.form.field.ComboBox", {
	extend: "Ext.form.field.ComboBox",

	emptyValue: "",
	forceEmptyValue: false,

	getValue: function() {
		var me = this;
		var value = me.callParent(arguments);
		if (Ext.isNull(value) && (true === me.forceEmptyValue))
			value = me.emptyValue;
		return value;
	}
});
