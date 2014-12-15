/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/window/Execute.js")
// require("js/omv/window/Upload.js")

/**
 * @class OMV.module.admin.system.plugin.Plugins
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.system.plugin.Plugins", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format",
		"OMV.window.Execute",
		"OMV.window.Upload"
	],
	uses: [
		"Ext.XTemplate"
	],

	hidePagingToolbar: false,
	hideAddButton: true,
	hideEditButton: true,
	deleteButtonText: _("Uninstall"),
	deletionWaitMsg: "Uninstalling selected plugin",
	stateful: true,
	stateId: "2bd3835f-56c4-4047-942b-7d7b5163de2a",
	selType: "checkboxmodel",
	features: [{
		ftype: "grouping"
	}],
	columns: [{
		xtype: "booleaniconcolumn",
		text: _("Installed"),
		sortable: true,
		dataIndex: "installed",
		stateId: "installed",
		resizable: false,
		width: 80,
		align: "center"
	},{
		text: _("Package information"),
		sortable: true,
		dataIndex: "name",
		stateId: "info",
		flex: 2,
		renderer: function(value, metaData, record) {
			var tpl = Ext.create("Ext.XTemplate",
			  '<b>{name} {version}</b><br/>',
			  '{summary}<br/><br/>',
			  '<tpl if="!Ext.isEmpty(values.extendeddescription)">',
				'{[OMV.util.Format.whitespace(values.extendeddescription)]}<br/><br/>',
			  '</tpl>',
			  '<tpl if="!Ext.isEmpty(values.size)">',
			    _("Size") + ': {[OMV.util.Format.binaryUnit(values.size)]}<br/>',
			  '</tpl>',
			  '<tpl if="!Ext.isEmpty(values.maintainer)">',
				_("Maintainer") + ': {[Ext.String.htmlEncode(values.maintainer)]}<br/>',
			  '</tpl>',
			  '<tpl if="!Ext.isEmpty(values.homepage)">',
				_("Homepage") + ': <a href="{[Ext.String.htmlEncode(values.homepage)]}" ',
				'target="_blank">{[Ext.String.htmlEncode(values.homepage)]}</a><br/>',
			  '</tpl>',
			  '<tpl if="!Ext.isEmpty(values.repository)">',
				_("Repository") + ': {[Ext.String.htmlEncode(values.repository)]}<br/>',
			  '</tpl>');
			value = tpl.apply(record.data);
			return value;
		}
	},{
		text: _("Section"),
		sortable: true,
		dataIndex: "pluginsection",
		stateId: "pluginsection",
		flex: 1,
		hidden: true
	},{
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name",
		width: 180,
		hidden: true
	},{
		text: _("Version"),
		sortable: false,
		dataIndex: "version",
		stateId: "version",
		width: 120,
		hidden: true
	},{
		text: _("Repository"),
		sortable: true,
		dataIndex: "repository",
		stateId: "repository",
		width: 160,
		hidden: true
	},{
		text: _("Abstract"),
		sortable: true,
		dataIndex: "abstract",
		stateId: "abstract",
		width: 340,
		hidden: true
	},{
		xtype: "whitespacecolumn",
		text: _("Description"),
		sortable: true,
		dataIndex: "extendeddescription",
		stateId: "extendeddescription",
		width: 340,
		hidden: true
	},{
		text: _("Size"),
		sortable: true,
		dataIndex: "size",
		stateId: "size",
		renderer: OMV.util.Format.binaryUnitRenderer(),
		width: 80,
		hidden: true
	},{
		text: _("Maintainer"),
		sortable: true,
		dataIndex: "maintainer",
		stateId: "maintainer",
		width: 140,
		hidden: true
	},{
		xtype: "hyperlinkcolumn",
		text: _("Homepage"),
		sortable: true,
		dataIndex: "homepage",
		stateId: "homepage",
		width: 140,
		hidden: true
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				groupField: "pluginsection",
				model: OMV.data.Model.createImplicit({
					idProperty: "name",
					fields: [
						{ name: "_readonly", type: "boolean" },
						{ name: "name", type: "string" },
						{ name: "version", type: "string" },
						{ name: "repository", type: "string" },
						{ name: "summary", type: "string" },
						{ name: "extendeddescription", type: "string" },
						{ name: "homepage", type: "string" },
						{ name: "maintainer", type: "string" },
						{ name: "installed", type: "boolean" },
						{ name: "size", type: "string" },
						{
							name: "pluginsection",
							type: "string",
							defaultValue: "utilities",
							convert: function(value, record) {
								if (Ext.isEmpty(value))
									value = this.defaultValue;
								return Ext.String.capitalize(value);
							}
						}
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Plugin",
						method: "getList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "name"
				},{
					direction: "ASC",
					property: "pluginsection"
				}]
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		Ext.Array.insert(items, 0, [{
			id: me.getId() + "-check",
			xtype: "button",
			text: _("Check"),
			icon: "images/refresh.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onCheckButton, me, [ me ]),
			scope: me
		},{
			id: me.getId() + "-upload",
			xtype: "button",
			text: _("Upload"),
			icon: "images/upload.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onUploadButton, me, [ me ]),
			scope: me
		},{
			id: me.getId() + "-install",
			xtype: "button",
			text: _("Install"),
			icon: "images/add.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onInstallButton, me, [ me ]),
			scope: me,
			disabled: true
		}]);
		return items;
	},

	onSelectionChange: function(model, records) {
		var me = this;
		me.callParent(arguments);
		// Process additional buttons.
		var tbarBtnDisabled = {
			"install": true
		};
		if (records.length <= 0) {
			tbarBtnDisabled["install"] = true;
		} else {
			tbarBtnDisabled["install"] = false;
			// Do not enable the 'Install' button if a plugin is already
			// installed.
			Ext.Array.each(records, function(record) {
				if (true === record.get("installed"))
					tbarBtnDisabled["install"] = true;
			}, me);
		}
		// Update the button controls.
		Ext.Object.each(tbarBtnDisabled, function(key, value) {
			this.setToolbarButtonDisabled(key, value);
		}, me);
	},

	doReload: function() {
		var me = this;
		me.callParent(arguments);
		// Deselect all records.
		me.getSelectionModel().deselectAll();
	},

	onCheckButton: function() {
		var me = this;
		OMV.MessageBox.wait(null, _("Checking for new plugins ..."));
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				this.getSelectionModel().deselectAll();
				if(!success) {
					OMV.MessageBox.hide();
					OMV.MessageBox.error(null, response);
				} else {
					// Execute RPC.
					OMV.Rpc.request({
						scope: this,
						callback: this.onIsRunning,
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
			rpcData: {
				service: "Apt",
				method: "update"
			}
		});
	},

	onIsRunning: function(id, success, response) {
		var me = this;
		if(!success) {
			OMV.MessageBox.hide();
			OMV.MessageBox.error(null, response);
		} else {
			if(response.running === true) {
				Ext.Function.defer(function() {
					// Execute RPC.
					OMV.Rpc.request({
						scope: me,
						callback: me.onIsRunning,
						relayErrors: true,
						rpcData: {
							service: "Exec",
							method: "isRunning",
							params: {
								filename: response.filename
							}
						}
					});
				}, 500, me);
			} else {
				OMV.MessageBox.hide();
				me.doReload();
			}
		}
	},

	onUploadButton: function() {
		var me = this;
		Ext.create("OMV.window.Upload", {
			title: _("Upload plugin"),
			service: "Plugin",
			method: "upload",
			listeners: {
				scope: me,
				success: function(wnd, response) {
					// The upload was successful, now resynchronize the
					// package index files from their sources.
					this.onCheckButton();
				}
			}
		}).show();
	},

	onInstallButton: function() {
		var me = this;
		var records = me.getSelection();
		var packages = [];
		Ext.Array.each(records, function(record) {
			packages.push(record.get("name"));
		});
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to install the selected plugin(s)?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if(answer !== "yes")
					return;
				var wnd = Ext.create("OMV.window.Execute", {
					title: _("Installing plugins ..."),
					rpcService: "Plugin",
					rpcMethod: "install",
					rpcParams: { "packages": packages },
					rpcIgnoreErrors: true,
					hideStartButton: true,
					hideStopButton: true,
					killCmdBeforeDestroy: false,
					listeners: {
						scope: me,
						finish: function(wnd, response) {
							wnd.appendValue(_("Done ..."));
							wnd.setButtonDisabled("close", false);
						},
						exception: function(wnd, error) {
							OMV.MessageBox.error(null, error);
							wnd.setButtonDisabled("close", false);
						},
						close: function() {
							Ext.MessageBox.show({
								title: _("Information"),
								msg: _("The page will reload now to let the changes take effect."),
								modal: true,
								icon: Ext.MessageBox.INFO,
								buttons: Ext.MessageBox.OK,
								fn: function() {
									// Reload the page.
									OMV.confirmPageUnload = false;
									document.location.reload();
								}
							});
						}
					}
				});
				wnd.setButtonDisabled("close", true);
				wnd.show();
				wnd.start();
			},
			scope: me,
			icon: Ext.Msg.QUESTION
		});
	},

	onDeleteButton: function() {
		var me = this;
		var records = me.getSelection();
		var packages = [];
		Ext.Array.each(records, function(record) {
			packages.push(record.get("name"));
		});
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to uninstall the selected plugin(s)?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if(answer !== "yes")
					return;
				var wnd = Ext.create("OMV.window.Execute", {
					title: _("Uninstall plugins ..."),
					rpcService: "Plugin",
					rpcMethod: "remove",
					rpcParams: { "packages": packages },
					rpcIgnoreErrors: [ OMV.E_ENGINED_CONNECT_SOCKET ],
					hideStartButton: true,
					hideStopButton: true,
					killCmdBeforeDestroy: false,
					listeners: {
						finish: function(wnd, response) {
							wnd.appendValue(_("Done ..."));
							wnd.setButtonDisabled("close", false);
						},
						exception: function(wnd, error) {
							OMV.MessageBox.error(null, error);
							wnd.setButtonDisabled("close", false);
						},
						close: function() {
							Ext.MessageBox.show({
								title: _("Information"),
								msg: _("The page will reload now to let the changes take effect."),
								modal: true,
								icon: Ext.MessageBox.INFO,
								buttons: Ext.MessageBox.OK,
								fn: function() {
									// Reload the page.
									OMV.confirmPageUnload = false;
									document.location.reload();
								}
							});
						},
						scope: me
					}
				});
				wnd.setButtonDisabled("close", true);
				wnd.show();
				wnd.start();
			},
			scope: me,
			icon: Ext.Msg.QUESTION
		});
	}
});

OMV.WorkspaceManager.registerNode({
	id: "plugin",
	path: "/system",
	text: _("Plugins"),
	icon16: "images/puzzle.png",
	iconSvg: "images/puzzle.svg",
	position: 90
});

OMV.WorkspaceManager.registerPanel({
	id: "plugins",
	path: "/system/plugin",
	text: _("Plugins"),
	position: 10,
	className: "OMV.module.admin.system.plugin.Plugins"
});
