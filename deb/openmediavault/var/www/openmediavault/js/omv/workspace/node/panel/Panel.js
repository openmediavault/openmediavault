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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/panel/Panel.js")
// require("js/omv/workspace/node/Model.js")

/**
 * Display all registered workspace nodes in an accordion panel.
 * @class OMV.workspace.node.panel.Panel
 * @derived OMV.workspace.panel.Panel
 */
Ext.define("OMV.workspace.node.panel.Panel", {
	extend: "OMV.workspace.panel.Panel",
	alias: "widget.workspacenodepanel",
	requires: [
		"Ext.view.View",
		"OMV.WorkspaceManager",
		"OMV.workspace.node.Model"
	],
	uses: [
		"Ext.data.Store"
	],

	layout: {
		type: "accordion",
		animate: true,
		multi: true
	},

	constructor: function(config) {
		var me = this;
		config = Ext.apply({
			root: OMV.WorkspaceManager.getRootNode()
		}, config || {});
		me.callParent([ config ]);
	},

	initComponent: function() {
		var me = this;
		me.items = [];
		me.root.eachChild(function(node) {
			var config = {
				title: node.getText(),
				html: new Ext.XTemplate(
					'<tpl for=".">',
						'<div class="thumb-wrap" id="{id:stripTags}" style="float:left; margin:5px">',
							'<div class="thumb" style="text-align:center;"><img width="32" height="32" src="{icon32}" title="{text:htmlEncode}"></div>',
							'<span>{text:htmlEncode}</span>',
						'</div>',
					'</tpl>'
				).apply(node.getRange())
			};
			me.items.push(config);
		});
		me.callParent(arguments);
	}
});
