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

Ext.ns("OMV.util");

/**
 * @class OMV.util.i18n
 * Internationalization and localization functions. The selected locale
 * is stored in a browser cookie.
 */
OMV.util.i18n = function() {
	locale = Ext.util.Cookies.get("locale");
	dictionary = {};
	return {
		getLocale: function() {
			return locale || "en";
		},

		setLocale: function(v) {
			locale = v;
			// Store the locale setting in a browser cookie which will expire
			// after one year.
			var expires = Ext.Date.add(new Date(), Ext.Date.YEAR, 1);
			Ext.util.Cookies.set("locale", locale, expires);
		},

		loadDictionary: function(data) {
			dictionary = data;
		},

		/**
		 * Lookup a string in the current domain.
		 * @return Returns a translated string if one is found in the translation
		 * table, or the submitted message if not found.
		 */
		gettext: function(str, params) {
			if(Ext.isDefined(dictionary[locale]) && Ext.isDefined(
			  dictionary[locale][str])) {
				str = dictionary[locale][str];
			}
			return str;
		}
	};
}();

function _(str, params) {
	return OMV.util.i18n.gettext.apply(this, arguments);
}

function gettext(str, params) {
	return OMV.util.i18n.gettext.apply(this, arguments);
}
