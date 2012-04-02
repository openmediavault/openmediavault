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
// require("js/omv/PluginMgr.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/RrdGraphPanel.js")

Ext.ns("OMV.Module.Diagnostics.SysInfo");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("diagnostics", "sysinfo", {
	text: "System Information",
	icon: "images/info.png",
	position: 100
});

/**
 * @class OMV.Module.Diagnostics.SysInfo.Overview
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Diagnostics.SysInfo.Overview = function(config) {
	var initialConfig = {
		disableLoadMaskOnLoad: true,
		autoReload: true,
		reloadInterval: 5000,
		hideToolbar: true,
		hidePagingToolbar: true,
		hideHeaders: true,
		disableSelection: true,
		trackMouseOver: false,
		stateId: "eda034fd-aa2e-4a7a-9f75-e054d26bcd31",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name",
				width: 25,
				css: "background-color: #FAFAFA;"
			},{
				header: "Value",
				sortable: true,
				dataIndex: "value",
				id: "value",
				renderer: function(val, cell, record, row, col, store) {
					var result = val;
					switch (record.get("type")) {
					case "time":
						var renderer = OMV.util.Format.localeTimeRenderer();
						result = renderer(val);
						break;
					case "progress":
						var id = Ext.id();
						(function(){
							new Ext.ProgressBar({
								renderTo: id,
								value: val.value / 100,
								text: val.text
							});
						}).defer(25)
						result = '<div id="' + id + '"></div>';
						break;
					default:
						// Nothing to do here
						break;
					}
					return result;
				}
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.Overview.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.Overview,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("System", "getInformation"),
			reader: new Ext.data.JsonReader({
				idProperty: "index",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "index" },
					{ name: "type" },
					{ name: "name" },
					{ name: "value" }
				]
			}),
			sortInfo: {
				field: "index",
				direction: "ASC"
			}
		});
		OMV.Module.Diagnostics.SysInfo.Overview.superclass.initComponent.
		  apply(this, arguments);
	}
});
OMV.NavigationPanelMgr.registerPanel("diagnostics", "sysinfo", {
	cls: OMV.Module.Diagnostics.SysInfo.Overview,
	title: "Overview",
	position: 10
});

/**
 * @class OMV.Module.Diagnostics.SysInfo.System
 * @derived Ext.TabPanel
 */
OMV.Module.Diagnostics.SysInfo.System = function(config) {
	var initialConfig = {
		border: false,
		activeTab: 0,
		layoutOnTabChange: true,
		enableTabScroll: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.System.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.System, Ext.TabPanel, {
	initComponent : function() {
		this.items = [];
		var plugins = OMV.PluginMgr.get("sysinfo");
		plugins.each(function(plugin) {
			if (plugin.type === "system") {
				this.items.push(new plugin.cls());
			}
		}, this);
		OMV.Module.Diagnostics.SysInfo.System.superclass.initComponent.
		  apply(this, arguments);
	}
});
OMV.NavigationPanelMgr.registerPanel("diagnostics", "sysinfo", {
	cls: OMV.Module.Diagnostics.SysInfo.System,
	title: "System",
	position: 20
});

/**
 * @class OMV.Module.Diagnostics.SysInfo.Services
 * @derived Ext.TabPanel
 */
OMV.Module.Diagnostics.SysInfo.Services = function(config) {
	var initialConfig = {
		border: false,
		activeTab: 0,
		layoutOnTabChange: true,
		enableTabScroll: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.Services.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.Services, Ext.TabPanel, {
	initComponent : function() {
		this.items = [
			new OMV.Module.Diagnostics.SysInfo.ServicesOverview
		];
		var plugins = OMV.PluginMgr.get("sysinfo");
		plugins.each(function(plugin) {
			if (plugin.type === "service") {
				this.items.push(new plugin.cls());
			}
		}, this);
		OMV.Module.Diagnostics.SysInfo.Services.superclass.initComponent.
		  apply(this, arguments);
	}
});
OMV.NavigationPanelMgr.registerPanel("diagnostics", "sysinfo", {
	cls: OMV.Module.Diagnostics.SysInfo.Services,
	title: "Services",
	position: 30
});

/**
 * @class OMV.Module.Diagnostics.SysInfo.ServicesOverview
 * @derived OMV.grid.TBarGridPanel
 * Display statistics from various services, e.g. SSH, FTP or SMB/CIFS.
 */
OMV.Module.Diagnostics.SysInfo.ServicesOverview = function(config) {
	var initialConfig = {
		title: "Overview",
		hideAdd: true,
		hideEdit: true,
		hideDelete: true,
		hideRefresh: false,
		hidePagingToolbar: true,
		disableSelection: true,
		stateId: "976130ef-a647-40e8-9b08-02ced906680a",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Service",
				sortable: true,
				dataIndex: "title",
				id: "title"
			},{
				header: "Enabled",
				sortable: true,
				dataIndex: "enabled",
				id: "enabled",
				width: 45,
				align: "center",
				renderer: OMV.util.Format.booleanIconRenderer("switch_on.png",
				  "switch_off.png")
			},{
				header: "Running",
				sortable: true,
				dataIndex: "running",
				id: "running",
				width: 45,
				align: "center",
				renderer: OMV.util.Format.booleanIconRenderer(
				  "led_green.png", "led_red.png")
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.ServicesOverview.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.ServicesOverview,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("Services", "getStatus"),
			reader: new Ext.data.JsonReader({
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "name" },
					{ name: "title" },
					{ name: "enabled" },
					{ name: "running" }
    			]
			})
		});
		OMV.Module.Diagnostics.SysInfo.ServicesOverview.superclass.
		  initComponent.apply(this, arguments);
	}
});

