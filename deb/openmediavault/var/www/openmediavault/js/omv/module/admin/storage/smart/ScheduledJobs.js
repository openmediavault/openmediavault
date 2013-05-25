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
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @class OMV.module.admin.storage.smart.schedule.Job
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.storage.smart.schedule.Job", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "Smart",
	rpcGetMethod: "getScheduledTest",
	rpcSetMethod: "setScheduledTest",
	plugins: [{
		ptype: "configobject"
	}],

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormItems: function() {
		return [{
			xtype: "combo",
			name: "devicefile",
			fieldLabel: _("Device"),
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefilebyid",
					fields: [
						{ name: "devicefilebyid", type: "string" },
						{ name: "description", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "Smart",
						method: "enumerateDevices"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefilebyid"
				}]
			}),
			emptyText: _("Select a device ..."),
			valueField: "devicefilebyid",
			displayField: "description",
			plugins: [{
				ptype: "fieldinfo",
				text: _("S.M.A.R.T. monitoring must be activated for the selected device.")
			}]
		},{
			xtype: "combo",
			name: "type",
			fieldLabel: _("Type"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "S", _("Short self-test") ],
					[ "L", _("Long self-test") ],
					[ "C", _("Conveyance self-test") ],
					[ "O", _("Offline immediate test") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "S"
		},{
			xtype: "combo",
			name: "hour",
			fieldLabel: _("Hour"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: Date.mapHour2Digits
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: Ext.Date.format(new Date(), "H")
		},{
			xtype: "combo",
			name: "dayofmonth",
			fieldLabel: _("Day of month"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: Date.mapDayOfMonth2Digits
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "combo",
			name: "month",
			fieldLabel: _("Month"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: Date.mapMonth2Digits
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "combo",
			name: "dayofweek",
			fieldLabel: _("Day of week"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: Date.mapDayOfWeek
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "*"
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		}];
	}
});

/**
 * @class OMV.module.admin.storage.smart.schedule.Jobs
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.smart.schedule.Jobs", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format"
	],
	uses: [
		"OMV.module.admin.storage.smart.schedule.Job"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "ca86feba-53c1-42b4-8eea-5119f0244fb5",
	columns: [{
		text: _("Device"),
		stateId: "device",
		sortable: true,
		renderer: function(value, metaData, record) {
			var tpl = new Ext.XTemplate(
			  '<b>', _("Model"), ':</b> {volumemodel}<br/>',
			  '<b>', _("Device"), ':</b> {volumedevicefile}<br/>',
			  '<b>', _("Capacity"), ':</b> {[OMV.util.Format.binaryUnit(values.volumesize)]}');
			return tpl.apply(record.data);
		}
	},{
		text: _("Type"),
		sortable: true,
		dataIndex: "type",
		stateId: "type",
		renderer: OMV.util.Format.arrayRenderer([
			[ "S", _("Short self-test") ],
			[ "L", _("Long self-test") ],
			[ "C", _("Conveyance self-test") ],
			[ "O", _("Offline immediate test") ]
		])
	},{
		text: _("Hour"),
		sortable: true,
		dataIndex: "hour",
		stateId: "hour"
	},{
		text: _("Day of month"),
		sortable: true,
		dataIndex: "dayofmonth",
		stateId: "dayofmonth",
		renderer: OMV.util.Format.arrayRenderer(Date.mapDayOfMonth2Digits)
	},{
		text: _("Month"),
		sortable: true,
		dataIndex: "month",
		stateId: "month",
		renderer: OMV.util.Format.arrayRenderer(Date.mapMonth2Digits)
	},{
		text: _("Day of week"),
		sortable: true,
		dataIndex: "dayofweek",
		stateId: "dayofweek",
		renderer: OMV.util.Format.arrayRenderer(Date.mapDayOfWeek)
	},{
		text: _("Comment"),
		sortable: true,
		dataIndex: "comment",
		stateId: "comment"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "uuid",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "volumedevicefile", mapping: "volume.devicefile", type: "string" },
						{ name: "volumemodel", mapping: "volume.model", type: "string" },
						{ name: "volumesize", mapping: "volume.size", type: "string" },
						{ name: "type", type: "string" },
						{ name: "hour", type: "string" },
						{ name: "dayofmonth", type: "string" },
						{ name: "month", type: "string" },
						{ name: "dayofweek", type: "string" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "Smart",
						method: "getScheduleList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.storage.smart.schedule.Job", {
			title: _("Add scheduled test"),
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.smart.schedule.Job", {
			title: _("Edit scheduled test"),
			uuid: record.get("uuid"),
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
				service: "Smart",
				method: "deleteScheduledTest",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "scheduledjobs",
	path: "/storage/smart",
	text: _("Scheduled tests"),
	position: 30,
	className: "OMV.module.admin.storage.smart.schedule.Jobs"
});
