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
// require("js/omv/Rpc.js")

/**
 * @ingroup webgui
 * @class OMV.SessionManager
 */
Ext.define("OMV.SessionManager", {
	requires: [
		"OMV.Rpc"
	],
	singleton: true,

	config: {
		username: "",
		role: 0
	},

	constructor: function(config) {
		var me = this;
		me.initConfig(config);
	},

	/**
	 * Check whether the user has administrator privileges.
	 * Note, this is only used to show/hide elements in the WebGUI that
	 * should be visible to the administrator. Hacking the variable does
	 * not give users administrator permissions, this is validated in the
	 * backend and does not interact with the frontend.
	 * @return TRUE if the user has administrator privileges, otherwise
	 *   FALSE.
	 */
	isAdministrator: function() {
		return this.role & OMV.ROLE_ADMINISTRATOR;
	},

	logout: function() {
		var me = this;
		// Mask the whole document body.
		OMV.viewport.mask({
			msg: _("Logging out, please wait ...")
		});
		// Notify backend.
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				if (success) {
					// Force a page reload if the RPC returns successful.
					// Do not unmask the document.

					// Reset the session data.
					this.username = null;
					// Disable confirm dialog and reload the whole page.
					OMV.confirmPageUnload = false;
					document.location.reload(true);
				} else {
					// Unnask the document body and display the error.
					OMV.viewport.unmask();
					OMV.MessageBox.error(null, response);
				}
			},
			rpcData: {
				service: "Session",
				method: "logout"
			}
		});
	}
});
