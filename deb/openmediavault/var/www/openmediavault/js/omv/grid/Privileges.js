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
// require("js/omv/grid/Panel.js")
// require("js/omv/grid/column/BooleanText.js")

/**
 * @ingroup webgui
 * @class OMV.grid.Privileges
 * @derived OMV.grid.Panel
 * @param uuid The UUID of the shared folder.
 * @param autoLoadData TRUE to call the store's load method automatically
 *   creation. Defaults to TRUE.
 * @param readOnly Set this grid to read-only. Defaults to FALSE.
 * @param hideSystemColumn TRUE to hide the 'System' column. Defaults
 *   to TRUE.
 */
Ext.define("OMV.grid.Privileges", {
	extend: "OMV.grid.Panel",
	alias: [ "widget.privilegesgrid" ],
	requires: [
		"Ext.grid.column.CheckColumn",
		"Ext.grid.feature.Grouping",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.grid.column.BooleanText"
	],

	autoLoadData: true,
	readOnly: false,
	hideSystemColumn: true,

	features: [{
		ftype: "grouping",
		groupHeaderTpl: "{renderedGroupValue}"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: me.autoLoadData,
				groupField: "system",
				model: OMV.data.Model.createImplicit({
					fields: [
						{ name: "type", type: "string" },
						{ name: "name", type: "string" },
						{ name: "perms", type: "int", useNull: true, defaultValue: null },
						{ name: "deny", type: "boolean", defaultValue: false },
						{ name: "readonly", type: "boolean", defaultValue: false },
						{ name: "writeable", type: "boolean", defaultValue: false },
						{ name: "system", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					extraParams: {
						uuid: me.uuid
					},
					rpcData: {
						service: "ShareMgmt",
						method: "getPrivileges"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "name"
				}],
				listeners: {
					scope: me,
					load: function(store, records, successful, eOpts) {
						Ext.Array.each(records, function(record) {
							record.beginEdit();
							switch(record.get("perms")) {
							case 0:
								record.set("deny", true);
								break;
							case 5:
								record.set("readonly", true);
								break;
							case 7:
								record.set("writeable", true);
								break;
							default:
								break;
							}
							record.endEdit();
						});
						store.commitChanges();
					}
				}
			}),
			columns: [{
				text: _("Type"),
				sortable: true,
				dataIndex: "type",
				stateId: "type",
				align: "center",
				width: 60,
				resizable: false,
				renderer: function(value, metaData) {
					switch(value) {
					case "user":
						metaData.tdAttr = "data-qtip='" + _("User") + "'";
						value = "<img border='0' src='images/user.png'>";
						break;
					case "group":
						metaData.tdAttr = "data-qtip='" + _("Group") + "'";
						value = "<img border='0' src='images/group.png'>";
						break;
					}
					return value;
				}
			},{
				text: _("Name"),
				sortable: true,
				dataIndex: "name",
				stateId: "name",
				flex: 2
			},{
				xtype: "checkcolumn",
				text: _("Read/Write"),
				sortable: true,
				groupable: false,
				dataIndex: "writeable",
				stateId: "writeable",
				align: "center",
				flex: 1,
				listeners: {
					scope: me,
					checkchange: me.onCheckChange
				}
			},{
				xtype: "checkcolumn",
				text: _("Read-only"),
				sortable: true,
				groupable: false,
				dataIndex: "readonly",
				stateId: "readonly",
				align: "center",
				flex: 1,
				listeners: {
					scope: me,
					checkchange: me.onCheckChange
				}
			},{
				xtype: "checkcolumn",
				text: _("No access"),
				sortable: true,
				groupable: false,
				dataIndex: "deny",
				stateId: "deny",
				align: "center",
				flex: 1,
				listeners: {
					scope: me,
					checkchange: me.onCheckChange
				}
			},{
				xtype: "booleantextcolumn",
				text: _("System"),
				sortable: true,
				groupable: true,
				width: 60,
				hidden: me.hideSystemColumn,
				dataIndex: "system",
				stateId: "system",
				align: "center",
				flex: 1
			}]
		});
		me.callParent(arguments);
	},

	onCheckChange: function(column, rowIndex, checked, eOpts) {
		var me = this;
		if(me.readOnly)
			return;
		var record = me.store.getAt(rowIndex);
		var fieldNames = [ "readonly", "writeable", "deny" ];
		// Clear all other fields except the current selected one.
		record.beginEdit();
		Ext.Array.each(fieldNames, function(fieldName) {
			if(fieldName === column.dataIndex)
				return;
			record.set(fieldName, false);
		});
		record.endEdit();
	}
});
