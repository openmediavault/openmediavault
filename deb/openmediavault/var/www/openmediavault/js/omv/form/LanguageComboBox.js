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

Ext.ns("OMV.form");

/**
 * @class OMV.form.LanguageComboBox
 * @derived Ext.form.ComboBox
 * Display all available languages. If a menu entry is selected, the choosen
 * language will be stored in a browser cookie. If a language is stored in a
 * cookie, then this one will be displayed as default value after the field
 * has been rendered the first time.
 */
OMV.form.LanguageComboBox = function(config) {
	var initialConfig = {
		emptyText: _("Select a language ..."),
		mode: "local",
		store: new Ext.data.SimpleStore({
			fields: [ "value","text" ],
			data: [ // The supported languages are hardcoded
				[ "en",_("English") ],
				[ "de",_("German") ],
				[ "ru",_("Russian") ],
				[ "nl_NL",_("Dutch") ],
				[ "fr_FR",_("French") ],
				[ "it",_("Italian") ],
				[ "es_ES",_("Spanish") ]
			]
		}),
		displayField: "text",
		valueField: "value",
		allowBlank: false,
		editable: false,
		triggerAction: "all",
		listeners: {
			select: function(combo, record, index) {
				var locale = record.get(combo.valueField);
				OMV.i18n.setLocale(locale);
				// Force rendering of whole page with selected language.
				OMV.confirmPageUnload = false;
				document.location.reload();
			},
			scope: this
		},
		value: OMV.i18n.getLocale()
	};
	Ext.apply(initialConfig, config);
	OMV.form.LanguageComboBox.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.form.LanguageComboBox, Ext.form.ComboBox, {
});
Ext.reg('languagecombo', OMV.form.LanguageComboBox);
