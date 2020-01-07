/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
// require("js/omv/SessionManager.js")
// require("js/omv/Rpc.js")
// require("js/omv/window/MessageBox.js")

/**
 * @ingroup webgui
 * @class OMV.toolbar.ApplyCfg
 * @derived Ext.toolbar.Toolbar
 * A toolbar that is shown when configuration changes have been done, thus
 * they can be applied. During the configuration changes are applied a
 * progress dialog will be displayed. The toolbar is hidden if no
 * outstanding configuration changes are available.
 * @param msgText The message to be displayed.
 * @param reloadInterval The frequency in milliseconds in which it is
 *   checked whether there are any outstanding configuration changes that
 *   can be applied. Defaults to 5 seconds.
 */
Ext.define("OMV.toolbar.ApplyCfg", {
	extend: "Ext.toolbar.Toolbar",
	alias: "widget.applycfgtoolbar",
	requires: [
		"OMV.SessionManager",
		"OMV.Rpc",
		"OMV.window.MessageBox"
	],

	msgText: _("The configuration has been changed. You must apply the changes in order for them to take effect."),
	reloadInterval: 5000,

	hidden: true,

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			cls: Ext.baseCSSPrefix + "toolbar-applycfg",
			items: [{
				xtype: "tbitem",
				cls: Ext.baseCSSPrefix + "message-box-warning"
			},{
				xtype: "tbtext",
				flex: 1,
				text: OMV.util.Format.whitespace(me.msgText)
			},{
				xtype: "button",
				text: _("Apply"),
				iconCls: "x-fa fa-check",
				tooltip: _("Apply configuration changes"),
				listeners: {
					scope: me,
					click: function(c, e) {
						OMV.MessageBox.confirm(null,
							_("Do you really want to apply the configuration changes?"),
							function(answer) {
								if (answer !== "yes")
									return;
								// Hide the toolbar during the process.
								this.inProgress = true;
								this.setVisible(false);
								OMV.RpcObserver.request({
									msg: _("Apply configuration changes"),
									rpcData: {
										service: "Config",
										method: "applyChangesBg",
										params: {
											modules: [],
											force: false
										}
									},
									finish: function() {
										delete this.inProgress;
									},
									exception: function() {
										delete this.inProgress;
									},
									scope: this
								});
							}, this);
					}
				}
			},{
				xtype: "button",
				text: _("Revert"),
				iconCls: "x-fa fa-undo",
				tooltip: _("Revert configuration changes"),
				listeners: {
					scope: me,
					click: function(c, e) {
						OMV.MessageBox.confirm(null,
							_("Do you really want to revert the configuration changes?"),
							function(answer) {
								if (answer !== "yes")
									return;
								// Hide the toolbar during the process.
								this.inProgress = true;
								this.setVisible(false);
								OMV.RpcObserver.request({
									msg: _("Revert configuration changes"),
									rpcData: {
										service: "Config",
										method: "revertChangesBg",
										params: {
											filename: ""
										}
									},
									finish: function() {
										delete this.inProgress;
									},
									exception: function() {
										delete this.inProgress;
									},
									scope: this
								});
							}, this);
					}
				}
			}]
		});
		me.callParent(arguments);
	},

	setVisible: function(visible) {
		var me = this;
		// If user does not have administrator privileges, then never
		// show this toolbar.
		if (!OMV.SessionManager.isAdministrator())
			visible = false;
		return me.callParent([ visible ]);
	},

	afterRender: function() {
		var me = this;
		if (Ext.isEmpty(me.reloadTask)) {
			me.reloadTask = Ext.util.TaskManager.newTask({
				run: me.doCheck,
				scope: me,
				interval: me.reloadInterval,
				fireOnStart: true
			});
			me.reloadTask.start();
		}
		me.callParent();
	},

	destroy: function() {
		var me = this;
		// Stop a running task?
		if (!Ext.isEmpty(me.reloadTask) && (me.reloadTask.isTask)) {
			me.reloadTask.destroy();
			me.reloadTask = null;
		}
		me.callParent();
	},

	/**
	 * @method doCheck
	 * Private function that checks whether the configuration is marked
	 * dirty. If the configuration is dirty, then the toolbar is shown,
	 * otherwise it is hidden.
	 */
	doCheck: function() {
		var me = this;
		// Exit immediatelly if configuration is applied at the moment.
		if (Ext.isDefined(me.inProgress) && (true === me.inProgress))
			return;
		// Exit immediatelly if there is a pending RPC request.
		if (Ext.isDefined(me.pendingRequest) && (-1 !== me.pendingRequest))
			return;
		// Execute RPC in background, this means errors will be ignored and
		// not forwarded to the caller.
		me.pendingRequest = OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				delete me.pendingRequest;
				if (true === success) {
					me.setVisible(response);
				}
			},
			rpcData: {
				service: "Config",
				method: "isDirty",
				options: {
					updatelastaccess: false
				}
			}
		});
	}
});
