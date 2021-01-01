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
// require("js/omv/SessionManager.js")
// require("js/omv/service/SystemInfo.js")
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
 */
Ext.define("OMV.toolbar.ApplyCfg", {
	extend: "Ext.toolbar.Toolbar",
	alias: "widget.applycfgtoolbar",
	requires: [
		"OMV.SessionManager",
		"OMV.service.SystemInfo",
		"OMV.window.MessageBox"
	],

	msgText: _("The configuration has been changed. You must apply the changes in order for them to take effect."),

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
		if (OMV.SessionManager.isAdministrator()) {
			OMV.service.SystemInfo.on("refresh", me.onRefreshSystemInfo, me);
		}
	},

	destroy: function() {
		var me = this;
		if (OMV.SessionManager.isAdministrator()) {
			OMV.service.SystemInfo.un("refresh", me.onRefreshSystemInfo, me);
		}
		me.callParent();
	},

	onRefreshSystemInfo: function(c, info) {
		var me = this;
		if (me.isVisible() !== info.configDirty) {
			me.setVisible(info.configDirty);
		}
	}
});