/**
 * @class OMV.Module.Diagnostics.SysInfo.Memory
 * @derived OMV.RrdGraphPanel
 */
OMV.Module.Diagnostics.SysInfo.Memory = function(config) {
	var initialConfig = {
		title: "Memory usage",
		rrdGraphName: "memory"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.Memory.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.Memory, OMV.RrdGraphPanel, {
});
OMV.preg("sysinfo", "system", OMV.Module.Diagnostics.SysInfo.Memory);

/**
 * @class OMV.Module.Diagnostics.SysInfo.Load
 * @derived OMV.RrdGraphPanel
 */
OMV.Module.Diagnostics.SysInfo.Load = function(config) {
	var initialConfig = {
		title: "Load average",
		rrdGraphName: "load"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.Load.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.Load, OMV.RrdGraphPanel, {
});
OMV.preg("sysinfo", "system", OMV.Module.Diagnostics.SysInfo.Load);

/**
 * @class OMV.Module.Diagnostics.SysInfo.Cpu
 * @derived OMV.RrdGraphPanel
 */
OMV.Module.Diagnostics.SysInfo.Cpu = function(config) {
	var initialConfig = {
		title: "CPU usage",
		rrdGraphName: "cpu-0"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.Cpu.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.Cpu, OMV.RrdGraphPanel, {
});
OMV.preg("sysinfo", "system", OMV.Module.Diagnostics.SysInfo.Cpu);

/**
 * @class OMV.Module.Diagnostics.SysInfo.Ifaces
 * @derived Ext.TabPanel
 */
OMV.Module.Diagnostics.SysInfo.Ifaces = function(config) {
	var initialConfig = {
		title: "Interfaces",
		border: false,
		activeTab: 0,
		layoutOnTabChange: true,
		enableTabScroll: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.Ifaces.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.Ifaces, Ext.TabPanel, {
	initComponent : function() {
		this.items = [];
		OMV.Ajax.request(function(id, response, error) {
			  if (error !== null) {
				  OMV.MessageBox.error(null, error);
			  } else {
				  response.each(function(item) {
					  this.add(new OMV.RrdGraphPanel({
							title: item.devicename,
							rrdGraphName: "interface-" + item.devicename
						}));
				  }, this);
			  }
		  }, this, "Network", "enumerateApplicableDevices");
		OMV.Module.Diagnostics.SysInfo.Ifaces.superclass.initComponent.
		  apply(this, arguments);
	}
});
OMV.preg("sysinfo", "system", OMV.Module.Diagnostics.SysInfo.Ifaces);

/**
 * @class OMV.Module.Diagnostics.SysInfo.DiskUsage
 * @derived Ext.TabPanel
 */
OMV.Module.Diagnostics.SysInfo.DiskUsage = function(config) {
	var initialConfig = {
		title: "Disk usage",
		border: false,
		activeTab: 0,
		layoutOnTabChange: true,
		enableTabScroll: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.SysInfo.DiskUsage.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.SysInfo.DiskUsage, Ext.TabPanel, {
	initComponent : function() {
		this.items = [];
		OMV.Ajax.request(function(id, response, error) {
			  if (error !== null) {
				  OMV.MessageBox.error(null, error);
			  } else {
				  response.each(function(item) {
					  this.add(new OMV.RrdGraphPanel({
							title: item.label || item.devicefile,
							rrdGraphName: "df-" + item.mountpoint.substr(1).
							  replace(/\//g, "-")
						}));
				  }, this);
			  }
		  }, this, "FileSystemMgmt", "enumerateMountedFilesystems");
		OMV.Module.Diagnostics.SysInfo.DiskUsage.superclass.initComponent.
		  apply(this, arguments);
	}
});
OMV.preg("sysinfo", "system", OMV.Module.Diagnostics.SysInfo.DiskUsage);
