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
 * @class OMV.module.admin.diagnostic.log.plugin.Smart
 * @derived OMV.module.admin.diagnostic.log.plugin.Plugin
 * Class that implements the 'S.M.A.R.T.' logfile diagnostics plugin.
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.Smart", {
	extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",
	alias: "omv.plugin.diagnostic.log.smart",
	requires: [
		"OMV.grid.column.WhiteSpace"
	],

	id: "smart",
	text: _("S.M.A.R.T."),
	stateful: true,
	stateId: "67659b68-3cb2-4434-a92e-f541236e12d0",
	columns: [{
		text: _("Date & Time"),
		sortable: true,
		dataIndex: "rownum",
		stateId: "date",
		renderer: function(value, metaData, record) {
			return record.get("date");
		}
	},{
		xtype: "whitespacecolumn",
		text: _("Message"),
		sortable: true,
		dataIndex: "message",
		stateId: "message",
		flex: 1
	}],
	rpcParams: {
		id: "smartd"
	},
	rpcFields: [
		{ name: "rownum", type: "int" },
		{ name: "ts", type: "int" },
		{ name: "date", type: "string" },
		{ name: "message", type: "string" }
	]
});
