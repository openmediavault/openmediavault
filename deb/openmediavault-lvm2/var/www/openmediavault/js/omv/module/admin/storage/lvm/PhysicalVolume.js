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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.storage.lvm.PhysicalVolume
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.storage.lvm.PhysicalVolume", {
	extend: "OMV.workspace.window.Form",

	rpcService: "LogicalVolumeMgmt",
	rpcSetMethod: "createPhysicalVolume",
	autoLoadData: false,
	hideResetButton: true,
	width: 500,

	getFormItems: function() {
		return [{
			xtype: "combo",
			name: "devicefile",
			fieldLabel: _("Device"),
			emptyText: _("Select an device ..."),
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "size", type: "string" },
						{ name: "description", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getPhysicalVolumeCandidates"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			}),
			displayField: "description",
			valueField: "devicefile",
			allowBlank: false,
			editable: false,
			triggerAction: "all"
		}];
	},

	doSubmit: function() {
		var me = this;
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to create the physical volume?"),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if(answer === "no")
					return;
				me.superclass.doSubmit.call(me);
			},
			scope: me,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.module.admin.storage.lvm.PhysicalVolumes
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.lvm.PhysicalVolumes", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.storage.lvm.PhysicalVolume"
	],

	hideAddButton: false,
	hideEditButton: true,
	hideDeleteButton: false,
	hideRefreshButton: true,
	hidePagingToolbar: false,
	stateful: true,
	stateId: "4f7be743-777a-4547-9277-21a30ce0856b",
	columns: [{
		text: _("Device"),
		sortable: true,
		dataIndex: "devicefile",
		stateId: "devicefile"
	},{
		xtype: "binaryunitcolumn",
		text: _("Available"),
		sortable: true,
		dataIndex: "size",
		stateId: "size"
	},{
		xtype: "binaryunitcolumn",
		text: _("Used"),
		sortable: true,
		dataIndex: "used",
		stateId: "used"
	},{
		xtype: "emptycolumn",
		text: _("Volume group"),
		sortable: true,
		dataIndex: "vgname",
		stateId: "vgName",
		emptyText: "--"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "used", type: "string" },
						{ name: "size", type: "string" },
						{ name: "vgname", type: "string" },
						{ name: "_used", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "LogicalVolumeMgmt",
						method: "getPhysicalVolumesList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.storage.lvm.PhysicalVolume", {
			title: _("Add physical volume"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	doDeletion: function(record) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "LogicalVolumeMgmt",
				method: "deletePhysicalVolume",
				params: {
					devicefile: record.get("devicefile")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "physicalvolumes",
	path: "/storage/lvm",
	text: _("Physical volumes"),
	position: 10,
	className: "OMV.module.admin.storage.lvm.PhysicalVolumes"
});
