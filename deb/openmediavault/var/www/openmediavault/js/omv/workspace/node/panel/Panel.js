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
// require("js/omv/workspace/panel/Panel.js")
// require("js/omv/workspace/node/Model.js")

/**
 * Display all children of the given workspace node with their icon
 * in a data view.
 * @class OMV.workspace.node.panel.Panel
 * @derived OMV.workspace.panel.Panel
 * @param root The root workspace node to be displayed.
 */
Ext.define("OMV.workspace.node.panel.Panel", {
	extend: "OMV.workspace.panel.Panel",
	alias: "widget.workspacenodepanel",
	requires: [
		"Ext.data.Store",
		"Ext.view.View",
		"Ext.XTemplate",
		"OMV.WorkspaceManager",
		"OMV.workspace.node.Model",
		"OMV.workspace.node.Node"
	],

	cls: Ext.baseCSSPrefix + "workspace-node-panel",

	constructor: function(config) {
		var me = this;
		config = Ext.apply({
			root: OMV.WorkspaceManager.getRootNode()
		}, config || {});
		me.callParent([ config ]);
		me.addEvents(
			/**
			 * Fires when a node has been selected.
			 * @param this The panel object.
			 * @param node The selected node object.
			 */
			"select"
		);
	},

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			items: Ext.create("Ext.view.View", {
				multiSelect: false,
				trackOver: true,
				overItemCls: Ext.baseCSSPrefix + "item-over",
				itemSelector: "div.thumb-wrap",
				store: Ext.create("Ext.data.Store", {
					model: "OMV.workspace.node.Model",
					data: me.getRootNode().getRange(),
					sorters: [{
						sorterFn: function(a, b) {
							var getPosition = function(o) {
								var node = Ext.create("OMV.workspace.node.Node",
								  o.getData());
								return node.getPosition();
							};
							return getPosition(a) < getPosition(b) ? -1 : 1;
						}
					}]
				}),
				tpl: Ext.create("Ext.XTemplate",
					'<div class="',Ext.baseCSSPrefix,'workspace-node-view">',
						'<tpl for=".">',
							'<div class="thumb-wrap" id="{id:stripTags}">',
								'<div class="thumb"><img src="{[this.renderIcon(values)]}" title="{text:htmlEncode}"></div>',
								'<span>{text:htmlEncode}</span>',
							'</div>',
						'</tpl>',
					'</div>',
					{
						renderIcon: function(values) {
							var node = Ext.create("OMV.workspace.node.Node",
							  values);
							return node.getIcon32();
						}
					}
				),
				listeners: {
					scope: me,
					select: function(view, record, eOpts) {
						var node = this.getRootNode().getChild(
						  record.get("id"));
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
