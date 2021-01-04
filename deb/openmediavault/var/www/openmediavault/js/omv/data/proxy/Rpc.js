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
// require("js/omv/data/reader/RpcJson.js")
// require("js/omv/window/MessageBox.js")

/**
 * @ingroup webgui
 * @class OMV.data.proxy.Rpc
 * @derived Ext.data.proxy.Ajax
 * This proxy uses AJAX requests to load data from the server delivered via
 * the OMV RPC engine.
 * @param config An array containing the following fields:
 *   \li rpcData The RPC parameters. \see OMV.Rpc.request
 *     for more details.
 *   \li extraParams An object of additional method parameters.
 *   \li appendSortParams Set to FALSE to do not append the sort parameters
 *     'start', 'limit', 'sortfield' and 'sortdir'. Defaults to TRUE.
 */
Ext.define("OMV.data.proxy.Rpc", {
	extend: "Ext.data.proxy.Ajax",
	alias: "proxy.rpc",
	requires: [
		"OMV.Rpc",
		"OMV.data.reader.RpcJson",
		"OMV.window.MessageBox"
	],
	uses: [ "Ext.data.Request" ],

	config: {
		reader: "rpcjson",
		simpleSortMode: true,
		idParam: "uuid",
		sortParam: "sortfield",
		directionParam: "sortdir",
		appendSortParams: true
	},

	constructor: function() {
		var me = this;
		Ext.apply(me, {
			timeout: OMV.HTTPREQUEST_TIMEOUT
		});
		me.callParent(arguments);
		me.on("exception", function(proxy, response, operation) {
			OMV.MessageBox.error(null, response);
		});
	},

	doRequest: function(operation) {
		var me = this;
		var request = me.buildRequest(operation);
		Ext.apply(request, {
			scope: me,
			callback: me.createRequestCallback(request, operation),
			method: me.getMethod(request),
			disableCaching: false
		});
		// Apply the request to the given operation.
		operation.setRequest(request);
		// Finally execute the RPC request.
		OMV.Rpc.request(request);
		return request;
	},

	createRequestCallback: function(request, operation) {
		var me = this;
		return function(id, success, response) {
			me.processResponse(success, operation, request, response);
		};
	},

	setException: function(operation, response) {
		operation.setException({
			status: response.code,
			statusText: response.message,
			response: response
		});
	},

	buildRequest: function(operation) {
		var me = this, request = null, rpcData = me.rpcData;
		rpcData.params = Ext.applyIf(rpcData.params || {},
		  me.extraParams || {});
		if (me.getAppendSortParams()) {
			rpcData.params = Ext.apply(rpcData.params,
			  me.getParams(operation));
		}
		request = Ext.create("Ext.data.Request", {
			action: operation.getAction(),
			operation: operation,
			proxy: me,
			rpcData: rpcData,
			relayErrors: true
		});
		// Use setTimeout() here because 'Ext.data.request.Base' uses
		// the getter method to access the timeout config option.
		request.setTimeout(me.timeout);
		return request;
	},

	getParams: function(operation) {
		var me = this,
		  params = {},
		  start = operation.getStart(),
		  limit = operation.getLimit(),
		  sorters = operation.getSorters(),
		  startParam = me.getStartParam(),
          limitParam = me.getLimitParam(),
		  sortParam = me.getSortParam(),
		  simpleSortMode = me.getSimpleSortMode(),
		  directionParam = me.getDirectionParam();
		if (startParam && Ext.isDefined(start))
			params[startParam] = start;
		if (limitParam && Ext.isDefined(limit))
			params[limitParam] = limit;
		if (sortParam && sorters && (sorters.length > 0)) {
			if (simpleSortMode) {
				params[sortParam] = sorters[0].getProperty();
				params[directionParam] = sorters[0].getDirection();
			} else {
				params[sortParam] = me.encodeSorters(sorters);
			}
		} else {
			params[sortParam] = null;
			params[directionParam] = null;
		}
		return params;
	}
});
