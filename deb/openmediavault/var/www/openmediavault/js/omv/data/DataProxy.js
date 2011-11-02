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

Ext.ns("OMV.data");

/**
 * Data proxy that submits the parameters 'start', 'limit', 'sort' and 'dir'
 * to the RPC function. It is also possible to submit additional parameters.
 * @param service The name of the RPC service class.
 * @param method The name of the RPC service class method.
 * @param extraParams An array of additional parameters.
 * @param appendDefaultParams Set to FALSE to do not append the default RPC
 * parameters 'start', 'limit', 'sort' and 'dir'. Defaults to TRUE.
 *
 * Example 1:
 * var store = new OMV.data.Store({
 *   autoLoad: true,
 *   remoteSort: false,
 *   proxy: new OMV.data.DataProxy("VolumeMgmt", "getPhysicalVolumeList"),
 *   reader: new Ext.data.JsonReader({
 *     totalProperty: "total",
 *     root: "data",
 *     fields: [
 *       { name: "uuid" },
 *       { name: "model" },
 *       { name: "capacity" },
 *       { name: "temperature" },
 *       { name: "status" },
 *       { name: "_used" }
 *     ]
 *   })
 * });
 * store.load({ params: { start: 0, limit: 50, sort: "model", dir: "DESC" } });
 *
 * Example 2:
 * var store = new OMV.data.Store({
 *   autoLoad: true,
 *   remoteSort: false,
 *   proxy: new OMV.data.DataProxy("Cron", "getListByType",
 *     [ [ "userdefined" ] ]),
 *   reader: new Ext.data.JsonReader({
 *     totalProperty: "total",
 *     root: "data",
 *     fields: [
 *       { name: "uuid" },
 *       { name: "type" },
 *       { name: "dayofweek" },
 *       { name: "hour" },
 *       { name: "minute" },
 *       { name: "username" },
 *       { name: "command" },
 *       { name: "comment" }
 *     ]
 *   })
 * });
 */
OMV.data.DataProxy = function(service, method, extraParams,
  appendDefaultParams) {
	Ext.apply(this, {
		service: service,
		method: method,
		extraParams: extraParams,
		appendDefaultParams: Ext.isDefined(appendDefaultParams) ?
		  appendDefaultParams : true
	});
	// Must define a dummy api with "read" action to satisfy
	// DataProxy#doRequest and Ext.data.Api#prepare
	var api = {};
	api[Ext.data.Api.actions.read] = true;
	OMV.data.DataProxy.superclass.constructor.call(this, {
		api: api
	});
};
Ext.extend(OMV.data.DataProxy, Ext.data.DataProxy, {
	doRequest : function(action, rs, params, reader, callback, scope, options) {
		this.requestParams = {
			params: params,
			reader: reader,
			callback: callback,
			scope: scope,
			options: options
		};
		OMV.Ajax.request(this.createCallback(action, rs), this,
			this.service, this.method, this.buildParams(params));
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
		var result = [];
		// Set default parameters. Note! Do not change the order of the
		// array parameters.
		if (this.appendDefaultParams === true) {
			result = [
				Ext.isNumber(params.start) ? params.start : 0,
				Ext.isNumber(params.limit) ? params.limit : -1,
				Ext.isString(params.sort) ? params.sort : null,
				Ext.isString(params.dir) ? params.dir : null
			];
		}
		// Append additional parameters
		if (this.extraParams) {
			result = result.concat(this.extraParams);
		}
		return result;
	}
});
