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
// require("js/omv/workspace/dashboard/View.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/util/Format.js")

/**
 * @class OMV.module.admin.dashboard.view.NetworkInterfaces
 * @derived OMV.workspace.dashboard.View
 */
Ext.define("OMV.module.admin.dashboard.view.NetworkInterfaces", {
	extend: "OMV.workspace.dashboard.View",
	alias: "widget.module.admin.dashboard.view.networkinterfaces",
	requires: [
		"OMV.workspace.grid.Panel",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format"
	],

	height: 200,
	refreshInterval: 10000,

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			items: [ me.gp = Ext.create("OMV.workspace.grid.Panel", {
				disableLoadMaskOnLoad: true,
				hideTopToolbar: true,
				hidePagingToolbar: true,
				disableSelection: true,
				stateful: true,
				stateId: "85bc79a2-f7f5-11e4-a5ad-0002b3a176b4",
				columns: [{
					xtype: "textcolumn",
					text: _("Name"),
					sortable: true,
					dataIndex: "devicename",
					stateId: "devicename",
					flex: 1
				},{
					xtype: "templatecolumn",
					text: _("Address"),
					sortable: true,
					stateId: "address",
					tpl: Ext.String.format(
					  '{0}: {[Ext.util.Format.defaultValue(values.address, "-")]}<br/>' +
					  '{1}: {[Ext.util.Format.defaultValue(values.address6, "-")]}',
					  _("IPv4"), _("IPv6")),
					flex: 1
				},{
					xtype: "templatecolumn",
					text: _("Netmask"),
					sortable: true,
					stateId: "netmask",
					tpl: Ext.String.format(
					  '{0}: {[Ext.util.Format.defaultValue(values.netmask, "-")]}<br/>' +
					  '{1}: {[Ext.util.Format.defaultValue((values.netmask6 < 0) ? "" : values.netmask6, "-")]}',
					  _("IPv4"), _("IPv6")),
					flex: 1
				},{
					xtype: "templatecolumn",
					text: _("Gateway"),
					sortable: true,
					stateId: "gateway",
					tpl: Ext.String.format(
					  '{0}: {[Ext.util.Format.defaultValue(values.gateway, "-")]}<br/>' +
					  '{1}: {[Ext.util.Format.defaultValue(values.gateway6, "-")]}',
					  _("IPv4"), _("IPv6")),
					flex: 1
				},{
					xtype: "emptycolumn",
					text: _("Hardware address"),
					sortable: true,
					dataIndex: "ether",
					stateId: "ether",
					flex: 1
				},{
					xtype: "emptycolumn",
					text: _("MTU"),
					sortable: true,
					dataIndex: "mtu",
					stateId: "mtu",
					width: 60
				},{
					text: _("Speed"),
					sortable: true,
					dataIndex: "speed",
					stateId: "speed",
					renderer: function(value) {
						if (-1 == value)
							return "-";
						return Ext.String.format("{0} Mbits/sec", value);
					},
					flex: 1
				},{
					xtype: "booleanfonticoncolumn",
					text: _("Link"),
					sortable: true,
					dataIndex: "link",
					stateId: "link",
					width: 80,
					resizable: false,
					trueCls: [
						"mdi mdi-ethernet",
						Ext.baseCSSPrefix + "color-boolean-true"
					].join(" "),
					falseCls: [
						"mdi mdi-ethernet",
						Ext.baseCSSPrefix + "color-boolean-false"
					].join(" ")
				}],
				viewConfig: {
					markDirty: false,
					trackOver: false
				},
				store: Ext.create("OMV.data.Store", {
					autoLoad: true,
					model: OMV.data.Model.createImplicit({
						idProperty: "devicename",
						fields: [
							{ name: "devicename", type: "string" },
							{ name: "address", type: "string" },
							{ name: "netmask", type: "string" },
							{ name: "gateway", type: "string" },
							{ name: "method6", type: "string" },
							{ name: "address6", type: "string" },
							{ name: "netmask6", type: "int" },
							{ name: "gateway6", type: "string" },
							{ name: "ether", type: "string" },
							{ name: "mtu", type: "string" },
							{ name: "link", type: "boolean" },
							{ name: "speed", type: "int" }
						]
					}),
					proxy: {
						type: "rpc",
						rpcData: {
							service: "Network",
							method: "enumerateDevicesList",
							options: {
								updatelastaccess: false
							}
						}
					},
					sorters: [{
						direction: "ASC",
						property: "devicename"
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
