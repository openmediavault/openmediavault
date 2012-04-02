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

Ext.ns("OMV");

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
					html: "<div id='header'><a title='www.openmediavault.org' href='http://www.openmediavault.org' target='_blank'><div id='headerlogo'></div></a><div id='headerrlogo'></div></div>"
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
						xtype: "tbbutton",
						icon: "images/logout.png",
						text: "Logout",
						handler: function() {
							Ext.MessageBox.show({
								title: "Confirmation",
								msg: "Do you really want to logout?",
								buttons: Ext.MessageBox.YESNO,
								fn: function(answer) {
									if (answer == "no")
										return;
									OMV.SessionMgr.logout();
								},
								scope: this,
								icon: Ext.MessageBox.QUESTION
							});
						}
					},{
						xtype: "tbseparator"
					},{
						xtype: "splitbutton",
						icon: "images/shutdown.png",
						handler: function() {
							this.showMenu();
						},
						menu: new Ext.menu.Menu({
							items: [{
								text: "Reboot",
								value: "reboot",
								icon: "images/reboot.png"
							},{
								text: "Shutdown",
								value: "shutdown",
								icon: "images/shutdown.png"
							}],
							listeners: {
								itemclick: function(item, e) {
									OMV.MessageBox.show({
										title: "Confirmation",
										msg: "Do you really want to " + item.value + " the system?",
										buttons: Ext.Msg.YESNO,
										fn: function(answer) {
											if (answer == "no")
												return;
											switch (item.value) {
											case "reboot":
												OMV.Ajax.request(function(id, response, error) {
													  if (error !== null) {
														  OMV.MessageBox.error(null, error);
													  } else {
														  OMV.MessageBox.show({
															  title: "Information",
															  icon: Ext.Msg.INFO,
															  msg: "The system will reboot now. This may take some time ...",
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
						}),
						text: "Shutdown"
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
