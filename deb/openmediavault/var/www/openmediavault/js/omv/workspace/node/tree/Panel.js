/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
// require("js/omv/tree/Panel.js")
// require("js/omv/workspace/node/tree/Model.js")

/**
 * @ingroup webgui
 * @class OMV.workspace.node.tree.Panel
 * @derived OMV.tree.Panel
 */
Ext.define("OMV.workspace.node.tree.Panel", {
	extend: "OMV.tree.Panel",
	alias: "widget.workspace.node.tree",
	requires: [
		"OMV.workspace.node.tree.Model"
	],
	uses: [
		"OMV.WorkspaceManager"
	],

	cls: Ext.baseCSSPrefix + "workspace-node-tree",
	stateful: true,
	stateId: "ee299152-4534-11e3-bbea-0002b3a176b4",
	stateEvents: [ "afteritemcollapse", "afteritemexpand",
	  "collapse", "expand" ],
	singleClickExpand: false,
	rootVisible: false,

	constructor: function(config) {
		var me = this;
		// Get the workspace root node.
		var rootNode = OMV.WorkspaceManager.getRootNode();
		// Create the tree store.
		config = Ext.apply({
			store: Ext.create("Ext.data.TreeStore", {
				model: "OMV.workspace.node.tree.Model",
				root: {
					id: Ext.id(),
					expanded: true,
					node: rootNode,
					text: rootNode.getText(),
					children: []
				},
				sorters: [{
					sorterFn: function(a, b) {
						var getCmpData = function(o) {
							var node = o.get("node");
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
						  a.position < b.position ? -1 : a.text > b.text ? 1 :
						  a.text < b.text ? -1 : 0;
					}
				}]
			})
		}, config || {});
		me.callParent([ config ]);
		// Automatically deselect the tree node after it has been selected.
		me.on("select", function(model, record, eOpts) {
			model.deselect(record);
		});
	},

	initComponent: function() {
		var me = this;
		var root = me.getRootNode();
		root.get("node").eachChild(function(node) {
			var treeNode = {
				id: Ext.id(),
				text: node.getText(),
				iconCls: Ext.baseCSSPrefix + "tree-icon-16x16",
				node: node,
				leaf: node.isLeaf(),
				children: []
			};
			if(node.hasChildNodes()) {
				Ext.apply(treeNode, {
					cls: "folder",
					expanded: true
				});
			}
			if(node.hasIcon("raster16")) {
				Ext.apply(treeNode, {
					icon: node.getIcon16()
				});
			}
			node.eachChild(function(childNode) {
				var treeChildNode = {
					id: Ext.id(),
					text: childNode.getText(),
					leaf: true,
					node: childNode,
					icon: childNode.getIcon16(),
					iconCls: Ext.baseCSSPrefix + "tree-icon-16x16"
				};
				treeNode.children.push(treeChildNode);
			});
			root.appendChild(treeNode);
		});
		me.callParent(arguments);
	},

	afterRender: function() {
		var me = this;
		me.callParent(arguments);
		// The list of the nodes to select after login is ordered by
		// preference.
		var uri = [
//			"/diagnostic/dashboard",
			"/diagnostic/system",
			"/info/about"
		];
		// Find the workspace node to automatically select after login.
		var nodeToSelect = null;
		me.getRootNode().get("node").cascadeBy(function(node) {
			var index = Ext.Array.indexOf(uri, node.getUri());
			if (0 <= index) {
				if (0 == index) {
					// The prefered workspace node to use.
					nodeToSelect = node;
				} else {
					// Continue search. This workspace node is only a fallback
					// if the prefered one is not found.
					if (Ext.isEmpty(nodeToSelect))
						nodeToSelect = node;
				}
			}
		});
		// Any workspace node found that can be selected?
		if (!Ext.isEmpty(nodeToSelect) && nodeToSelect.isNode) {
			// Select the found node delayed to ensure the components
			// are already rendered (especially the toolbar displaying
			// the path of the selected node). Maybe not the best
			// solution but it works at the moment.
			Ext.Function.defer(function() {
				me.selectPathByNode(nodeToSelect);
			}, 500, me);
		}
	},

	/**
	 * Gets the current state of the tree panel. It contains an array of
	 * expanded nodes.
	 * @return The current state of the object.
	 */
	getState: function() {
		var me = this;
		var state = me.callParent(arguments);
		var nodeURI = [];
		me.getRootNode().cascadeBy(function(treeNode) {
			var node = treeNode.get("node");
			if(!Ext.isObject(node) || !node.isNode)
				return;
			if(node.isLeaf() || !node.hasChildNodes())
				return;
			if(!treeNode.isExpanded())
				return;
			var uri = node.getUri();
			nodeURI.push(uri);
		});
		return Ext.apply(state, {
			expandedNodes: nodeURI
		});
	},

	/**
	 * Applies the state to the tree panel.
	 */
	applyState: function(state) {
		var me = this;
		me.callParent(arguments);
		var nodeURI = Ext.apply([], state.expandedNodes);
		me.getRootNode().cascadeBy(function(treeNode) {
			var node = treeNode.get("node");
			if(!Ext.isObject(node) || !node.isNode)
				return;
			if(node.isLeaf() || !node.hasChildNodes())
				return;
			var uri = node.getUri();
			if(!Ext.Array.contains(nodeURI, uri))
				treeNode.collapse();
		});
	},

	/**
	 * Expand the tree to the path of a particular workspace node,
	 * then select it.
	 * @param node The workspace node to select.
	 */
	selectPathByNode: function(node) {
		var me = this;
		if (!Ext.isObject(node) || !node.isNode)
			return;
		var treeNodeToSelect = null;
		me.getRootNode().cascadeBy(function(treeNode) {
			var node2 = treeNode.get("node");
			if (!Ext.isObject(node2) || !node2.isNode)
				return;
			if (node.getUri() == node2.getUri())
				treeNodeToSelect = treeNode;
		});
		// Any tree node found that can be selected?
		if (!Ext.isEmpty(treeNodeToSelect) && treeNodeToSelect.isModel) {
			var path = treeNodeToSelect.getPath();
			me.selectPath(path);
		}
	}
});
