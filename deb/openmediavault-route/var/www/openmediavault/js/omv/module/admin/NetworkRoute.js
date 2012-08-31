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
// require("js/omv/module/admin/Network.js")
// require("js/omv/NavigationPanel.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.System.Network");

/**
 * @class OMV.Module.System.Network.RouteGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.System.Network.RouteGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		hideEdit: true, // Simplifies duplicate checks
		stateId: "a6faec48-f389-11e1-8b67-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Network"),
				sortable: true,
				dataIndex: "network",
				id: "network"
			},{
				header: _("Gateway"),
				sortable: true,
				dataIndex: "gateway",
				id: "gateway"
			},{
				header: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.RouteGridPanel.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.RouteGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "NetworkRoute",
				"method": "getList"
			}),
			reader: new Ext.data.JsonReader({
				"idProperty": "uuid",
				"totalProperty": "total",
				"root": "data",
				"fields": [
					{ "name": "uuid" },
					{ "name": "network" },
					{ "name": "gateway" },
					{ "name": "comment" }
				]
			})
		});
		OMV.Module.System.Network.RouteGridPanel.superclass.
		  initComponent.apply(this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.System.Network.RoutePropertyDialog({
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
		var wnd = new OMV.Module.System.Network.RoutePropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "NetworkRoute",
		  "delete", { "uuid": record.get("uuid") });
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "network", {
	cls: OMV.Module.System.Network.RouteGridPanel,
	position: 70,
	title: _("Static Routes")
});

/**
 * @class OMV.Module.System.Network.RoutePropertyDialog
 * @derived OMV.CfgObjectDialog
 * @param uuid The UUID of the static route.
 */
OMV.Module.System.Network.RoutePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "NetworkRoute",
		rpcGetMethod: "get",
		rpcSetMethod: "set",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add static route") : _("Edit static route"),
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.Network.RoutePropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.System.Network.RoutePropertyDialog,
  OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "network",
			fieldLabel: _("Network"),
			vtype: "IPv4NetCIDR",
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("IP or network address.")
		},{
			xtype: "textfield",
			name: "gateway",
			fieldLabel: _("Gateway"),
			vtype: "IPv4",
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("Gateway used to reach the above network address.")
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		}];
	}
});
