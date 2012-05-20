/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
// require("js/omv/NavigationPanel.js")
// require("js/omv/form/LanguageComboBox.js")

Ext.ns("OMV");

/**
 * @class OMV.Viewport
 * @derived Ext.Viewport
 * A specialized container representing the viewable application area.
 */
OMV.Viewport = function(config) {
	var initialConfig = {
		id: "viewport-component",
		layout: "border",
		items: [
			new Ext.BoxComponent({
				id: "north-panel",
				region: "north",
				cls: "x-border-panel-header",
				autoEl: {
					tag: "div",
					html: "<div id='header'><a title='" + OMV.PRODUCT_NAME +
					  "' href='" + OMV.PRODUCT_URL + "' target='_blank'>" +
					  "<div id='headerlogo'></div></a><div id='headerrlogo'>" +
					  "</div></div>"
				}
			})
		,{
			title: " ",
			region: "west",
			id: "west-panel",
			split: true,
			width: 210,
			minSize: 150,
			maxSize: 280,
			collapsible: true,
			margins: "0 0 0 0",
			layout: "fit",
			items: [
				new OMV.NavigationPanel({
					border: false,
					autoScroll: true,
					listeners: {
						click: this.onClickNavPanelHdl,
						scope: this
					}
				})
			]
		},
			new Ext.Panel({
				region: "center",
				id: "center-region",
				layout: "fit",
				tbar: new Ext.Toolbar({
					cls: "x-panel-header",
					height: 25,
					items: [{
						xtype: "tbtext",
						text: "title"
					},{
						xtype: "tbfill"
					},{
						xtype: "languagecombo",
						autoWidth: true
					},{
						xtype: "tbseparator"
					},{
						xtype: "splitbutton",
						icon: "images/wheel.png",
						handler: function() {
							this.showMenu();
						},
						menu: new Ext.menu.Menu({
							items: [{
								text: _("Reset WebGUI state"),
								action: "resetstate",
								msg: _("Do you really want to reset WebGUI state, e.g. grid column order?")
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
								icon: "images/reboot.png"
							},{
								text: _("Shutdown"),
								action: "shutdown",
								msg: _("Do you really want to shutdown the system?"),
								icon: "images/shutdown.png"
							}],
							listeners: {
								itemclick: function(item, e) {
									if (!Ext.isDefined(item.action))
										return;
									OMV.MessageBox.show({
										title: _("Confirmation"),
										msg: item.msg,
										buttons: Ext.Msg.YESNO,
										fn: function(answer) {
											if (answer == "no")
												return;
											switch (item.action) {
											case "resetstate":
												Ext.state.Manager.getProvider().clearCookies();
												break;
											case "logout":
												OMV.SessionMgr.logout();
												break;
											case "reboot":
												OMV.Ajax.request(function(id, response, error) {
													  if (error !== null) {
														  OMV.MessageBox.error(null, error);
													  } else {
														  OMV.MessageBox.show({
															  title: _("Information"),
															  icon: Ext.Msg.INFO,
															  msg: _("The system will reboot now. This may take some time ..."),
															  wait: true,
															  closable: false
														  });
														  this.waitingForResponse = false;
														  this.hasRebooted = false;
														  Ext.TaskMgr.start({
															  run: function() {
																  if (this.waitingForResponse)
																	  return;
																  this.waitingForResponse = true;
																  OMV.Ajax.request(function(id, response, error) {
																	  this.waitingForResponse = false;
																	  if (error == null) {
																		  if (this.hasRebooted) {
																			  OMV.confirmPageUnload = false;
																			  document.location.reload();
																		  }
																	  } else {
																		  this.hasRebooted = true;
																	  }
																  }, this, "System", "noop");
															  },
															  scope: this,
															  interval: 5000
														  });
													  }
												  }, this, "System", "reboot");
												break;
											case "shutdown":
												OMV.Ajax.request(function(id, response, error) {
													  if (error !== null) {
														  OMV.MessageBox.error(null, error);
													  } else {
														  OMV.confirmPageUnload = false;
														  document.location.replace("shutdown.php");
													  }
												  }, this, "System", "shutdown");
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
				})
			})
		],
		titleSeperator: " | "
	};
	Ext.apply(initialConfig, config);
	OMV.Viewport.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.Viewport, Ext.Viewport, {
	/**
	 * Handler that is called when a menu leaf has been selected in the
	 * navigation panel.
	 */
	onClickNavPanelHdl : function(node, e) {
		// Only process leafs
		if (node.leaf !== true)
			return;
		// Generate the content page title
		var title = "";
		var current = node;
		while (current.isRoot !== true) {
			title = current.text + ((title.length > 0) ? this.titleSeperator : "") + title;
			current = current.parentNode;
		}
		// Update content page
		var centerPanel = this.findById("center-region");
		if (Ext.isDefined(centerPanel)) {
			var object;
			// Get the menu configuration of the selected node and create
			// the panel(s) to be displayed.
			var menu = OMV.NavigationPanelMgr.getMenu(
			  node.attributes.categoryId, node.attributes.menuId);
			if (menu.items.getCount() == 1) {
				var item = menu.items.first();
				var initialConfig = {};
//				if (Ext.isDefined(item.title)) {
//					initialConfig.title = item.title;
//				}
				object = new item.cls(initialConfig);
			} else {
				object = new Ext.TabPanel({
					border: false,
					activeTab: 0,
					layoutOnTabChange: true
				});
				menu.items.keySort("ASC");
				menu.items.each(function(item) {
					var initialConfig = {};
					if (Ext.isDefined(item.title)) {
						initialConfig.title = item.title;
					}
					object.add(new item.cls(initialConfig));
				}, this);
			}
			// Display the panel.
			centerPanel.removeAll(true);
			centerPanel.add(object);
			centerPanel.getTopToolbar().get(0).setText(title);
			centerPanel.doLayout();
		}
	}
});
