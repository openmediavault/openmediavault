/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/window/Execute.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.clamav.Job
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.clamav.Job", {
	extend: "OMV.workspace.window.Form",
	uses: [
		"OMV.form.field.SharedFolderComboBox",
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "ClamAV",
	rpcGetMethod: "getJob",
	rpcSetMethod: "setJob",
	plugins: [{
		ptype: "configobject"
	}],
	width: 540,
	height: 375,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormItems: function() {
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: true
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			fieldLabel: _("Shared folder"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The location of the files to scan.")
			}]
		},{
			xtype: "compositefield",
			fieldLabel: _("Minute"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "minute",
				queryMode: "local",
				store: Ext.Array.insert(Ext.Array.range(0, 59, 1, true),
				  0, [ "*" ]),
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: String(new Date().getMinutes()),
				flex: 1
			},{
				xtype: "checkbox",
				name: "everynminute",
				fieldLabel: "",
				checked: false,
				boxLabel: _("Every N minute"),
				width: 140
			}]
		},{
			xtype: "compositefield",
			fieldLabel: _("Hour"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "hour",
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: Date.mapHour
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: String(new Date().getHours()),
				flex: 1
			},{
				xtype: "checkbox",
				name: "everynhour",
				fieldLabel: "",
				checked: false,
				boxLabel: _("Every N hour"),
				width: 140
			}]
		},{
			xtype: "compositefield",
			fieldLabel: _("Day of month"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "dayofmonth",
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: Date.mapDayOfMonth
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "*",
				flex: 1
			},{
				xtype: "checkbox",
				name: "everyndayofmonth",
				fieldLabel: "",
				checked: false,
				boxLabel: _("Every N day of month"),
				width: 140
			}]
		},{
			xtype: "combo",
			name: "month",
			fieldLabel: _("Month"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: Date.mapMonth
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
			xtype: "checkbox",
			name: "onaccess",
			fieldLabel: _("On-access"),
			checked: false,
			hidden: true,
			boxLabel: _("Enable on-access virus scanning"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("Monitor the shared folder and scan new or modified files.")
			}]
		},{
			xtype: "combo",
			name: "virusaction",
			fieldLabel: _("Infected files"),
			queryMode: "local",
			store: [
				[ "nothing", "Perform nothing" ],
				[ "quarantine", "Move to quarantine" ],
				[ "delete", "Delete immediately" ]
			],
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "nothing",
			plugins: [{
				ptype: "fieldinfo",
				text: _("The option, how to handle infected files.")
			}]
		},{
			xtype: "checkbox",
			name: "multiscan",
			fieldLabel: _("Multiscan"),
			boxLabel: _("Scan the directory contents in parallel using available threads."),
			checked: false
		},{
			xtype: "checkbox",
			name: "verbose",
			fieldLabel: _("Verbose"),
			boxLabel: _("Be verbose."),
			checked: false
		},{
			xtype: "checkbox",
			name: "sendemail",
			fieldLabel: _("Send email"),
			checked: false,
			boxLabel: _("Send command output via email"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("An email message with the command output (if any produced) is send to the administrator.")
			}]
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			vtype: "comment"
		}];
	}
});

/**
 * @class OMV.module.admin.service.clamav.Jobs
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.clamav.Jobs", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format",
		"OMV.window.Execute"
	],
	uses: [
		"OMV.module.admin.service.clamav.Job"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "f8a8cf1c-a107-11e1-a5a0-00221568ca88",
	columns: [{
		xtype: "enabledcolumn",
		text: _("Enabled"),
		sortable: true,
		dataIndex: "enable",
		stateId: "enable"
	},{
		xtype: "textcolumn",
		text: _("Shared folder"),
		sortable: true,
		dataIndex: "sharedfoldername",
		stateId: "sharedfoldername"
	},{
		text: _("Minute"),
		sortable: true,
		dataIndex: "minute",
		stateId: "minute",
		renderer: function(value, metaData, record) {
			var everynminute = record.get("everynminute");
			if(everynminute == true) {
				value = "*/" + value;
			}
			return value;
		}
	},{
		text: _("Hour"),
		sortable: true,
		dataIndex: "hour",
		stateId: "hour",
		renderer: function(value, metaData, record) {
			var everynhour = record.get("everynhour");
			var func = OMV.util.Format.arrayRenderer(Date.mapHour);
			value = func(value);
			if(everynhour == true) {
				value = "*/" + value;
			}
			return value;
		}
	},{
		text: _("Day of month"),
		sortable: true,
		dataIndex: "dayofmonth",
		stateId: "dayofmonth",
		renderer: function(value, metaData, record) {
			var everyndayofmonth = record.get("everyndayofmonth");
			var func = OMV.util.Format.arrayRenderer(
			  Date.mapDayOfMonth);
			value = func(value);
			if(everyndayofmonth == true) {
				value = "*/" + value;
			}
			return value;
		}
	},{
		text: _("Month"),
		sortable: true,
		dataIndex: "month",
		stateId: "month",
		renderer: OMV.util.Format.arrayRenderer(Date.mapMonth)
	},{
		text: _("Day of week"),
		sortable: true,
		dataIndex: "dayofweek",
		stateId: "dayofweek",
		renderer: OMV.util.Format.arrayRenderer(Date.mapDayOfWeek)
	},{
		xtype: "textcolumn",
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
						{ name: "enable", type: "boolean" },
						{ name: "sharedfoldername", type: "string" },
						{ name: "minute", type: "string" },
						{ name: "everynminute", type: "boolean" },
						{ name: "hour", type: "string" },
						{ name: "everynhour", type: "boolean" },
						{ name: "dayofmonth", type: "string" },
						{ name: "everyndayofmonth", type: "boolean" },
						{ name: "month", type: "string" },
						{ name: "dayofweek", type: "string" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "ClamAV",
						method: "getJobList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "sharedfoldername"
				}]
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		// Add 'Run' button to top toolbar.
		Ext.Array.insert(items, 2, [{
			id: me.getId() + "-run",
			xtype: "button",
			text: _("Run"),
			iconCls: "x-fa fa-play",
			handler: Ext.Function.bind(me.onRunButton, me, [ me ]),
			scope: me,
			disabled: true,
			selectionConfig: {
				minSelections: 1,
				maxSelections: 1
			}
		}]);
		return items;
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.clamav.Job", {
			title: _("Add scheduled job"),
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
		Ext.create("OMV.module.admin.service.clamav.Job", {
			title: _("Edit scheduled job"),
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
				service: "ClamAV",
				method: "deleteJob",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	},

	onRunButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.window.Execute", {
			title: _("Execute scheduled job"),
			rpcService: "ClamAV",
			rpcMethod: "executeJob",
			rpcParams: {
				uuid: record.get("uuid")
			},
			listeners: {
				scope: me,
				finish: function(wnd, response) {
					wnd.appendValue(_("Done ..."));
				},
				exception: function(wnd, error) {
					OMV.MessageBox.error(null, error);
				}
			}
		}).show();
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "jobs",
	path: "/service/clamav",
	text: _("Scheduled Jobs"),
	position: 30,
	className: "OMV.module.admin.service.clamav.Jobs"
});
