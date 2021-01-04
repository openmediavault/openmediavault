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
 * @class OMV.module.admin.system.update.Packages
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.system.update.Packages", {
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

	hideAddButton: true,
	hideEditButton: true,
	hideDeleteButton: true,
	stateful: true,
	stateId: "1a2ca00e-37ac-4aa4-8cbe-290d8f95bd1b",
	selModel: "checkboxmodel",
	columns: [{
		xtype: "templatecolumn",
		text: _("Package information"),
		sortable: true,
		stateId: "info",
		flex: 1,
		tpl: '<b>{name} {version}</b><br/>' +
		  '{summary}<br/><br/>' +
		  '<tpl if="!Ext.isEmpty(values.extendeddescription)">' +
			'{[OMV.util.Format.whitespace(values.extendeddescription)]}<br/><br/>' +
		  '</tpl>' +
		  _("Size") + ': {[OMV.util.Format.binaryUnit(values.size)]}<br/>' +
		  '<tpl if="!Ext.isEmpty(values.maintainer)">' +
			_("Maintainer") + ': {[Ext.String.htmlEncode(values.maintainer)]}<br/>' +
		  '</tpl>' +
		  '<tpl if="!Ext.isEmpty(values.homepage)">' +
			_("Homepage") + ': <a href="{[Ext.String.htmlEncode(values.homepage)]}" ' +
			'target="_blank">{[Ext.String.htmlEncode(values.homepage)]}</a><br/>' +
		  '</tpl>' +
		  '<tpl if="!Ext.isEmpty(values.repository)">' +
			_("Repository") + ': {[Ext.String.htmlEncode(values.repository)]}<br/>' +
		  '</tpl>'
	},{
		xtype: "textcolumn",
		text: _("Name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name",
		width: 180,
		hidden: true
	},{
		xtype: "textcolumn",
		text: _("Version"),
		sortable: false,
		dataIndex: "version",
		stateId: "version",
		width: 120,
		hidden: true
	},{
		xtype: "textcolumn",
		text: _("Repository"),
		sortable: true,
		dataIndex: "repository",
		stateId: "repository",
		width: 160,
		hidden: true
	},{
		xtype: "textcolumn",
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
		xtype: "binaryunitcolumn",
		text: _("Size"),
		sortable: true,
		dataIndex: "size",
		stateId: "size",
		width: 80,
		hidden: true
	},{
		xtype: "textcolumn",
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
				model: OMV.data.Model.createImplicit({
					idProperty: "name",
					fields: [
						{ name: "name", type: "string" },
						{ name: "version", type: "string" },
						{ name: "repository", type: "string" },
						{ name: "summary", type: "string" },
						{ name: "extendeddescription", type: "string" },
						{ name: "homepage", type: "string" },
						{ name: "maintainer", type: "string" },
						{ name: "size", type: "string" },
						{ name: "filename", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Apt",
						method: "enumerateUpgraded"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "name"
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
			iconCls: "x-fa fa-refresh",
			handler: Ext.Function.bind(me.onCheckButton, me, [ me ]),
			scope: me
		},{
			id: me.getId() + "-upload",
			xtype: "button",
			text: _("Upload"),
			iconCls: "x-fa fa-upload",
			handler: Ext.Function.bind(me.onUploadButton, me, [ me ]),
			scope: me
		},{
			id: me.getId() + "-install",
			xtype: "button",
			text: _("Install"),
			iconCls: "x-fa fa-plus",
			handler: Ext.Function.bind(me.onInstallButton, me, [ me ]),
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1
			}
		}]);
		Ext.Array.push(items, {
			id: me.getId() + "-changelog",
			xtype: "button",
			text: _("Show changelog"),
			iconCls: "x-fa fa-file-text",
			handler: Ext.Function.bind(me.onChangelogButton, me, [ me ]),
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1
			}
		});
		return items;
	},

	onInstallButton: function() {
		var me = this;
		var records = me.getSelection();
		var packages = [];
		var showMessageBox = false;
		Ext.Array.each(records, function(record) {
			var name = record.get("name");
			packages.push(name);
			// Is it a plugin?
			if (RegExp("^"+OMV.PRODUCT_PACKAGENAME+"(-\S+)?$",
			  "i").test(name)) {
				showMessageBox = true;
			}
		});
		var wnd = Ext.create("OMV.window.Execute", {
			title: _("Installing updates ..."),
			rpcService: "Apt",
			rpcMethod: "install",
			rpcParams: { "packages": packages },
			rpcIgnoreErrors: true,
			hideStartButton: true,
			hideStopButton: true,
			listeners: {
				scope: me,
				finish: function(wnd, response) {
					wnd.appendValue(_("Done ..."));
					wnd.setButtonDisabled("close", false);
				},
				exception: function(wnd, error) {
					OMV.MessageBox.error(null, error);
					wnd.setButtonDisabled("close", false);
					showMessageBox = false;
				},
				close: function() {
					// Display a message box if plugins have been updated to
					// notify the user to reload the page. This is necessary
					// to let potentially new WebGUI Javascript code take
					// effect.
					if (true === showMessageBox) {
						Ext.MessageBox.show({
							title: _("Information"),
							msg: _("Please reload the page to let the changes take effect."),
							modal: true,
							icon: Ext.MessageBox.INFO,
							buttons: Ext.MessageBox.OKCANCEL,
							buttonText: {
								ok: _("Reload"),
								cancel: _("Close")
							},
							fn: function(answer) {
								if ("ok" === answer) {
									// Reload the page.
									OMV.confirmPageUnload = false;
									document.location.reload();
								} else {
									// Reload the grid.
									me.doReload();
								}
							}
						});
					} else {
						me.doReload();
					}
				}
			}
		});
		wnd.setButtonDisabled("close", true);
		wnd.show();
		wnd.start();
	},

	onUploadButton: function() {
		var me = this;
		Ext.create("OMV.window.Upload", {
			title: _("Upload package"),
			service: "Apt",
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

	onCheckButton: function() {
		var me = this;
		OMV.RpcObserver.request({
			msg: _("Checking for new updates ..."),
			rpcData: {
				service: "Apt",
				method: "update"
			},
			scope: me,
			finish: function() {
				this.doReload();
			}
		});
	},

	onChangelogButton: function() {
		var me = this;
		var record = me.getSelected();
		var wnd = Ext.create("OMV.window.Execute", {
			title: _("Changelog"),
			rpcService: "Apt",
			rpcMethod: "getChangeLog",
			rpcParams: {
				filename: record.get("filename")
			},
			hideStartButton: true,
			hideStopButton: true,
			scrollBottom: false,
			listeners: {
				scope: me,
				finish: function(wnd, response) {
					wnd.setButtonDisabled("close", false);
				},
				exception: function(wnd, error) {
					OMV.MessageBox.error(null, error);
					wnd.close();
				}
			}
		});
		wnd.setButtonDisabled("close", true);
		wnd.show();
		wnd.start();
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "packages",
	path: "/system/update",
	text: _("Updates"),
	position: 10,
	className: "OMV.module.admin.system.update.Packages"
});
