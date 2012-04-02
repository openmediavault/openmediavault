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
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Storage");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("storage", "physicaldisks", {
	text: "Physical Disks",
	icon: "images/harddisk.png",
	position: 10
});

/**
 * @class OMV.Module.Storage.PhysicalDiskPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.PhysicalDiskPanel = function(config) {
	var initialConfig = {
		hideAdd: true,
		hideDelete: true,
		hideRefresh: true,
		hidePagingToolbar: false,
		stateId: "5e5cc148-c1e7-11e0-99e1-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Device",
				sortable: true,
				dataIndex: "devicefile",
				id: "device",
				width: 50
			},{
				header: "Model",
				sortable: true,
				dataIndex: "model",
				id: "model"
			},{
				header: "Serial Number",
				sortable: true,
				dataIndex: "serialnumber",
				id: "serialnumber"
			},{
				header: "Vendor",
				sortable: true,
				dataIndex: "vendor",
				id: "vendor",
				width: 30
			},{
				header: "Capacity",
				sortable: true,
				dataIndex: "capacity",
				id: "capacity",
				width: 50
			},{
				header: "Temperature",
				sortable: true,
				dataIndex: "temperature",
				id: "temperature",
				width: 45
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.PhysicalDiskPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.PhysicalDiskPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("DiskMgmt", "getList"),
			reader: new Ext.data.JsonReader({
				idProperty: "devicefile",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "devicefile" },
					{ name: "model" },
					{ name: "vendor" },
					{ name: "serialnumber" },
					{ name: "capacity" },
					{ name: "temperature" },
					{ name: "hdparm" }
    			]
			})
		});
		OMV.Module.Storage.PhysicalDiskPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var hdparm = record.get("hdparm");
		var wnd = new OMV.Module.Storage.HdParmPropertyDialog({
			uuid: Ext.isObject(hdparm) ? hdparm.uuid : OMV.UUID_UNDEFINED,
			devicefile: record.get("devicefile"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "physicaldisks", {
	cls: OMV.Module.Storage.PhysicalDiskPanel
});

/**
 * @class OMV.Module.Storage.HdParmPropertyDialog
 */
OMV.Module.Storage.HdParmPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "DiskMgmt",
		rpcGetMethod: "getHdParm",
		rpcSetMethod: "setHdParm",
		title: "Edit physical disk properties",
		autoHeight: true,
		width: 450
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.HdParmPropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.HdParmPropertyDialog, OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "apm",
			hiddenName: "apm",
			fieldLabel: "Advanced Power Management",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ 255,"Disabled" ],
					[ 1,"1 - Minimum power usage with standby (spindown)" ],
					[ 64,"64 - Intermediate power usage with standby" ],
					[ 127,"127 - Intermediate power usage with standby" ],
					[ 128,"128 - Minimum power usage without standby (no spindown)" ],
					[ 192,"192 - Intermediate power usage without standby" ],
					[ 254,"254 - Maximum performance, maximum power usage" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: 255
		},{
			xtype: "combo",
			name: "aam",
			hiddenName: "aam",
			fieldLabel: "Automatic Acoustic Management",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ 0,"Disabled" ],
					[ 128,"Minimum performance, Minimum acoustic output" ],
					[ 254,"Maximum performance, maximum acoustic output" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: 0
		},{
			xtype: "combo",
			name: "spindowntime",
			hiddenName: "spindowntime",
			fieldLabel: "Spindown time",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ 0,"Disabled" ],
					[ 60,"5 minutes" ],
					[ 120,"10 minutes" ],
					[ 240,"20 minutes" ],
					[ 241,"30 minutes" ],
					[ 242,"60 minutes" ],
					[ 244,"120 minutes" ],
					[ 246,"180 minutes" ],
					[ 248,"240 minutes" ],
					[ 250,"300 minutes" ],
					[ 252,"360 minutes" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: 0
		},{
			xtype: "checkbox",
			name: "smart",
			fieldLabel: "S.M.A.R.T.",
			checked: false,
			inputValue: 1,
			boxLabel: "Activate S.M.A.R.T. monitoring."
		},{
			xtype: "checkbox",
			name: "writecache",
			fieldLabel: "Write cache",
			checked: false,
			inputValue: 1,
			boxLabel: "Enable write-cache.",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This function is effective only if the hard drive " +
			  "supports it."
		}];
	},

	getValues : function() {
		var values = OMV.Module.Storage.HdParmPropertyDialog.superclass.
		  getValues.call(this);
		// Append the devicefile
		values.devicefile = this.devicefile;
		return values;
	}
});
