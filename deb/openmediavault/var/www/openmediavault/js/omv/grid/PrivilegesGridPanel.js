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
// require("js/omv/grid/GridPanel.js")

Ext.ns("OMV.form");

/**
 * @class OMV.grid.PrivilegesGridPanel
 * @derived OMV.grid.GridPanel
 */
OMV.grid.PrivilegesGridPanel = function(config) {
	var initialConfig = {
		bodyCssClass: "x-grid3-without-dirty-cell",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Type"),
				sortable: true,
				dataIndex: "type",
				id: "type",
				align: "center",
				width: 50,
				renderer: this.typeRenderer,
				scope: this
			},{
				header: _("Name"),
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: _("Read/Write"),
				dataIndex: "writeable",
				id: "writeable",
				align: "center",
				renderer: this.checkBoxRenderer,
				scope: this
			},{
				header: _("Read-only"),
				dataIndex: "readonly",
				id: "readonly",
				align: "center",
				renderer: this.checkBoxRenderer,
				scope: this
			},{
				header: _("No access"),
				dataIndex: "deny",
				id: "deny",
				align: "center",
				renderer: this.checkBoxRenderer,
				scope: this
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.grid.PrivilegesGridPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.grid.PrivilegesGridPanel, OMV.grid.GridPanel, {
	initComponent : function() {
		OMV.grid.PrivilegesGridPanel.superclass.initComponent.apply(
		  this, arguments);
		// Register event handler
		this.on("cellclick", this.onCellClick, this);
	},

	/**
	 * Handle grid cell clicks. Only process columns with a checkbox.
	 */
	onCellClick : function(grid, rowIndex, columnIndex, e) {
		if (this.readOnly)
			return;
		var record = this.store.getAt(rowIndex);
		var dataIndex = this.getColumnModel().getDataIndex(columnIndex);
		var dataIndices = [ "readonly", "writeable", "deny" ];
		if (-1 !== dataIndices.indexOf(dataIndex)) {
			// Clear all selections
			for (var i = 0; i < dataIndices.length; i++) {
				// Skip current clicked record field, otherwise unselection
				// of cells will not work
				if (dataIndices[i] === dataIndex)
					continue;
				// Set to 'false' per default
				record.set(dataIndices[i], false);
			}
			// Set new selection
			record.set(dataIndex, !record.get(dataIndex));
		}
	},

	/**
	 * Render a user/group icon and a tooltip in the given grid cell.
	 */
	typeRenderer : function(val, cell, record, row, col, store) {
		switch (val) {
		case "user":
			val = "<img border='0' src='images/user.png'>";
			cell.attr = "ext:qtip='" + _("User") + "'";
			break;
		case "group":
			val = "<img border='0' src='images/group.png'>";
			cell.attr = "ext:qtip='" + _("Group") + "'";
			break;
		}
		return val;
	},

	/**
	 * Render a checkbox in the given grid cell.
	 */
	checkBoxRenderer : function(val, cell, record, row, col, store) {
		cell.css += " x-grid3-check-col-td";
		return '<div class="x-grid3-check-col' + ((true === val) ? '-on' : '') +
		  ' x-grid3-cc-' + this.id + '">&#160;</div>';
	}
});
Ext.reg("privilegesgrid", OMV.grid.PrivilegesGridPanel);
