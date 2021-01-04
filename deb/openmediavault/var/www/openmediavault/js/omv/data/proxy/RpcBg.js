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
// require("js/omv/data/proxy/Rpc.js")

/**
 * @ingroup webgui
 * @class OMV.data.proxy.RpcBg
 * @derived OMV.data.proxy.Rpc
 * This proxy uses AJAX requests to load data from the server delivered via
 * the OMV RPC engine. Use this proxy if the RPC has started a background
 * process for long running tasks.
 */
Ext.define("OMV.data.proxy.RpcBg", {
	extend: "OMV.data.proxy.Rpc",
	alias: "proxy.rpcbg",

	rpcDelay: 1000, // 1 second

	createRequestCallback: function(request, operation) {
		var me = this;
		return function(id, success, response) {
			if (!success) {
				// Error handling.
				me.processResponse(success, operation, request, response);
			} else {
				// The RPC returns the name of the background status file.
				request.rpcBgStatusFilename = response;
				// Check if the background process is running until it
				// has been finished or an error occurs. If successfully
				// finished, then process the retrieved content.
				me.doGetOutput(request, operation);
			}
		};
	},

	doGetOutput: function(request, operation) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				if (success) {
					if (response.running) {
						Ext.Function.defer(this.doGetOutput, this.rpcDelay,
							this, [request, operation]);
						return;
					}
					response = Ext.JSON.decode(response.output);
				}
				this.processResponse(success, operation, request, response);
			},
			relayErrors: true,
			rpcData: {
				service: "Exec",
				method: "getOutput",
				params: {
					filename: request.rpcBgStatusFilename,
					pos: 0
				}
			}
		});
	}
});
