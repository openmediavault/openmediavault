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
 * @defgroup webgui The WebGUI Application Programming Interface
 */

Ext.ns("OMV");

// Configure the dynamic dependency loading capability.
Ext.Loader.setConfig({
	enabled: false, // Disable the dynamic dependency loading feature.
	disableCaching: false,
	paths: {
		OMV: "js/omv"
	}
});

// The supported languages are hardcoded and are not translated.
OMV.languages = [
	[ "en","English" ], // English
	[ "de_DE","Deutsch" ], // German
	[ "ru_RU","Русский" ], // Russian
	[ "nl_NL","Nederlands" ], // Dutch
	[ "it_IT","Italiano" ], // Italian
	[ "el_GR","ελληνικά" ], // Greek
	[ "tr_TR","Türkçe" ], // Turkish
	[ "fr_FR","Français" ], // French
	[ "pl_PL","Polski" ], // Polish
	[ "cs_CZ","Čeština" ], // Czech
	[ "es_ES","Español" ], // Spanish
	[ "gl","Galego" ], // Galician
	[ "hu_HU","Magyar" ], // Hungarian
	[ "da_DA","Dansk" ], // Danish
	[ "uk_UK","Українська" ], // Ukrainian
	[ "no_NO","Norsk" ], // Norwegian
	[ "sv_SV","Svenska" ], // Swedish
	[ "pt_PT","Português" ], // Portuguese
	[ "zh_CN","简体中文" ], // Chinese (Simplified Chinese)
	[ "zh_TW","繁體中文" ], // Chinese (Taiwan)
	[ "ja_JP","日本語" ], // Japanese (Japan)
	[ "ko_KR","한국어"] // Korean
];
