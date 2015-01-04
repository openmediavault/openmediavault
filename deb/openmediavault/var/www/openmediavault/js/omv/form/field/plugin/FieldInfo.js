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
 * @class OMV.form.field.plugin.FieldInfo
 * @derived Ext.AbstractPlugin
 */
Ext.define("OMV.form.field.plugin.FieldInfo", {
	extend: "Ext.AbstractPlugin",
	alias: "plugin.fieldinfo",

	config: {
		cls: "x-form-fieldinfo",
		text: ""
	},

	init: function(c) {
		var me = this;
		if(!c.isFieldLabelable)
			return;
		if(!Ext.isEmpty(me.text)) {
			c.afterSubTpl = Ext.String.format("<div class='{0}'>{1}</div>",
			  me.getCls(), me.getText());
		}
	}
});
