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
// require("js/omv/data/Store.js")
// require("js/omv/workspace/panel/Panel.js")
// require("js/omv/workspace/node/Model.js")

/**
 * Display the child nodes of a workspace category in a data view.
 * @class OMV.workspace.node.panel.Category
 * @derived OMV.workspace.panel.Panel
 * @param root The workspace category node to be displayed.
 */
Ext.define("OMV.workspace.node.panel.Category", {
	extend: "OMV.workspace.panel.Panel",
	alias: "widget.workspace.node.panel.category",
	requires: [
		"Ext.view.View",
		"Ext.XTemplate",
		"OMV.data.Store",
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
		Ext.apply(me, {
			items: Ext.create("Ext.view.View", {
				multiSelect: false,
				trackOver: true,
				overItemCls: Ext.baseCSSPrefix + "item-over",
				itemSelector: "div.thumb-wrap",
				store: Ext.create("OMV.data.Store", {
					model: "OMV.workspace.node.Model",
					data: me.getRootNode().getRange(),
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
				}),
				tpl: Ext.create("Ext.XTemplate",
					'<div class="',Ext.baseCSSPrefix,'workspace-node-view-category">',
						'<div class="',Ext.baseCSSPrefix,'workspace-node-view-category-items">',
							'<tpl for=".">',
								'<div class="thumb-wrap" id="{internalId}">',
									'<div class="thumb">{[this.renderIcon(values)]}</div>',
									'<span>{[this.renderText(values)]}</span>',
								'</div>',
							'</tpl>',
						'</div>',
					'</div>',
					'<div class="x-clear"></div>',
					{
						renderText: function(node) {
							return Ext.String.htmlEncode(node.getText());
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
