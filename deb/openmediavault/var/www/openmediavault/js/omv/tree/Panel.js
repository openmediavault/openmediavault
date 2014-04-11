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

/**
 * @class OMV.tree.Panel
 * @derived Ext.tree.Panel
 * @param singleClickExpand TRUE for single click expand on nodes.
 *   Defaults to FALSE.
 */
Ext.define("OMV.tree.Panel", {
	extend: "Ext.tree.Panel",

	singleClickExpand: false,

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		if(true === me.singleClickExpand) {
			// Expand nodes with a single click.
			me.on("itemclick", function(view, record) {
				if(record.isLeaf()) // Skip leafs.
					return;
				record.isExpanded() ? record.collapse() : record.expand();
			}, me);
		}
	}
});
