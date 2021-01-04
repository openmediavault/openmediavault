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
// require("js/omv/workspace/panel/Panel.js")
// require("js/omv/workspace/node/Model.js")
// require("js/omv/workspace/node/Node.js")

/**
 * Display all workspace categories and their child nodes in an data view.
 * @class OMV.workspace.node.panel.Overview
 * @derived OMV.workspace.panel.Panel
 * @param root The root workspace node to be displayed.
 */
Ext.define("OMV.workspace.node.panel.Overview", {
	extend: "OMV.workspace.panel.Panel",
	alias: "widget.workspace.node.panel.overview",
	requires: [
		"Ext.data.Store",
		"Ext.view.View",
		"Ext.XTemplate",
		"OMV.WorkspaceManager",
		"OMV.workspace.node.Model",
		"OMV.workspace.node.Node"
	],

	cls: Ext.baseCSSPrefix + "workspace-node-view",

	constructor: function(config) {
		var me = this;
		config = Ext.apply({
			root: OMV.WorkspaceManager.getRootNode()
		}, config || {});
		me.callParent([ config ]);
		/**
		 * @event select
		 * Fires when a node has been selected.
		 * @param this The panel object.
		 * @param node The selected node object.
		 */
	},

	initComponent: function() {
		var me = this;
		var store = Ext.create("Ext.data.Store", {
			model: "OMV.workspace.node.Model",
			sorters: [{
				sorterFn: function(a, b) {
					var getCmpData = function(o) {
						var node = o.getData();
						return {
							position: node.getPosition(),
							text: node.getText().toLowerCase()
						};
					};
					// Get data to compare.
					a = getCmpData(a);
					b = getCmpData(b);
					// Sort by position and text.
					return a.position > b.position ? 1 :
					  a.position < b.position ? -1 :
					  a.text > b.text ? 1 : a.text < b.text ? -1 : 0;
				}
			}]
		});
		me.getRootNode().eachChild(function(node) {
			var model = store.add(node);
			model[0].set("header", true);
			node.eachChild(function(subNode) {
				model = store.add(subNode);
				model[0].set("header", false);
			});
		});
		Ext.apply(me, {
			items: Ext.create("Ext.view.View", {
				multiSelect: false,
				trackOver: true,
				overItemCls: Ext.baseCSSPrefix + "item-over",
				itemSelector: "div.thumb-wrap",
				store: store,
				collectNodes: function() {
					this.callPrototype("collectNodes", arguments);
					// We need to take care that the data store and composite
					// elements list are ordered in the same way.
					var records = this.getViewRange();
					var elements = this.all.elements;
					var newElements = [];
					Ext.Array.each(records, function(record, index) {
						var newElement = Ext.Array.findBy(elements,
							function(element) {
								return element.id == record.get("internalId");
							});
						Ext.Array.insert(newElements, index, [newElement]);
					});
					this.all.fill(newElements);
				},
				tpl: Ext.create("Ext.XTemplate",
					'<div class="',Ext.baseCSSPrefix,'workspace-node-view-categories">',
						'<tpl for=".">',
							'<tpl if="values.header == true">',
								'<div class="',Ext.baseCSSPrefix,'workspace-node-view-category">',
									'<div class="',Ext.baseCSSPrefix,'workspace-node-view-category-header">',
										'<div class="thumb-wrap" id="{internalId}">',
											'{[this.renderCategoryHeaderText(values)]}',
										'</div>',
									'</div>',
									'<div class="',Ext.baseCSSPrefix,'workspace-node-view-category-items">',
										'{[this.renderCategory(values, parent)]}',
									'</div>',
								'</div>',
								'<div class="x-clear"></div>',
							'</tpl>',
						'</tpl>',
					'</div>',
					{
						renderCategoryHeaderText: function(node) {
							return Ext.String.htmlEncode(node.getText());
						},
						renderCategory: function(categoryNode, models) {
							var tpl = Ext.create("Ext.XTemplate",
								'<tpl for=".">',
									'<tpl if="this.isRenderNode(values)">',
										'<div class="thumb-wrap" id="{internalId}">',
											'<div class="thumb">{[this.renderIcon(values)]}</div>',
											'<span>{[this.renderText(values)]}</span>',
										'</div>',
									'</tpl>',
								'</tpl>',
								{
									isRenderNode: function(node) {
										return OMV.workspace.node.Node.
										  compareUri(node.getPath(),
										  categoryNode.getUri());
									},
									renderText: function(node) {
										return Ext.String.htmlEncode(
										  node.getText());
									},
									renderIcon: function(node) {
										var html = Ext.String.format(
											"<div class='thumb-icon {0}'></div>",
											node.getIconCls());
										if (Ext.isEmpty(node.iconCls) && node.hasIcon(
												"svg|raster32")) {
											html = Ext.String.format(
												"<img class='thumb-icon' src='{0}'>",
												node.getProperIcon32());
										}
										return html;
									}
								});
							return tpl.apply(models);
						}
					}
				),
				listeners: {
					scope: me,
					select: function(view, record, eOpts) {
						var node = record.getData();
						this.fireEvent("select", this, node);
					}
				}
			})
		});
		me.callParent(arguments);
	},

	getRootNode: function() {
		var me = this;
		return me.root;
	}
});
