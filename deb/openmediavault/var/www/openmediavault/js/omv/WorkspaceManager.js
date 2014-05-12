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
// require("js/omv/workspace/node/Node.js")

/**
 * @ingroup webgui
 * @class OMV.WorkspaceManager
 */
Ext.define("OMV.WorkspaceManager", {
	singleton: true,
	requires: [
		"Ext.util.MixedCollection"
	],
	uses: [
		"OMV.workspace.node.Node"
	],

	constructor: function(config) {
		var me = this;
		Ext.apply(me, config || {});
		me.root = me.createNode({
			id: "root",
			path: "/",
			text: _("Overview"),
			leaf: false,
			icon16: "images/home.png",
			iconSvg: "images/home.svg"
		});
	},

	/**
	 * Helper method to create a node configuration.
	 * @return The default node configuration.
	 */
	createNode: function(config) {
		return OMV.workspace.node.Node.create(config);
	},

	/**
	 * Helper method to explode the node path in seperate parts.
	 * @return An array containg the parts of the given path.
	 */
	explodeNodePath: function(path) {
		return OMV.workspace.node.Node.explodeUri(path);
	},

	/**
	 * Helper method to build a node path.
	 * @return A string containing the node path.
	 */
	buildNodePath: function(parts) {
		return OMV.workspace.node.Node.buildUri(parts);
	},

	/**
	 * Get the root node.
	 * @return The root node object.
	 */
	getRootNode: function() {
		var me = this;
		return me.root;
	},

	/**
	 * Register a new node. These config is used to create the navigation tree
	 * and working area. Nodes are sorted by their position and text.
	 * @param config The node configuration. An object which may contain
	 *   the following properties:
	 *   \em id The id of the node. This must be unique.
	 *   \em path The path of the parent node.
	 *   \em text
	 *   \em position The position of the node. Defaults to 100.
	 *   \em icon16
	 *   \em icon32
	 */
	registerNode: function(config) {
		var me = this;
		// Append default values.
		config = Ext.apply({
			position: 100
		}, config);
		// Extract path nodes.
		var parts = me.explodeNodePath(config.path)
		var parent = me.getRootNode();
		// Walk down the node path. Create non-existing nodes during this
		// process. Normally these nodes will be reconfigured later.
		Ext.Array.each(parts, function(id, index, array) {
			if(parent.containsChild(id)) {
				parent = parent.getChild(id);
			} else {
				// If not, create a new node.
				parent = parent.appendChild(me.createNode({
					id: id,
					path: parent.getPath(),
					leaf: false
				}));
			}
		});
		// Finally create/update the given node.
		if(parent.containsChild(config.id)) {
			result = parent.getChild(config.id);
			Ext.apply(result, Ext.apply(config, {
				leaf: false
			}));
		} else {
			result = parent.appendChild(me.createNode(
			  Ext.apply(config, {
				leaf: false
			})));
		}
		parent.sort([{
			property : "position",
			direction: "ASC"
		},{
			property : "text",
			direction: "DESC"
		}]);
		return result;
	},

	/**
	 * Register a new panel for the given node path. Panels are sorted by
	 * their position and text.
	 * @param config The node configuration. An object which may contain
	 *   the following properties:
	 *   \em id The id of the node. This must be unique.
	 *   \em path The path of the parent node.
	 *   \em text
	 *   \em position The position of the panel. Defaults to 100.
	 *   \em icon16
	 *   \em icon32
	 *   \em className
	 */
	registerPanel: function(config) {
		var me = this;
		// Append default values.
		config = Ext.apply({
			position: 100
		}, config);
		// Check if the given class exists.
		if(!Ext.ClassManager.isCreated(config.className)) {
			Ext.Error.raise(Ext.String.format("Failed to register " +
			  "workspace panel. The class '{0}' does not exist.",
			  config.className));
			return;
		}
		// Find the parent node.
		var parent = me.getNodeByPath(config.path);
		if(null == parent) {
			// Extract and rebuild node path.
			var parts = me.explodeNodePath(config.path);
			var id = parts.pop();
			var path = me.buildNodePath(parts)
			// Register parent node, should be re-registered later with
			// correct values.
			parent = me.registerNode({
				id: id,
				path: path
			});
		} else {
			// Check if child node already exits.
			if(parent.containsChild(config.id)) {
				Ext.Error.raise(Ext.String.format("A child node already " +
				  "exists (id={0}, path={1}, text={2}, position{3}).",
				  config.id, config.path, config.text, config.position));
			}
		}
		var node = parent.appendChild(me.createNode(Ext.apply(config, {
			leaf: true
		})));
		parent.sort([{
			property : "position",
			direction: "ASC"
		},{
			property : "text",
			direction: "DESC"
		}]);
		return node;
	},

	/**
	 * Gets a node by its path.
	 * @return The match node or NULL if the node does not exist.
	 */
	getNodeByPath: function(path) {
		var me = this;
		var result = me.getRootNode();
		var parts = me.explodeNodePath(path);
		Ext.Array.each(parts, function(id, index, array) {
			if(!result.containsChild(id)) {
				result = null;
				return false;
			}
			if(index == array.length) {
				result = result.getChild(id);
				return false;
			} else {
				result = result.getChild(id);
			}
		}, me);
		return result;
	}
});
