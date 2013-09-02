/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
// require("js/omv/module/admin/diagnostic/log/plugin/Plugin.js")

/**
 * @class OMV.module.admin.diagnostic.log.plugin.Rsyncd
 * @derived OMV.module.admin.diagnostic.log.plugin.Plugin
 * Class that implements the 'Rsyncd' logfile diagnostics plugin.
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.Rsyncd", {
	extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",

	id: "rsyncd",
	text: _("Rsync - Server"),
	stateful: true,
	stateId: "a469de30-13ae-11e3-9456-0002b3a176b4",
	columns: [{
		text: _("Date & Time"),
		sortable: true,
		dataIndex: "rownum",
		stateId: "date",
		renderer: function(value, metaData, record) {
			return record.get("date");
		}
	},{
		text: _("Event"),
		sortable: true,
		dataIndex: "event",
		stateId: "event",
		flex: 1
	}],
	rpcParams: {
		id: "rsyncd"
	},
	rpcFields: [
		{ name: "rownum", type: "int" },
		{ name: "ts", type: "int" },
		{ name: "date", type: "string" },
		{ name: "event", type: "string" }
	]
});

OMV.PluginManager.register({
	ptype: "diagnostic",
	id: "log",
	className: "OMV.module.admin.diagnostic.log.plugin.Rsyncd"
});
