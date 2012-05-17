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
// require("js/omv/NavigationPanel.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/UserComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")
// require("js/omv/ExecCmdDialog.js")

Ext.ns("OMV.Module.System");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "cronjobs", {
	text: _("Cron Jobs"),
	icon: "images/cron.png",
	position: 70
});

/**
 * @class OMV.Module.System.CronGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.System.CronGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "a882a76d-6804-4632-b31b-8b48c0ea6dde",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Enabled"),
				sortable: true,
				dataIndex: "enable",
				id: "enable",
				align: "center",
				width: 60,
				renderer: OMV.util.Format.booleanRenderer()
			},{
				header: _("Minute"),
				sortable: true,
				dataIndex: "minute",
				id: "minute",
				renderer: function(val, cell, record, row, col, store) {
					var everynminute = record.get("everynminute");
					if (everynminute == true) {
						val = "*/" + val;
					}
					return val;
				}
			},{
				header: _("Hour"),
				sortable: true,
				dataIndex: "hour",
				id: "hour",
				renderer: function(val, cell, record, row, col, store) {
					var everynhour = record.get("everynhour");
					var func = OMV.util.Format.arrayRenderer(Date.mapHour);
					val = func(val);
					if (everynhour == true) {
						val = "*/" + val;
					}
					return val;
				}
			},{
				header: _("Day of month"),
				sortable: true,
				dataIndex: "dayofmonth",
				id: "dayofmonth",
				renderer: function(val, cell, record, row, col, store) {
					var everyndayofmonth = record.get("everyndayofmonth");
					var func = OMV.util.Format.arrayRenderer(
					  Date.mapDayOfMonth);
					val = func(val);
					if (everyndayofmonth == true) {
						val = "*/" + val;
					}
					return val;
				}
			},{
				header: _("Month"),
				sortable: true,
				dataIndex: "month",
				id: "month",
				renderer: OMV.util.Format.arrayRenderer(Date.mapMonth)
			},{
				header: _("Day of week"),
				sortable: true,
				dataIndex: "dayofweek",
				id: "dayofweek",
				renderer: OMV.util.Format.arrayRenderer(Date.mapDayOfWeek)
			},{
				header: _("User"),
				sortable: true,
				dataIndex: "username",
				id: "username"
			},{
				header: _("Command"),
				sortable: true,
				dataIndex: "command",
				id: "command"
			},{
				header: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.CronGridPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.System.CronGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "Cron",
				"method": "getList",
				"extraParams": { "type": [ "userdefined" ] }
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "enable" },
					{ name: "type" },
					{ name: "minute" },
					{ name: "everynminute" },
					{ name: "hour" },
					{ name: "everynhour" },
					{ name: "dayofmonth" },
					{ name: "everyndayofmonth" },
					{ name: "month" },
					{ name: "dayofweek" },
					{ name: "username" },
					{ name: "command" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.System.CronGridPanel.superclass.initComponent.apply(this,
		  arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.System.CronGridPanel.superclass.initToolbar.apply(
		  this);
		// Add 'Run' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-run",
			xtype: "button",
			text: _("Run"),
			icon: "images/run.png",
			handler: this.cbRunBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.System.CronGridPanel.superclass.cbSelectionChangeHdl.apply(
		  this, arguments);
		// Process additional buttons
		var records = model.getSelections();
		var tbarRunCtrl = this.getTopToolbar().findById(this.getId() + "-run");
		if (records.length <= 0) {
			tbarRunCtrl.disable();
		} else if (records.length == 1) {
			tbarRunCtrl.enable();
		} else {
			tbarRunCtrl.disable();
		}
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.System.CronPropertyDialog({
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
		var wnd = new OMV.Module.System.CronPropertyDialog({
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
		  "delete", { "uuid": record.get("uuid") });
	},

	cbRunBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.ExecCmdDialog({
			title: _("Execute cron job"),
			rpcService: "Cron",
			rpcMethod: "execute",
			rpcArgs: { "uuid": record.get("uuid") },
			listeners: {
				exception: function(wnd, error) {
					OMV.MessageBox.error(null, error);
				},
				scope: this
			}
		});
		wnd.show();
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "cronjobs", {
	cls: OMV.Module.System.CronGridPanel
});

/**
 * @class OMV.Module.System.CronPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.System.CronPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "Cron",
		rpcGetMethod: "get",
		rpcSetMethod: "set",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add cron job") : _("Edit cron job"),
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.CronPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.CronPropertyDialog, OMV.CfgObjectDialog, {
	getFormItems : function() {
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: true,
			inputValue: 1
		},{
			xtype: "compositefield",
			fieldLabel: _("Minute"),
			combineErrors: false,
			items: [{
				xtype: "combo",
				name: "minute",
				hiddenName: "minute",
				mode: "local",
				store: Array.range(0, 59, 1, true).insert(0, "*"),
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: new Date().format("i"),
				flex: 1
			},{
				xtype: "checkbox",
				name: "everynminute",
				fieldLabel: "",
				checked: false,
				inputValue: 1,
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
				hiddenName: "hour",
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
				value: new Date().format("H"),
				flex: 1
			},{
				xtype: "checkbox",
				name: "everynhour",
				fieldLabel: "",
				checked: false,
				inputValue: 1,
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
				hiddenName: "dayofmonth",
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
				value: "*",
				flex: 1
			},{
				xtype: "checkbox",
				name: "everyndayofmonth",
				fieldLabel: "",
				checked: false,
				inputValue: 1,
				boxLabel: _("Every N day of month"),
				width: 140
			}]
		},{
			xtype: "combo",
			name: "month",
			hiddenName: "month",
			fieldLabel: _("Month"),
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
			fieldLabel: _("Day of week"),
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
			xtype: "usercombo",
			name: "username",
			hiddenName: "username",
			fieldLabel: _("User"),
			value: "root"
		},{
			xtype: "textfield",
			name: "command",
			fieldLabel: _("Command"),
			allowBlank: false
		},{
			xtype: "checkbox",
			name: "sendemail",
			fieldLabel: _("Send email"),
			checked: false,
			inputValue: 1,
			boxLabel: _("Send command output via email"),
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("An email message with the command output (if any produced) is send to the user who performs the job.")
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		},{
			xtype: "hidden",
			name: "type",
			value: "userdefined"
		}];
	},

	isValid : function() {
		var valid = OMV.Module.System.CronPropertyDialog.superclass.
		  isValid.apply(this, arguments);
		if (valid) {
			// It is not allowed to select '*' if the everyxxx checkbox
			// is checked.
			[ "minute", "hour", "dayofmonth" ].each(function(fieldName) {
				var field = this.findFormField(fieldName);
				field.clearInvalid(); // combineErrors is false
				if ((field.getValue() === "*") && (this.findFormField(
				  "everyn" + fieldName).checked)) {
					field.markInvalid(_("Ranges of numbers are not allowed"));
					valid = false;
				}
			}, this);
		}
		return valid;
	}
});
