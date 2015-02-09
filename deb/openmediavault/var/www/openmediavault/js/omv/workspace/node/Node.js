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

/**
 * @ingroup webgui
 * @class OMV.workspace.node.Node
 */
Ext.define("OMV.workspace.node.Node", {
	isNode: true,

	config: {
		id: null,
		path: null,
		className: null,
		text: "undefined",
		position: 0,
		icon16: null,
		icon32: null,
		iconSvg: null,
		childNodes: null,
		parentNode: null,
		leaf: false
	},

	statics: {
		/**
		 * Creates a node from config object.
		 * @return The created node.
		 * @static
		 */
		create: function(config) {
			if(!config.isNode) {
				node = Ext.create("OMV.workspace.node.Node", config);
			}
			return node;
		},

		/**
		 * Build the node URI.
		 * @param array The parts of the URI, e.g. [ "/services", "ftp" ].
		 * @return An array containg the parts of the given path.
		 */
		buildUri: function(array) {
			var parts = [];
			var uri = "";
			Ext.Array.each(array, function(item) {
				Ext.Array.push(parts, this.explodeUri(item));
			}, this);
			return Ext.String.format("/{0}", parts.join("/"));
		},

		/**
		 * Explode the node URI in seperate parts.
		 * @param uri The URI to process.
		 * @return An array containg the parts of the given path.
		 */
		explodeUri: function(uri) {
			var parts = uri.split("/");
			parts = Ext.Array.filter(parts, function(part, index, array) {
				return !Ext.isEmpty(part);
			});
			return parts;
		},

		/**
		 * Compares the 2 URIs using strict equality.
		 * @param uri1 The first URI.
		 * @param uri2 The second URI.
		 * @return TRUE if the URIs are equal.
		 */
		compareUri: function(uri1, uri2) {
			return Ext.Array.equals(this.explodeUri(uri1),
			  this.explodeUri(uri2));
		}
	},

	constructor: function(config) {
		var me = this;
		Ext.apply(me, config || {}, {
			id: Ext.id(),
			childNodes: new Ext.util.MixedCollection()
		});
		me.uri = me.getUri();
	},

	appendChild: function(child) {
		var me = this;
		if(!child.isNode) {
			child = me.create(child);
		}
		child.parentNode = me;
		return me.childNodes.add(child.id, child);
	},

	getChild: function(key) {
		return this.childNodes.get(key);
	},

	getChildAt: function(index) {
		return this.childNodes.getAt(index);
	},

	getRange: function(start, end) {
		return this.childNodes.getRange(start, end);
	},

	containsChild: function(key) {
		return this.childNodes.containsKey(key);
	},

	hasChildNodes: function() {
		return !this.isLeaf() && (this.childNodes.getCount() > 0);
	},

	getChildCount: function() {
		return this.childNodes.getCount();
	},

	getDepth: function() {
		var me = this;
		var depth = 0;
		me.bubble(function(node) {
			depth++;
		});
		return depth;
	},

	isRoot: function() {
		return !this.parentNode;
	},

	isLeaf: function() {
		return (this.leaf === true);
	},

	eachChild: function(fn, scope, origArgs) {
		var me = this;
		var childNodes = me.childNodes;
		var length = childNodes.getCount();
		var i = 0;
		for(; i < length; i++) {
			var child = me.getChildAt(i);
			var args = origArgs ? origArgs.concat(child) : [child];
//			if(fn.apply(scope || me, args || [me]) === false) {
			if(fn.apply(scope || me, args) === false) {
				break;
			}
		}
	},

	cascadeBy: function(fn, scope, origArgs) {
		var me = this;
		var args = origArgs ? origArgs.concat(me) : [me];
//		if(fn.apply(scope || me, args) !== false) {
		if(fn.apply(scope || me, args || [me]) !== false) {
			var childNodes = me.childNodes;
			var length = childNodes.getCount();
			var i = 0;
			for(; i < length; i++) {
				var child = me.getChildAt(i);
				child.cascadeBy(fn, scope, origArgs);
			}
		}
	},

	bubble: function(fn, scope, origArgs) {
		var me = this;
		var p = me;
		while(p) {
			var args = origArgs ? origArgs.concat(p) : [p];
//			if(fn.apply(scope || p, args || [p]) === false) {
			if(fn.apply(scope || p, args) === false) {
				break;
			}
			p = p.parentNode;
		}
	},

	sort: function(sorters, direction, where, doSort) {
		return this.childNodes.sort(sorters, direction, where, doSort);
	},

	/**
	 * Finds the first child that has the attribute with the specified value.
	 * @param attribute The name of the attribute.
	 * @param value The value to search for.
	 * @param deep Set to TRUE to search through nodes deeper than the
	 *   immediate children.
	 * @return The found child node or null if none was found.
	 */
	findChild: function(attribute, value, deep) {
		var me = this;
		var childNodes = me.childNodes;
		var length = childNodes.getCount();
		var i = 0;
		for (; i < length; i++) {
			var node = me.getChildAt(i);
			if (node.get(attribute) == value) {
				return node;
			} else if (deep) {
				var child = node.findChild(attribute, value, deep);
				if (child !== null)
					return child;
			}
		}
		return null;
	},

	/**
	 * Get the path of the 16x16 icon. If the device supports SVG, then the
	 *   SVG icon will be returned instead if this is set.
	 * @return The icon path.
	 */
	getIcon16: function() {
		var me = this;
		if(me.hasIcon("svg"))
			return me.iconSvg;
		return me.icon16;
	},

	/**
	 * Get the path of the 32x32 icon. If the device supports SVG, then the
	 *   SVG icon will be returned instead if this is set.
	 * @return The icon path.
	 */
	getIcon32: function() {
		var me = this;
		if(me.hasIcon("svg"))
			return me.iconSvg;
		return me.icon32;
	},

	/**
	 * Check if the given icon type is defined. For SVG it will be also
	 * checked whether the device supports this.
	 * @return TRUE if the requested icon is defined.
	 */
	hasIcon: function(type) {
		var me = this;
		var result = false;
		switch(type) {
		case "svg":
			result = !Ext.isEmpty(me.iconSvg) && Ext.supports.Svg;
			break;
		case "raster16":
			result = !Ext.isEmpty(me.icon16);
			break;
		case "raster32":
			result = !Ext.isEmpty(me.icon32);
			break;
		case "raster":
			result = !Ext.isEmpty(me.icon16) || !Ext.isEmpty(me.icon32);
			break;
		default:
			result = !Ext.isEmpty(me.icon16) || !Ext.isEmpty(me.icon32) ||
			  (!Ext.isEmpty(me.iconSvg) && Ext.supports.Svg);
			break;
		}
		return result;
	},

	/**
	 * Get the URI of the node based on its path and id.
	 * @return The URI of this node.
	 */
	getUri: function() {
		var me = this;
		if (!Ext.isEmpty(me.uri))
			return me.uri;
		return me.uri = me.self.buildUri([ me.path, me.id ]);
	},

	/**
	 * Returns the value of the given attribute.
	 * @param attribute The attribute to fetch the value for.
	 * @return The value.
	 */
	get: function(attribute) {
		var me = this;
        return me[attribute];
    },

    /**
     * Flattens all the nodes into an array.
     * @return The flattened nodes.
     */
    flatten: function() {
		var me = this;
		var fn = function(node) {
			var childNodes = [];
			var length = node.getChildCount();
			var i = 0;
			for (; i < length; i++) {
				var child = node.getChildAt(i);
				Ext.Array.push(childNodes, fn(child));
			}
			// Duplicate primitive node values.
			var elem = {};
			for (attr in node.config) {
				if (Ext.isPrimitive(node[attr]))
					elem[attr] = node[attr];
			}
			elem.childNodes = childNodes;
			return elem;
		}
		var array = fn(me);
		return array;
	}
});
