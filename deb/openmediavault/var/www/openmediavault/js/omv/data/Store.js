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
// require("js/omv/MessageBox.js")

Ext.ns("OMV.data");

/**
 * @class OMV.data.Store
 * @derived Ext.data.Store
 */
OMV.data.Store = function(config) {
	var initialConfig = {};
	Ext.applyEx(initialConfig, config);
	OMV.data.Store.superclass.constructor.call(this, initialConfig);
	// Register event handler
	this.on("exception", function(proxy, type, action, options, response,
	  arg) {
		OMV.MessageBox.error(null, response || arg);
	}, this);
};
Ext.extend(OMV.data.Store, Ext.data.Store, {
});
