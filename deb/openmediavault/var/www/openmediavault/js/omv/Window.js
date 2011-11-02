/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
// require("js/omv/MessageBox.js")
// require("js/omv/data/Connection.js")

Ext.ns("OMV");

/**
 * @class OMV.Window
 * A specialized panel intended for use as an application window.
 */
OMV.Window = function(config) {
	var initialConfig = {
		helpId: null
	};
	Ext.apply(initialConfig, config);
	OMV.Window.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.Window, Ext.Window, {
	/**
	 * Get the help identifier of this window used to identify the
	 * corresponding chapter in the online help or documentation.
	 * @return The help identifier UUID string.
	 */
	getHelpId : function() {
		return this.helpId;
	}
});
