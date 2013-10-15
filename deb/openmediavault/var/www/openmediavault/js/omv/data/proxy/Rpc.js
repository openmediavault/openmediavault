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
// require("js/omv/Rpc.js")
// require("js/omv/data/reader/RpcJson.js")
// require("js/omv/window/MessageBox.js")

/**
 * @ingroup webgui
 * @class OMV.data.proxy.Rpc
 * @derived Ext.data.proxy.Ajax
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

	defaultReaderType: "rpcjson",
	simpleSortMode: true,
	idParam: "uuid",
	sortParam: "sortfield",
	directionParam: "sortdir",
	appendSortParams: true,
	timeout: OMV.HTTPREQUEST_TIMEOUT,

	constructor: function() {
		var me = this;
		me.callParent(arguments);
		me.on("exception", function(proxy, response, operation) {
			OMV.MessageBox.error(null, response);
		});
	},

	doRequest: function(operation, callback, scope) {
		var me = this;
		var request = me.buildRequest(operation);
		Ext.apply(request, {
			timeout: me.timeout,
			scope: me,
			callback: me.createRequestCallback(request, operation,
			  callback, scope),
			method: me.getMethod(request),
			disableCaching: false
		});
		OMV.Rpc.request(request);
		return request;
	},

	createRequestCallback: function(request, operation, callback, scope) {
		var me = this;
		return function(id, success, response) {
			me.processResponse(success, operation, request, response,
			  callback, scope);
		};
	},

	buildRequest: function(operation) {
		var me = this, request = null, rpcData = me.rpcData;
		rpcData.params = Ext.applyIf(rpcData.params || {},
		  me.extraParams || {});
		if(this.appendSortParams) {
			rpcData.params = Ext.apply(rpcData.params,
			  me.getParams(operation));
		}
		request = Ext.create("Ext.data.Request", {
			action: operation.action,
			operation: operation,
			proxy: me,
			rpcData: rpcData,
			relayErrors: true
		});
		operation.request = request;
		return request;
	},

	getParams: function(operation) {
		var me = this, params = {};
		if(me.startParam && Ext.isDefined(operation.start)) {
			params[me.startParam] = operation.start;
		}
		if(me.limitParam && Ext.isDefined(operation.limit)) {
			params[me.limitParam] = operation.limit;
		}
		if(me.sortParam && operation.sorters &&
		  (operation.sorters.length > 0)) {
			if(me.simpleSortMode) {
				params[me.sortParam] = operation.sorters[0].property;
				params[me.directionParam] = operation.sorters[0].direction;
			} else {
				params[me.sortParam] = me.encodeSorters(operation.sorters);
			}
		} else {
			params[me.sortParam] = null;
			params[me.directionParam] = null;
		}
		return params;
	}
});
