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

/**
 * @ingroup webgui
 * @class OMV.workspace.node.Node
 */
Ext.define("OMV.workspace.node.Node", {
	isNode: true,
	$configPrefixed: false,

	config: {
		id: null,
		path: null,
		className: null,
		text: "undefined",
		position: 0,
		icon16: null,
		icon32: null,
		iconSvg: null,
		iconCls: null,
		childNodes: null,
		parentNode: null,
		leaf: false,
		uri: null
	},

	statics: {
		/**
		 * Creates a node from config object.
		 * @return The created node.
		 * @static
		 */
		create: function(config) {
			if (!config.isNode)
				node = Ext.create("OMV.workspace.node.Node", config);
			return node;
		},

		/**
		 * Build the node URI.
		 * @param array The parts of the URI, e.g. [ "/services", "ftp" ].
		 * @param separator The separator to use. Defaults to '/'.
		 * @return An array containg the parts of the given path.
		 */
		buildUri: function(array, separator) {
			separator = separator || "/";
			var parts = [];
			Ext.Array.each(array, function(item) {
				Ext.Array.push(parts, this.explodeUri(item));
			}, this);
			return separator + parts.join(separator);
		},

		/**
		 * Explode the node URI in seperate parts.
		 * @param uri The URI to process.
		 * @param separator The separator to use. Defaults to '/'.
		 * @return An array containg the parts of the given path.
		 */
		explodeUri: function(uri, separator) {
			separator = separator || "/";
			var parts = uri.split(separator);
			parts = Ext.Array.filter(parts, function(part, index, array) {
				return !Ext.isEmpty(part);
			});
			return parts;
		},

		/**
		 * Compares the 2 URIs using strict equality.
		 * @param uri1 The first URI.
		 * @param uri2 The second URI.
		 * @param separator The separator to use. Defaults to '/'.
		 * @return TRUE if the URIs are equal.
		 */
		compareUri: function(uri1, uri2, separator) {
			separator = separator || "/";
			return Ext.Array.equals(this.explodeUri(uri1, separator),
			  this.explodeUri(uri2, separator));
		}
	},

	constructor: function(config) {
		var me = this;
		// Append default values.
		config = Ext.apply({
			internalId: Ext.id(),
			childNodes: new Ext.util.MixedCollection()
		}, config);
		me.initConfig(config);
		// Finally build/generate the 'uri' config attribute.
		me.setConfig("uri", me.getUri());
	},

	appendChild: function(child) {
		var me = this;
		if (!child.isNode)
			child = me.create(child);
		child.setParentNode(me);
		return me.getChildNodes().add(child.getId(), child);
	},

	getChild: function(key) {
		return this.getChildNodes().get(key);
	},

	getChildAt: function(index) {
		return this.getChildNodes().getAt(index);
	},

	getRange: function(start, end) {
		return this.getChildNodes().getRange(start, end);
	},

	containsChild: function(key) {
		return this.getChildNodes().containsKey(key);
	},

	hasChildNodes: function() {
		var me = this;
		return !me.isLeaf() && (me.getChildNodes().getCount() > 0);
	},

	getChildCount: function() {
		return this.getChildNodes().getCount();
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
		return !this.getParentNode();
	},

	isLeaf: function() {
		return (true === this.getLeaf());
	},

	eachChild: function(fn, scope, origArgs) {
		var me = this;
		var childNodes = me.getChildNodes();
		var length = childNodes.getCount();
		var i = 0;
		for (; i < length; i++) {
			var child = me.getChildAt(i);
			var args = origArgs ? origArgs.concat(child) : [child];
//			if (fn.apply(scope || me, args || [me]) === false)
			if (fn.apply(scope || me, args) === false)
				break;
		}
	},

	cascadeBy: function(fn, scope, origArgs) {
		var me = this;
		var args = origArgs ? origArgs.concat(me) : [me];
//		if (fn.apply(scope || me, args) !== false) {
		if (fn.apply(scope || me, args || [me]) !== false) {
			var childNodes = me.getChildNodes();
			var length = childNodes.getCount();
			var i = 0;
			for (; i < length; i++) {
				var child = me.getChildAt(i);
				child.cascadeBy(fn, scope, origArgs);
			}
		}
	},

	bubble: function(fn, scope, origArgs) {
		var me = this;
		var p = me;
		while (p) {
			var args = origArgs ? origArgs.concat(p) : [p];
//			if (fn.apply(scope || p, args || [p]) === false)
			if (fn.apply(scope || p, args) === false)
				break;
			p = p.getParentNode();
		}
	},

	sort: function(sorters, direction, where, doSort) {
		// Set the default sorters. Note, use the internal property names,
		// otherwise the Ext.util.Sorter::sortFn() method may not access
		// the property values.
		if (!Ext.isDefined(sorters)) {
			sorters = [{
				property : "position",
				direction: "ASC"
			},{
				property : "text",
				direction: "DESC"
			}];
        }
		return this.getChildNodes().sort(sorters, direction, where, doSort);
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
		var childNodes = me.getChildNodes();
		var length = childNodes.getCount();
		var i = 0;
		for (; i < length; i++) {
			var node = me.getChildAt(i);
			if (node.getConfig(attribute) == value) {
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
	 * SVG icon will be returned instead if this is set.
	 * @return The icon path.
	 */
	getProperIcon16: function() {
		var me = this;
		if (me.hasIcon("svg"))
			return me.getIconSvg();
		return me.getIcon16();
	},

	/**
	 * Get the path of the 32x32 icon. If the device supports SVG, then the
	 * SVG icon will be returned instead if this is set.
	 * @return The icon path.
	 */
	getProperIcon32: function() {
		var me = this;
		if (me.hasIcon("svg"))
			return me.getIconSvg();
		return me.getIcon32();
	},

	/**
	 * Check if the given icon type is defined. For SVG it will be also
	 * checked whether the device supports this.
	 * @return TRUE if the requested icon is defined.
	 */
	hasIcon: function(type) {
		var me = this;
		var result = false;
		switch (type) {
		case "svg":
			result = !Ext.isEmpty(me.getIconSvg()) && Ext.supports.Svg;
			break;
		case "raster16":
			result = !Ext.isEmpty(me.getIcon16());
			break;
		case "raster32":
			result = !Ext.isEmpty(me.getIcon32());
			break;
		case "raster":
			result = !Ext.isEmpty(me.getIcon16()) || !Ext.isEmpty(
				me.getIcon32());
			break;
		case "svg|raster16":
			result = (!Ext.isEmpty(me.getIconSvg()) && Ext.supports.Svg) ||
				!Ext.isEmpty(me.getIcon16());
			break;
		case "svg|raster32":
			result = (!Ext.isEmpty(me.getIconSvg()) && Ext.supports.Svg) ||
				!Ext.isEmpty(me.getIcon32());
			break;
		default:
			result = !Ext.isEmpty(me.getIcon16()) || !Ext.isEmpty(
				me.getIcon32()) || (!Ext.isEmpty(me.getIconSvg()) &&
				Ext.supports.Svg);
			break;
		}
		return result;
	},

	/**
	 * Get the URI of the node based on its path and id.
	 * @param separator The separator to use. Defaults to '/'.
	 * @return The URI of this node.
	 */
	getUri: function(separator) {
		separator = separator || "/";
		var me = this;
		return me.self.buildUri([ me.getPath(), me.getId() ], separator);
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
			for (var attr in node.config) {
				if (Ext.isPrimitive(node[attr]))
					elem[attr] = node.getConfig(attr);
			}
			elem.childNodes = childNodes;
			return elem;
		}
		var array = fn(me);
		return array;
	}
});
