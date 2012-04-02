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
// require("js/omv/data/Connection.js")
// require("js/omv/LoginDialog.js")

Ext.ns("OMV");

OMV.SessionMgr = function() {
	username = null;

	return {
		getUsername : function() {
			return this.username;
		},

		logout : function() {
			OMV.Ajax.request(function(id, response, error) {
				if (error === null) {
					// Reset the session data
					this.username = null;
					// Reload page
					OMV.confirmPageUnload = false;
					document.location.reload(true);
				} else {
					OMV.MessageBox.error(null, error);
				}
			}, this, "Authentication", "logout", null);
		}
	};
}();
