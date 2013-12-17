/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
// require("js/omv/window/MessageBox.js")

/**
 * @ingroup webgui
 * @class OMV.Rpc
 * @derived Ext.data.Connection
 */
Ext.define("OMV.Rpc", {
	extend: "Ext.data.Connection",
	requires: [ "OMV.window.MessageBox" ],
	singleton: true,
	autoAbort: false,

	config: {
		disabled: false
	},

	/**
	 * Sends a RPC request to a remote server.
	 * @param options An object which may contain the following properties:
	 *   \li callback The function to be called upon receipt of the HTTP
	 *     response. The callback is called regardless of success or failure
	 *     and is passed the following parameters:
	 *     \li id The request id.
	 *     \li success True if the request succeeded.
	 *     \li response The object containing the response data.
	 *   \li scope The scope in which to execute the callback.
	 *   \li rpcData The RPC parameters.
	 *     \li service The name/class of the service to be executed.
	 *     \li method The method name to be executed.
	 *     \li params The parameters of the method to be executed as object.
	 *     \li options Optional RPC options. Defaults to NULL.
	 *   \li relayErrors Set to FALSE to do not relay errors to the caller.
	 *     Defaults to TRUE.
	 *   \li showErrors Set to FALSE to do not show an error message
	 *     dialog in case of an error. This option is only used when
	 *     relayErrors is set to FALSE. Defaults to TRUE.
	 * @return The id of the server transaction or -1 if requests are
	 *   disabled.
	 */
	request: function(options) {
		var me = this;
		if(true === me.disabled)
			return null;
		// Apply default values.
		options = Ext.apply({
			timeout: OMV.HTTPREQUEST_TIMEOUT,
			relayErrors: true,
			showErrors: true
		}, options);
		// Apply generic values that can't be modified.
		Ext.apply(options, {
			url: "rpc.php",
			method: "POST",
			jsonData: {
				service: options.rpcData.service,
				method: options.rpcData.method,
				params: options.rpcData.params || null,
				options: options.rpcData.options || null
			},
			callback: me.createRequestCallback(options.callback)
		});
		return me.callParent([ options ]);
	},

	createRequestCallback: function(callback) {
		var me = this;
		return function(options, success, response) {
			// Change back origin callback function.
			options.callback = callback;
			// Process RPC response.
			me.processResponse(options, success, response);
		};
	},

	/**
	 * The function to be called upon receipt of the HTTP response.
	 * @private
	 */
	processResponse: function(options, success, response) {
		var me = this;
		var rpcResponse = null;

		if(success) {
			try {
				// Decode RPC response. It contains the fields called
				// response and error.
				if(!Ext.isEmpty(response.responseText)) {
					var o = Ext.JSON.decode(response.responseText);
					// Check if RPC response contains an error object.
					if(Ext.isObject(o.error)) {
						success = false;
						rpcResponse = o.error;
					} else {
						rpcResponse = o.response;
					}
				}
			} catch(e) {
				success = false;
				rpcResponse = {
					code: null,
					message: "",
					trace: response.responseText || e.toString()
				};
			}
		} else {
			rpcResponse = {
				code: null,
				message: "",
				trace: response.responseText || response.statusText
			};
		}

		// Handle special errors.
		if(!success) {
			var abort = false;
			var reload = false;
			// Translate various error messages and decide if RPC response
			// delivery is aborted.
			switch(rpcResponse.code) {
			case OMV.E_SESSION_NOT_AUTHENTICATED:
				abort = true;
				reload = true;
				rpcResponse.message = _("Session not authenticated.");
				break;
			case OMV.E_SESSION_TIMEOUT:
				abort = true;
				reload = true;
				rpcResponse.message = _("Session expired.");
				break;
			}
			// Reload page and display a message box?
			if(true === reload) {
				// Disable further request, otherwise error message dialogs
				// (e.g. 'Invalid context') will pop up.
				me.setDisabled(true);
				// Display a dialog forcing user to click 'OK' to reload
				// the page.
				OMV.MessageBox.guru({
					msg: rpcResponse.message,
					fn: function() {
						OMV.confirmPageUnload = false;
						// Force browser to reload document. The login
						// dialog will be displayed then.
						document.location.reload(true);
					}
				});
			}
			// Abort RPC response delivery?
			if(true === abort)
				return;
		}

		// Handle other errors.
		if(!success) {
			// Relay/Forward the error?
			if(Ext.isDefined(options.relayErrors) && !options.relayErrors) {
				// Display an error message dialog.
				if(options.showErrors) {
					OMV.MessageBox.error(null, rpcResponse);
				}
				return;
			}
		}

		// Call the given callback function.
		if(Ext.isFunction(options.callback)) {
			options.callback.call(options.scope || me, response.requestId,
			  success, rpcResponse);
		}
	}
});

/**
 * @ingroup webgui
 * @class OMV.RpcRunner
 * @derived Ext.util.Observable
 * Provides the ability to execute a RPC while displaying a waiting dialog.
 */
Ext.define("OMV.RpcRunner", {
	extend: "Ext.util.Observable",
	requires: [
		"OMV.Rpc",
		"OMV.window.MessageBox"
	],
	singleton: true,

	/**
	 * Execute the given RPC and displays a modal waiting dialog while the
	 * given RPC is running.
	 * @param options An object which may contain the following properties:
	 *   \li title The title bar text. Defaults to null.
	 *   \li msg The message box body text.
	 *   \li finish The function to be called upon the RPC has been finished.
	 *   \li scope The scope in which to execute the callbacks. Defaults to
	 *     the browser window.
	 *   \li rpcDelay The milliseconds to delay the RPC request. Default is 500.
	 *   \li rpcData The RPC parameters. See OMV.Rpc.request.
	 *     \li service The name/class of the service to be executed.
	 *     \li method The method name to be executed.
	 *     \li params The parameters of the method to be executed as object.
	 *     \li options Optional RPC options. Defaults to NULL.
	 */
	request: function(options) {
		var me = this;
		options = Ext.apply(options, {
			title: null,
			rpcDelay: 500
		});
		// Display a waiting dialog while the RPC is running.
		OMV.MessageBox.wait(options.title, options.msg);
		// Execute RPC.
		var isRunningFn = function(id, success, response) {
			if(!success) {
				OMV.MessageBox.hide();
				OMV.MessageBox.error(null, response);
			} else {
				if(response.running === true) {
					Ext.Function.defer(function() {
						// Execute RPC.
						OMV.Rpc.request({
							scope: this,
							callback: isRunningFn,
							relayErrors: true,
							rpcData: {
								service: "Exec",
								method: "isRunning",
								params: {
									filename: response.filename
								}
							}
						});
					}, options.rpcDelay, this);
				} else {
					OMV.MessageBox.hide();
					if(Ext.isFunction(options.finish))
						options.finish.call(options.scope || window);
				}
			}
		};
		OMV.Rpc.request({
			  scope: me,
			  callback: function(id, success, response) {
				  if(!success) {
					  OMV.MessageBox.hide();
					  OMV.MessageBox.error(null, response);
				  } else {
					  // Execute RPC.
					  OMV.Rpc.request({
						  scope: me,
						  callback: isRunningFn,
						  relayErrors: true,
						  rpcData: {
							  service: "Exec",
							  method: "isRunning",
							  params: {
								  "filename": response
							  }
						  }
					  });
				  }
			  },
			  relayErrors: true,
			  rpcData: options.rpcData
		  });
	}
});
