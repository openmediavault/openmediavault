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

/**
 * @ingroup webgui
 * @class OMV.form.field.LanguageComboBox
 * @derived Ext.form.field.ComboBox
 * Display all available languages. If a menu entry is selected, the choosen
 * language will be stored in a browser cookie. If a language is stored in a
 * cookie, then this one will be displayed as default value after the field
 * has been rendered the first time.
 */
Ext.define("OMV.form.field.LanguageComboBox", {
	extend: "Ext.form.field.ComboBox",
	alias: "widget.languagecombo",

	emptyText: _("Select a language ..."),
	queryMode: "local",
	displayField: "text",
	valueField: "value",
	allowBlank: false,
	editable: false,
	forceSelection: true,

	constructor: function() {
		var me = this;
		me.callParent(arguments);
		me.addEvents(
			/**
			 * Fires when the locale name has been changed.
			 * @param this This combo box.
			 * @param locale The selected locale name.
			 */
			"localechange"
		);
	},

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: {
				type: "array",
				fields: [ "value", "text" ],
				data: OMV.languages
			},
			value: OMV.util.i18n.getLocale()
		});
		me.callParent(arguments);
		me.on("change", function(combo, newValue, oldValue, eOpts) {
			if (!Ext.isString(newValue))
				return;
			this.fireEvent("localechange", this, newValue);
		}, me);
	}
});
