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
 * @class OMV.service.SystemInfo
 * Every event subscriber will be notified immediately with the latest data
 * after subscription. After that the \em refresh event is triggered every
 * \em refreshInterval milliseconds.
 * @param refreshInterval The frequency in milliseconds in which the
 *   system info is checked. Defaults to 5 seconds.
 */
Ext.define("OMV.service.SystemInfo", {
	singleton: true,
	requires: [
		"OMV.Rpc"
	],
	mixins: [
		"Ext.mixin.Observable"
	],

	config: {
		refreshInterval: 5000,
		data: []
	},

	constructor: function(config) {
		var me = this;
		me.mixins.observable.constructor.call(me);
		me.callParent([ config ]);
		me.refreshTask = Ext.util.TaskManager.newTask({
			run: me.doRefresh,
			scope: me,
			interval: me.getRefreshInterval(),
			fireOnStart: true
		});
		me.refreshTask.start();

		/**
		 * @event refresh
		 * Fires when the system information has been changed.
		 * @param {OMV.service.SystemInfo} this
		 * @param e
		 */
	},

	destroy: function() {
		var me = this;
		if (!Ext.isEmpty(me.refreshTask) && (me.refreshTask.isTask)) {
			me.refreshTask.destroy();
			me.refreshTask = null;
		}
		me.callParent();
	},

	addListener: function(eventName, fn, scope) {
		var me = this;
		me.mixins.observable.addListener.apply(me, arguments);
		// Immediately call the listener function with the latest data.
		if (Ext.isFunction(fn)) {
			fn.call(scope || me, me, me.getData());
		}
	},

	/**
	 * @private
	 */
	doRefresh: function() {
		var me = this;
		// Exit immediatelly if there is a pending RPC request.
		if (Ext.isDefined(me.pendingRequest) && (-1 !== me.pendingRequest))
			return;
		// Execute RPC in background, this means errors will be ignored and
		// not forwarded to the caller.
		me.pendingRequest = OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				delete me.pendingRequest;
				me.setData(response);
				me.fireEvent("refresh", me, me.getData());
			},
			rpcData: {
				service: "System",
				method: "getInformation",
				options: {
					updatelastaccess: false
				}
			}
		});
	}
});
