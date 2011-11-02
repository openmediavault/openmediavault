/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
// require("js/omv/NavigationPanel.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.System.PowerMgmt");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "powermanagement", {
	text: "Power Management",
	icon: "images/battery.png",
	position: 50
});

/**
 * @class OMV.Module.System.PowerMgmt.SettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.System.PowerMgmt.SettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "PowerMgmt"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.PowerMgmt.SettingsPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.System.PowerMgmt.SettingsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "General Settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "cpufreq",
				fieldLabel: "Monitoring",
				checked: true,
				inputValue: 1,
				boxLabel: "Specifies whether to monitor the system status and select the most appropriate CPU level."
			},{
				xtype: "checkbox",
				name: "powerbtn",
				fieldLabel: "Power button",
				checked: false,
				inputValue: 1,
				boxLabel: "Shutdown the system when pressing the power button."
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "powermanagement", {
	cls: OMV.Module.System.PowerMgmt.SettingsPanel,
	title: "Settings",
	position: 10
});

/**
 * @class OMV.Module.System.PowerMgmt.ScheduleGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.System.PowerMgmt.ScheduleGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "7db7131c-a7ec-4048-88de-606fd587af8e",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Type",
				sortable: true,
				dataIndex: "type",
				id: "type",
				renderer: OMV.util.Format.arrayRenderer([
					[ "reboot","Reboot" ],
					[ "shutdown","Shutdown" ]
				])
			},{
				header: "Minute",
				sortable: true,
				dataIndex: "minute",
				id: "minute"
			},{
				header: "Hour",
				sortable: true,
				dataIndex: "hour",
				id: "hour",
				renderer: OMV.util.Format.arrayRenderer(Date.mapHour)
			},{
				header: "Day of month",
				sortable: true,
				dataIndex: "dayofmonth",
				id: "dayofmonth",
				renderer: OMV.util.Format.arrayRenderer(Date.mapDayOfMonth)
			},{
				header: "Month",
				sortable: true,
				dataIndex: "month",
				id: "month",
				renderer: OMV.util.Format.arrayRenderer(Date.mapMonth)
			},{
				header: "Day of week",
				sortable: true,
				dataIndex: "dayofweek",
				id: "dayofweek",
				renderer: OMV.util.Format.arrayRenderer(Date.mapDayOfWeek)
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.PowerMgmt.ScheduleGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.PowerMgmt.ScheduleGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("Cron", "getListByType",
			  [ [ "reboot","shutdown" ] ]),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "type" },
					{ name: "minute" },
					{ name: "hour" },
					{ name: "dayofmonth" },
					{ name: "month" },
					{ name: "dayofweek" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.System.PowerMgmt.ScheduleGridPanel.superclass.
		  initComponent.apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.System.PowerMgmt.CronJobPropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.System.PowerMgmt.CronJobPropertyDialog({
			uuid: record.get("uuid"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "Cron",
		  "delete", [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "powermanagement", {
	cls: OMV.Module.System.PowerMgmt.ScheduleGridPanel,
	title: "Shutdown/Reboot schedule",
	position: 20
});

/**
 * @class OMV.Module.System.PowerMgmt.CronJobPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.System.PowerMgmt.CronJobPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Cron",
		rpcGetMethod: "get",
		rpcSetMethod: "setRebootShutdown",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " cron job",
		height: 310
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.PowerMgmt.CronJobPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.System.PowerMgmt.CronJobPropertyDialog,
  OMV.CfgObjectDialog, {
	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "type",
			hiddenName: "type",
			fieldLabel: "Type",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "shutdown","Shutdown" ],
					[ "reboot","Reboot" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "reboot"
		},{
			xtype: "combo",
			name: "minute",
			hiddenName: "minute",
			fieldLabel: "Minute",
			mode: "local",
			store: Array.range(0, 59, 1, true).insert(0, "*"),
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: new Date().format("i")
		},{
			xtype: "combo",
			name: "hour",
			hiddenName: "hour",
			fieldLabel: "Hour",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapHour
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: new Date().format("H")
		},{
			xtype: "combo",
			name: "dayofmonth",
			hiddenName: "dayofmonth",
			fieldLabel: "Day of month",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value", "text" ],
				data: Date.mapDayOfMonth
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
			hiddenName: "month",
			fieldLabel: "Month",
			mode: "local",
			store: new Ext.data.SimpleStore({
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
			hiddenName: "dayofweek",
			fieldLabel: "Day of week",
			mode: "local",
			store: new Ext.data.SimpleStore({
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
			fieldLabel: "Comment",
			allowBlank: true
		}];
	}
});
