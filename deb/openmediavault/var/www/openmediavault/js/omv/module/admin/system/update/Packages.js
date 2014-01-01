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

	maskBody: true,
	hidePagingToolbar: true,
	hideAddButton: true,
	hideEditButton: true,
	hideDeleteButton: true,
	stateful: true,
	stateId: "1a2ca00e-37ac-4aa4-8cbe-290d8f95bd1b",
	selType: "checkboxmodel",
	columns: [{
		text: _("Package information"),
		sortable: true,
		dataIndex: "name",
		stateId: "info",
		flex: 1,
		renderer: function(value, metaData, record) {
			var tpl = new Ext.XTemplate(
			  '<b>{name} {version}</b><br/>',
			  '{description}<br/><br/>',
			  '<tpl if="!Ext.isEmpty(values.longdescription)">',
				'{[OMV.util.Format.whitespace(values.longdescription)]}<br/><br/>',
			  '</tpl>',
			  _("Size") + ': {[OMV.util.Format.binaryUnit(values.size)]}<br/>',
			  '<tpl if="!Ext.isEmpty(values.maintainer)">',
				_("Maintainer") + ': {[Ext.String.htmlEncode(values.maintainer)]}<br/>',
			  '</tpl>',
			  '<tpl if="!Ext.isEmpty(values.homepage)">',
				_("Homepage") + ': {[Ext.String.htmlEncode(values.homepage)]}<br/>',
			  '</tpl>',
			  '<tpl if="!Ext.isEmpty(values.repository)">',
				_("Repository") + ': {[Ext.String.htmlEncode(values.repository)]}<br/>',
			  '</tpl>');
			value = tpl.apply(record.data);
			return value;
		}
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
		text: _("Description"),
		sortable: true,
		dataIndex: "description",
		stateId: "description",
		renderer: function(value, metaData, record) {
			var longdescription = record.get("longdescription");
			if(!Ext.isEmpty(longdescription)) {
				value = Ext.String.format("<b>{0}</b><br/>{1}",
				  value, Ext.String.htmlEncode(longdescription));
			}
			return OMV.util.Format.whitespace(value);
		},
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
						{ name: "description", type: "string" },
						{ name: "longdescription", type: "string" },
						{ name: "homepage", type: "string" },
						{ name: "maintainer", type: "string" },
						{ name: "size", type: "string" },
						{ name: "filename", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "Apt",
						method: "getUpgraded"
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
		Ext.Array.push(items, {
			id: me.getId() + "-changelog",
			xtype: "button",
			text: _("Show changelog"),
			icon: "images/changelog.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: Ext.Function.bind(me.onChangelogButton, me, [ me ]),
			scope: me,
			disabled: true
		});
		return items;
	},

	onSelectionChange: function(model, records) {
		var me = this;
		me.callParent(arguments);
		// Process additional buttons.
		var tbarBtnName = [ "install", "changelog" ];
		var tbarBtnDisabled = {
			"install": true,
			"changelog": true
		};
		if(records.length <= 0) {
			tbarBtnDisabled["install"] = true;
			tbarBtnDisabled["changelog"] = true;
		} else if(records.length == 1) {
			tbarBtnDisabled["install"] = false;
			tbarBtnDisabled["changelog"] = false;
		} else {
			tbarBtnDisabled["install"] = false;
			tbarBtnDisabled["changelog"] = true;
		}
		// Update the button states.
		for(var i = 0; i < tbarBtnName.length; i++) {
			var tbarBtnCtrl = me.queryById(me.getId() + "-" + tbarBtnName[i]);
			if(!Ext.isEmpty(tbarBtnCtrl)) {
				if(true == tbarBtnDisabled[tbarBtnName[i]]) {
					tbarBtnCtrl.disable();
				} else {
					tbarBtnCtrl.enable();
				}
			}
		}
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
			if(RegExp("^"+OMV.PRODUCT_PACKAGENAME+"(-\S+)?$", "i").test(
			  name)) {
				showMessageBox = true;
			}
		});
		var wnd = Ext.create("OMV.window.Execute", {
			title: _("Install updates ..."),
			rpcService: "Apt",
			rpcMethod: "upgrade",
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
				},
				close: function() {
					// Display a message box if plugins have been updated to
					// notify the user to reload the page. This is necessary
					// to let potentially new WebGUI Javascript code take
					// effect.
					if(true === showMessageBox) {
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
								if("ok" === answer) {
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
