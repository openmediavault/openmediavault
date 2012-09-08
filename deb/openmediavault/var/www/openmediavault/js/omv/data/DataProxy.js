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
// require("js/omv/data/Connection.js")

Ext.ns("OMV.data");

/**
 * @class OMV.data.DataProxy
 * Data proxy that submits the parameters 'start', 'limit', 'sortfield' and
 * 'sortdir' to the RPC function. It is also possible to submit additional
 * parameters.
 * @param config An array containing the following fields:
 *   \em service The name of the RPC service class.
 *   \em method The name of the RPC service class method.
 *   \em extraParams An object of additional parameters.
 *   \em appendPagingParams Set to FALSE to do not append the parameters
 *   'start', 'limit', 'sortfield' and 'sortdir'. Defaults to TRUE.
 */
OMV.data.DataProxy = function(config) {
	Ext.apply(this, {
		"service": config.service,
		"method": config.method,
		"extraParams": Ext.isDefined(config.extraParams) ?
		  config.extraParams : undefined,
		"appendPagingParams": Ext.isDefined(config.appendPagingParams) ?
		  config.appendPagingParams : true
	});
	// Must define a dummy api with "read" action to satisfy
	// DataProxy#doRequest and Ext.data.Api#prepare
	var api = {};
	api[Ext.data.Api.actions.read] = true;
	OMV.data.DataProxy.superclass.constructor.call(this, {
		"api": api
	});
};
Ext.extend(OMV.data.DataProxy, Ext.data.DataProxy, {
	doRequest : function(action, rs, params, reader, callback, scope, options) {
		this.requestParams = {
			"params": params,
			"reader": reader,
			"callback": callback,
			"scope": scope,
			"options": options
		};
		OMV.Ajax.request(this.createCallback(action, rs), this, this.service,
		  this.method, this.buildParams(params));
	},

	/**
	 * Returns a callback function for a request.
	 * @param action [create|update|delete|load]
	 * @param The Store-recordset being acted upon
	 * @private
	 */
	createCallback : function(action, rs) {
		return function(id, response, error) {
			var result = null;
			var success = false;
			if (error === null) {
				try {
					result = this.requestParams.reader.readRecords(
                      response);
					success = true;
				} catch(e) {
					this.fireEvent("exception", this, "response", null,
					  this.requestParams.options, null, e);
				}
			} else {
				this.fireEvent("exception", this, "remote", null,
				  this.requestParams.options, error, null);
			}
			this.requestParams.callback.call(this.requestParams.scope,
              result, this.requestParams.options, success);
		}
	},

	/**
	 * @method buildParams
	 * Helper function to build the RPC 'params' parameter.
	 * @param params The paramaters given by Ext.data.DataProxy::load
	 * @private
	 */
	buildParams : function(params) {
		var result = {};
		// Set default parameters.
		if (this.appendPagingParams === true) {
			result = {
				"start": Ext.isNumber(params.start) ? params.start : 0,
				"limit": Ext.isNumber(params.limit) ? params.limit : -1,
				"sortfield": Ext.isString(params.sort) ? params.sort : null,
				"sortdir": Ext.isString(params.dir) ? params.dir : null
			};
		}
		// Append additional parameters
		if (Ext.isDefined(this.extraParams)) {
			result = Ext.apply(result, this.extraParams);
		}
		return result;
	}
});
