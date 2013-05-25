/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
		}
	},

	constructor: function(config) {
		var me = this;
		Ext.apply(me, config || {}, {
			id: Ext.id(),
			childNodes: new Ext.util.MixedCollection()
		});
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
	}
});
