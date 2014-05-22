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
// require("js/omv/SessionManager.js")
// require("js/omv/Rpc.js")
// require("js/omv/window/MessageBox.js")
// require("js/omv/toolbar/ApplyCfg.js")
// require("js/omv/workspace/tab/Panel.js")
// require("js/omv/workspace/node/tree/Panel.js")
// require("js/omv/workspace/node/panel/Category.js")
// require("js/omv/workspace/node/panel/Overview.js")
// require("js/omv/form/field/LanguageComboBox.js")

/**
 * @ingroup webgui
 * @class OMV.workspace.Workspace
 * @derived Ext.container.Viewport
 * A specialized container representing the viewable application area.
 */
Ext.define("OMV.workspace.Workspace", {
	extend: "Ext.container.Viewport",
	requires: [
		"OMV.SessionManager",
		"OMV.Rpc",
		"OMV.toolbar.ApplyCfg",
		"OMV.window.MessageBox",
		"OMV.form.field.LanguageComboBox",
		"OMV.workspace.node.tree.Panel"
	],
	uses: [
		"OMV.workspace.node.panel.Category",
		"OMV.workspace.node.panel.Overview",
		"OMV.workspace.tab.Panel"
	],

	constructor: function(config) {
		var me = this;
		config = Ext.apply({
			layout: "border",
			items: [
				me.header = me.buildHeader(),
				me.tree = me.buildTree(),
				me.view = me.buildView()
 			],
		}, config || {});
		me.callParent([ config ]);
	},

	mask: function(config) {
		var me = this;
		if(!Ext.isDefined(me.loadMask)) {
			Ext.apply(config, {
				target: me,
				useMsg: true
			});
			me.loadMask = new Ext.LoadMask(config);
		} else {
			Ext.apply(me.loadMask, config);
		}
		me.loadMask.show();
	},

	unmask: function() {
		var me = this;
		if(Ext.isDefined(me.loadMask))
			me.loadMask.hide();
	},

	/**
	 * @private
	 */
	buildHeader: function() {
		var me = this;
		return Ext.create("Ext.Component", {
			region: "north",
			autoEl: {
				tag: "div",
				html: "<div id='header'><a title='" + OMV.PRODUCT_NAME +
				  "' href='" + OMV.PRODUCT_URL + "' target='_blank'>" +
				  "<div id='headerlogo'></div></a><div id='headerrlogo'>" +
				  "</div></div>"
			}
		});
	},

	/**
	 * @private
	 */
	buildTree: function() {
		var me = this;
		return me.tp = Ext.create("OMV.workspace.node.tree.Panel", {
			region: "west",
			split: true,
			width: 210,
			minSize: 150,
			maxSize: 280,
			collapsible: true,
			margins: "0 0 0 0",
			layout: "fit",
			border: true,
			autoScroll: true,
			listeners: {
				scope: me,
				select: function(view, record, eOpts) {
					// Get the workspace node object.
					var node = record.get("node");
					this.onSelectNode(node, !record.isLeaf());
				}
			}
		});
	},

	/**
	 * @private
	 */
	getTree: function() {
		var me = this;
		return me.tp;
	},

	/**
	 * @private
	 */
	buildView: function() {
		var me = this;
		return me.vp = Ext.create("Ext.panel.Panel", {
			id: me.getId() + "-center",
			region: "center",
			layout: "fit",
			dockedItems: [ me.tb = Ext.create("Ext.toolbar.Toolbar", {
				dock: "top",
				height: 25,
				items: [{
					xtype: "tbtext",
					id: me.getId() + "-text",
					text: "&nbsp;"
				},{
					xtype: "tbfill"
				},{
					xtype: "languagecombo"
				},{
					xtype: "tbseparator"
				},{
					xtype: "splitbutton",
					icon: "images/menu.png",
					iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
					handler: function() {
						this.showMenu();
					},
					menu: Ext.create("Ext.menu.Menu", {
						defaults: {
							iconCls: Ext.baseCSSPrefix + "menu-item-icon-16x16"
						},
						items: [{
							text: _("Reset WebGUI state"),
							action: "resetstate",
							msg: _("Do you really want to reset WebGUI state, e.g. grid column order?"),
							tooltip: _("Reset WebGUI state will restore default settings of grid columns for example."),
							icon: "images/refresh.png"
						},{
							xtype: "menuseparator"
						},{
							text: _("Logout"),
							action: "logout",
							msg: _("Do you really want to logout?"),
							icon: "images/logout.png"
						},{
							text: _("Reboot"),
							action: "reboot",
							msg: _("Do you really want to reboot the system?"),
							icon: "images/reboot.png",
							hidden: !OMV.SessionManager.isAdministrator()
						},{
							text: _("Shutdown"),
							action: "shutdown",
							msg: _("Do you really want to shutdown the system?"),
							icon: "images/shutdown.png",
							hidden: !OMV.SessionManager.isAdministrator()
						}],
						listeners: {
							click: function(menu, item, e, eOpts) {
								if(!Ext.isDefined(item.action))
									return;
								OMV.MessageBox.show({
									title: _("Confirmation"),
									msg: item.msg,
									buttons: Ext.Msg.YESNO,
									fn: function(answer) {
										if(answer !== "yes")
											return;
										switch (item.action) {
										case "resetstate":
											Ext.state.Manager.getProvider().clearAll();
											break;
										case "logout":
											OMV.SessionManager.logout();
											break;
										case "reboot":
											OMV.Rpc.request({
												scope: this,
												callback: function(id, success, response) {
													OMV.MessageBox.show({
														title: _("Information"),
														icon: Ext.Msg.INFO,
														msg: _("The system will reboot now. This may take some time ..."),
														wait: true,
														closable: false
													});
													this.waitingForResponse = false;
													this.hasRebooted = false;
													Ext.util.TaskManager.start({
														run: function() {
															if(this.waitingForResponse)
																return;
															this.waitingForResponse = true;
															// Execute RPC.
															OMV.Rpc.request({
																scope: this,
																callback: function(id, success, response) {
																	this.waitingForResponse = false;
																	if(success) {
																		if(this.hasRebooted) {
																			OMV.confirmPageUnload = false;
																			document.location.reload();
																		}
																	} else {
																		this.hasRebooted = true;
																	}
																},
																relayErrors: true,
																rpcData: {
																	service: "System",
																	method: "noop"
																}
															});
														},
														scope: this,
														interval: 5000,
														fireOnStart: true
													});
												},
												relayErrors: false,
												rpcData: {
													service: "System",
													method: "reboot"
												}
											});
											break;
										case "shutdown":
											OMV.Rpc.request({
												scope: this,
												callback: function(id, success, response) {
													OMV.confirmPageUnload = false;
													document.location.replace("shutdown.php");
												},
												relayErrors: false,
												rpcData: {
													service: "System",
													method: "shutdown"
												}
											});
											break;
										}
									},
									scope: this,
									icon: Ext.Msg.QUESTION
								});
							},
							scope: this
						}
					})
				}]
			}),{
				xtype: "applycfgtoolbar"
			}]
		});
	},

	/**
	 * @private
	 */
	getView: function() {
		var me = this;
		return me.vp;
	},

	/**
	 * @private
	 */
	getToolbar: function() {
		var me = this;
		return me.tb;
	},

	/**
	 * Function that is called when a workspace node has been selected.
	 * @param node The workspace node object.
	 * @param iconView Set to TRUE to display the node childen in an
	 *   icon view panel.
	 */
	onSelectNode: function(node, iconView) {
		var me = this;
		if (!Ext.isObject(node) || !node.isNode)
			return;
		// Update content page.
		var selector = Ext.String.format("#{0}-center", me.getId());
		var centerPanel = me.query(selector).shift();
		if (Ext.isDefined(centerPanel)) {
			var object;
			if (false == iconView) {
				if (0 == node.getChildCount()) {
					object = Ext.create(node.getClassName(), {
						text: false,
						title: node.getText()
					});
				} else if (1 == node.getChildCount()) {
					// Display a single panel.
					var childNode = node.getChildAt(0);
					object = Ext.create(childNode.getClassName(), {
						text: false,
						// Note, do not set the title here, otherwise it
						// will be rendered automatically what is not
						// wanted here.
						//title: childNode.getText()
					});
				} else {
					// Create a tab panel containg all registered panels.
					object = Ext.create("OMV.workspace.tab.Panel");
					// Sort child nodes based on their position.
					node.eachChild(function(childNode) {
						if (!childNode.isLeaf())
							return;
						object.add(Ext.create(childNode.getClassName(), {
							title: childNode.getText()
						}));
					});
					object.setActiveTab(0);
				}
			} else {
				var className = "OMV.workspace.node.panel.Category";
				if (node.isRoot())
					className = "OMV.workspace.node.panel.Overview";
				object = Ext.create(className, {
					root: node,
					listeners: {
						scope: me,
						select: function(panel, node) {
							me.getTree().selectPathByNode(node);
						}
					}
				});
			}
			// Display the panel.
			centerPanel.removeAll(true);
			centerPanel.add(object);
			// Remove all  workspace node navigation buttons.
			Ext.Array.each(me.getToolbar().query("*[cls=" +
			  Ext.baseCSSPrefix + "workspace-node-toolbar-item]"),
			  function(c) {
				me.getToolbar().remove(c);
			});
			// Add workspace node navigation buttons.
			var components = [];
			node.bubble(function(parentNode) {
				// Insert a button per workspace node.
				var config = {
					xtype: "button",
					cls: Ext.baseCSSPrefix + "workspace-node-toolbar-item",
					handler: function() {
						var iconView = (parentNode.getPath() == "/");
						me.onSelectNode(parentNode, iconView);
					}
				};
				if (!parentNode.isRoot()) {
					Ext.apply(config, {
						text: parentNode.getText()
					});
				}
				if (parentNode.hasIcon("raster16")) {
					Ext.apply(config, {
						icon: parentNode.getIcon16(),
						iconCls: Ext.baseCSSPrefix + "btn-icon-16x16"
					});
				}
				Ext.Array.insert(components, 0, [ config ]);
				// Insert separator?
				if (!parentNode.isRoot()) {
					Ext.Array.insert(components, 0, [{
						xtype: "tbseparator",
						cls: Ext.baseCSSPrefix + "workspace-node-toolbar-item"
					}]);
				}
			});
			me.getToolbar().insert(0, components);
		}
	}
});
