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
// require("js/omv/workspace/dashboard/Widget.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/util/Format.js")

/**
 * @class OMV.module.admin.dashboard.widget.FileSystemStatus
 * @derived OMV.workspace.dashboard.Widget
 */
Ext.define("OMV.module.admin.dashboard.widget.FileSystemStatus", {
	extend: "OMV.workspace.dashboard.Widget",
	alias: "omv.widget.dashboard.filesystemstatus",
	requires: [
		"OMV.workspace.grid.Panel",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format"
	],

	title: "File system status",
	icon: "images/filesystem.svg",
	width: 300,
	height: 150,
	refreshInterval: 60000,

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			items: [ me.gp = Ext.create("OMV.workspace.grid.Panel", {
				disableLoadMaskOnLoad: true,
				hideTopToolbar: true,
				hidePagingToolbar: true,
				disableSelection: true,
				stateful: true,
				stateId: "778ea266-eaf3-11e3-8211-0002b3a176b4",
				columns: [{
					xtype: "emptycolumn",
					text: _("Device/Label"),
					sortable: true,
					dataIndex: "devicefile",
					stateId: "devicelabel",
					flex: 1,
					renderer: function(value, metaData, record) {
						if (!Ext.isEmpty(record.get("label")))
							return record.get("label");
						return value;
					}
				},{
					xtype: "emptycolumn",
					text: _("Device"),
					sortable: true,
					dataIndex: "devicefile",
					stateId: "devicefile",
					hidden: true,
					flex: 1
				},{
					text: _("Label"),
					sortable: true,
					dataIndex: "label",
					stateId: "label",
					hidden: true,
					flex: 1
				},{
					text: _("Used"),
					sortable: true,
					dataIndex: "used",
					stateId: "used",
					align: "center",
					flex: 2,
					renderer: function(value, metaData, record) {
						var percentage = parseInt(record.get("percentage"));
						if (-1 == percentage)
							return _("n/a");
						var renderer = OMV.util.Format.progressBarRenderer(
						  percentage / 100, value);
						return renderer.apply(this, arguments);
					}
				}],
				viewConfig: {
					trackOver: false
				},
				store: Ext.create("OMV.data.Store", {
					autoLoad: true,
					model: OMV.data.Model.createImplicit({
						idProperty: "devicefile",
						fields: [
							{ name: "devicefile", type: "string" },
							{ name: "label", type: "string" },
							{ name: "used", type: "string" },
							{ name: "percentage", type: "string" }
						]
					}),
					proxy: {
						type: "rpc",
						appendSortParams: false,
						rpcData: {
							service: "FileSystemMgmt",
							method: "enumerateFilesystems"
						}
					},
					sorters: [{
						direction: "ASC",
						property: "devicefile"
					}]
				})
			}) ]
		});
		me.callParent(arguments);
	},

	doRefresh: function() {
		var me = this;
		me.gp.doReload();
	}
});
