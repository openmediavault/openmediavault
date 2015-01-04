/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
// require("js/omv/grid/column/WhiteSpace.js")
// require("js/omv/module/admin/diagnostic/log/plugin/Plugin.js")

/**
 * @class OMV.module.admin.diagnostic.log.plugin.smb.Audit
 * @derived OMV.module.admin.diagnostic.log.plugin.Plugin
 * Class that implements the 'SMB/CIFS' file audit logfile diagnostics plugin.
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.smb.Audit", {
	extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",
	alias: "omv.plugin.diagnostic.log.smb.audit",
	requires: [
		"OMV.grid.column.WhiteSpace"
	],

	id: "smbdaudit",
	text: _("SMB/CIFS - Audit"),
	stateful: true,
	stateId: "b4cc4047-eeca-42ee-9e5c-8338f7e41001",
	columns: [{
		text: _("Date & Time"),
		sortable: true,
		dataIndex: "rownum",
		stateId: "date",
		renderer: function(value, metaData, record) {
			return record.get("date");
		}
	},{
		text: _("Hostname"),
		hidden: true,
		sortable: true,
		dataIndex: "hostname",
		stateId: "hostname"
	},{
		text: _("Username"),
		sortable: true,
		dataIndex: "username",
		stateId: "username"
	},{
		text: _("Client address"),
		sortable: true,
		dataIndex: "clientipaddr",
		stateId: "clientipaddr"
	},{
		text: _("Client NetBIOS name"),
		hidden: true,
		sortable: true,
		dataIndex: "clientnetbiosname",
		stateId: "clientnetbiosname"
	},{
		text: _("Shared folder path"),
		sortable: true,
		dataIndex: "servicerootdir",
		stateId: "servicerootdir",
		flex: 1
	},{
		text: _("Shared folder name"),
		sortable: true,
		dataIndex: "servicename",
		stateId: "servicename"
	},{
		text: _("Filename"),
		sortable: true,
		dataIndex: "filename",
		stateId: "filename",
		flex: 1
	},{
		text: _("Operation"),
		dataIndex: "operation",
		stateId: "operation",
		width: 80
	},{
		xtype: "booleaniconcolumn",
		text: _("Result"),
		sortable: true,
		width: 50,
		resizable: false,
		align: "center",
		dataIndex: "result",
		stateId: "result",
		trueIcon: "led_blue.png",
		falseIcon: "led_red.png"
	}],
	rpcParams: {
		id: "smbdaudit"
	},
	rpcFields: [
		{ name: "rownum", type: "int" },
		{ name: "ts", type: "int" },
		{ name: "date", type: "string" },
		{ name: "hostname", type: "string" },
		{ name: "username", type: "string" },
		{ name: "clientipaddr", type: "string" },
		{ name: "clientnetbiosname", type: "string" },
		{ name: "servicerootdir", type: "string" },
		{ name: "servicename", type: "string" },
		{ name: "operation", type: "string" },
		{ name: "result", type: "string" },
		{ name: "filename", type: "string" }
	]
});
