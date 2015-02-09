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
// require("js/omv/util/Format.js")

/**
 * @class OMV.toolbar.Tip
 * @derived Ext.toolbar.Toolbar
 * @param text The text to display.
 * @param icon The icon to display. Defaults to INFO.
 */
Ext.define("OMV.toolbar.Tip", {
	extend: "Ext.toolbar.Toolbar",
	alias: "widget.tiptoolbar",
	requires: [
		"Ext.toolbar.Item",
		"Ext.toolbar.TextItem",
		"OMV.util.Format"
	],
	statics: {
		INFO: Ext.baseCSSPrefix + "message-box-info",
		WARNING: Ext.baseCSSPrefix + "message-box-warning",
		QUESTION: Ext.baseCSSPrefix + "message-box-question",
	},

	iconHeight: 35,
	iconWidth: 35,

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			cls: Ext.baseCSSPrefix + "toolbar-tip",
			items: [{
				xtype: "tbitem",
				height: me.iconHeight,
				width: me.iconWidth,
				cls: me.icon || OMV.toolbar.Tip.INFO
			},{
				xtype: "tbtext",
				flex: 1,
				text: OMV.util.Format.whitespace(me.text)
			}]
		});
		me.callParent(arguments);
	}
});
