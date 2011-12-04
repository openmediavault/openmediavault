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

Ext.ns("OMV.data");

/**
 * The class encapsulates a connection to the page's originating domain,
 * allowing requests to be made either to a configured URL, or to a URL
 * specified at request time.
 */
OMV.data.Connection = function() {
};
OMV.data.Connection.prototype = {
	init : function(url) {
		this.url = url;
		this.connection = new Ext.data.Connection({
			timeout: OMV.HTTPREQUEST_TIMEOUT
		});
		this.extraOptions = {}; // List of additional options per request
	},

	/**
	 * Sends an request to a remote server.
	 * @param callback The function to be called upon receipt of the HTTP
	 * response.
	 * @param scope The scope in which to execute the callback.
	 * @param service The name/class of the service to be executed.
	 * @param method The method name to be executed.
	 * @param params The parameters of the method to be executed as array.
	 * @return The id of the server transaction.
	 */
	request : function(callback, scope, service, method, params) {
		var o = this.connection.request({
			url: this.url,
			method: "POST",
			jsonData: {
				service: service,
				method: method,
				params: params || []
			},
			callback: this.cbRequestHdl,
			scope: this
		});
		// Store the callback and scope in an internal array, use the
		// transaction identifier as key. This is necessary because there
		// is no field in the request options to hold additional options or
		// user data.
		this.extraOptions[o.tId] = {
			callback: callback,
			scope: scope
		};
		return o;
	},

	/**
	 * Private function which handles the AJAX request response.
	 * @private
	 * @param options The parameter to the request call.
	 * @param success True if the request succeeded.
	 * @param response The XMLHttpRequest object containing the response data.
	 * See http://www.w3.org/TR/XMLHttpRequest/ for details about accessing
	 * elements of the response.
	 */
	cbRequestHdl : function(options, success, response) {
		var transactionId = response.tId;
		var responseData = {
			response: null,
			error: null
		};

		// Get the additional request options and remove them from the
		// internal list.
		if (!Ext.isDefined(this.extraOptions[transactionId])) {
			return;
		}
		var extraOptions = this.extraOptions[transactionId];
		delete(this.extraOptions[transactionId]);

		if (success) {
			try {
				if(!Ext.isEmpty(response.responseText)) {
					responseData = Ext.util.JSON.decode(response.responseText);
				}
			} catch(e) {
				responseData = {
					response: null,
					error: {
						code: null,
						message: "",
						trace: response.responseText || e.toString()
					}
				};
			}
		} else {
			responseData = {
				response: null,
				error: {
					code: null,
					message: "",
					trace: response.responseText || response.statusText
				}
			};
		}

		// Handle errors
		if (Ext.isObject(responseData.error)) {
			var abort = false;
			switch (responseData.error.code) {
			case OMV.E_SESSION_NOT_AUTHENTICATED:
			case OMV.E_SESSION_TIMEOUT:
				abort = true;
				OMV.MessageBox.failure(null, responseData.error.message,
				  function() {
					  // Force browser to reload document. The login
					  // dialog will be displayed then.
					  document.location.reload(true);
				  });
				break;
			}
			// Abort RPC response delivery?
			if (abort === true) {
				return;
			}
		}

		try {
			if (extraOptions.scope !== null) {
				extraOptions.callback.call(extraOptions.scope, transactionId,
				  responseData.response, responseData.error);
			} else {
				extraOptions.callback(transactionId, responseData.response,
				  responseData.error);
			}
		} catch(e) {
			OMV.MessageBox.failure("RPC error", "Failed to call callback " +
			  "function: " + e.message);
		}
	}
}

OMV.Ajax = new OMV.data.Connection();
OMV.Ajax.init("rpc.php");
