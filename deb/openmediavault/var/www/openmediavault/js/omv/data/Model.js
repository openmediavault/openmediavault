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
// require("js/omv/data/field/Array.js")
// require("js/omv/data/field/Object.js")

/**
 * @ingroup webgui
 * @class OMV.data.Model
 * @derived Ext.data.Model
 */
Ext.define("OMV.data.Model", {
	extend: "Ext.data.Model",
	requires: [
		"OMV.data.field.Array",
		"OMV.data.field.Object"
	],

	statics: {
		createImplicit: function(config) {
			var className = "OMV.data.Store.ImplicitModel-" + Ext.id();
			return Ext.define(className, Ext.apply(config, {
				extend: "OMV.data.Model"
			}));
		}
	}
});
