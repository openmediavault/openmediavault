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

Ext.ns("OMV.form");

/**
 * @class OMV.form.TreeComboBox
 * @derived Ext.form.TriggerField
 * A combobox field that displays a tree when pressing the trigger field.
 * @config treeWidth The width of the dropdown tree. Defaults to the width of
 *   the combobox field.
 * @config treeHeight The height of the dropdown tree. Defaults to 200.
 * @config treeAlign A valid anchor position value. Defaults to 'tl-bl?'.
 * @config lazyInit Do not initialize the tree for this combobox field until
 *   the field is focused. Defaults to TRUE.
 * @config treeLoader The Ext.tree.TreeLoader used by the dropdown tree.
 * @config treeRoot The root node of the dropdown tree.
 * @config treeRootVisible Set to FALSE to hide the root node. Defaults to
 *   TRUE.
 * @config treeConfig Additional config options for the dropdown tree.
 */
OMV.form.TreeComboBox = function(config) {
	var initialConfig = {
		editable: false,
		lazyInit: true,
		treeHeight: 200,
		treeAlign: "tl-bl?",
		treeRootVisible: true
	};
	Ext.apply(initialConfig, config);
	OMV.form.TreeComboBox.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires when the dropdown tree is expanded.
		 */
		"expand",
		/**
		 * Fires when the dropdown tree is collapsed.
		 */
		"collapse",
		/**
		 * Fires when a node in the dropdown tree is expanded.
		 */
		"expandnode",
		/**
		 * Fires when a node in the dropdown tree is collapsed.
		 */
		"collapsenode",
		/**
		 * Fires when a node in the dropdown tree is clicked.
		 */
		"select"
	);
};
Ext.extend(OMV.form.TreeComboBox, Ext.form.TriggerField, {
	onRender : function(ct, position) {
		OMV.form.TreeComboBox.superclass.onRender.call(this, ct,
		  position);
		if (!this.lazyInit) {
			this.initTree();
		}else {
			this.on("focus", this.initTree, this, { single: true });
		}
	},

	postBlur : function() {
		OMV.form.TreeComboBox.superclass.postBlur.call(this);
		this.collapse();
	},

	getTree : function() {
		return this.tree;
	},

	onTriggerClick : function() {
		if (this.isExpanded()) {
			this.collapse();
			this.el.focus();
		} else {
			this.onFocus({});
			this.expand();
			this.el.focus();
		}
	},

	isExpanded : function() {
		return this.tree && this.tree.isVisible();
	},

	expand : function() {
		if (this.isExpanded() || !this.hasFocus) {
			return;
		}
		this.tree.getEl().alignTo(this.el, this.treeAlign);
		this.tree.show();
		this.fireEvent("expand", this);
	},

	collapse : function() {
		if (!this.isExpanded()) {
			return;
		}
		this.tree.hide();
		this.fireEvent("collapse", this);
	},

	initTree : function() {
		if (!this.tree) {
			// Use the width of the combobox field by default.
			if (!this.treeWidth) {
				this.treeWidth = this.getWidth();
			}
			var initialTreeConfig = {
				renderTo: Ext.getBody(),
				loader: this.treeLoader,
				root: this.treeRoot,
				rootVisible: this.treeRootVisible,
				floating: true,
				autoScroll: true,
				width: this.treeWidth,
				height: this.treeHeight,
				hidden: true,
				listeners: {
					click: this.onSelect,
					scope: this
				}
			};
			Ext.apply(initialTreeConfig, this.treeConfig);
			this.tree = new Ext.tree.TreePanel(initialTreeConfig);
			// Relay various events.
            this.relayEvents(this.tree, [
				"expandnode",
				"collapsenode"
			]);
		}
	},

	/**
	 * Callback function which is called when a tree node has been clicked.
	 * By default the text of the selected tree node will be displayed in the
	 * combobox field. Override this function to change this behaviour.
	 */
    onSelect : function(node, e) {
		this.setValue(node.text);
		this.collapse();
		this.fireEvent("select", this, node);
    }
});
Ext.reg('treecombo', OMV.form.TreeComboBox);
