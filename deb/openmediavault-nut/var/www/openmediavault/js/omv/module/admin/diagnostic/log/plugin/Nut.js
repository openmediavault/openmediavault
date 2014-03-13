/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
// require("js/omv/PluginManager.js")
// require("js/omv/grid/column/Empty.js")
// require("js/omv/grid/column/WhiteSpace.js")
// require("js/omv/module/admin/diagnostic/log/plugin/Plugin.js")

/**
 * @class OMV.module.admin.diagnostic.log.plugin.Nut
 * @derived OMV.module.admin.diagnostic.log.plugin.Plugin
 * Class that implements the 'NUT (Network UPS Tools)' logfile
 * diagnostics plugin.
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.Nut", {
	extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",
	requires: [
		"OMV.grid.column.WhiteSpace"
	],

	id: "nut",
	text: _("UPS"),
	stateful: true,
	stateId: "d24bb3fa-8b86-11e3-a4cc-000c29f7c0eb",
	columns: [{
		text: _("Date & Time"),
		sortable: true,
		dataIndex: "rownum",
		stateId: "date",
		renderer: function(value, metaData, record) {
			return record.get("date");
		}
	},{
		text: _("Program"),
		sortable: true,
		dataIndex: "program",
		stateId: "program",
		flex: 1
	},{
		xtype: "emptycolumn",
		text: _("PID"),
		sortable: true,
		hidden: true,
		dataIndex: "pid",
		stateId: "pid",
		flex: 1
	},{
		xtype: "whitespacecolumn",
		text: _("Message"),
		sortable: true,
		dataIndex: "message",
		stateId: "message",
		flex: 1
	}],
	rpcParams: {
		id: "nut"
	},
	rpcFields: [
		{ name: "rownum", type: "int" },
		{ name: "ts", type: "int" },
		{ name: "date", type: "string" },
		{ name: "program", type: "string" },
		{ name: "pid", type: "string" },
		{ name: "message", type: "string" }
	]
});

OMV.PluginManager.register({
	ptype: "diagnostic",
	id: "log",
	className: "OMV.module.admin.diagnostic.log.plugin.Nut"
});
