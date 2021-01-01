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
// require("js/omv/grid/column/WhiteSpace.js")
// require("js/omv/module/admin/diagnostic/log/plugin/Plugin.js")

/**
 * @class OMV.module.admin.diagnostic.log.plugin.apt.Term
 * @derived OMV.module.admin.diagnostic.log.plugin.Plugin
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.apt.Term", {
	extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",
	alias: "omv.plugin.diagnostic.log.apt.term",
	requires: [
		"OMV.grid.column.WhiteSpace"
	],

	id: "apt_term",
	text: _("Update Management - Terminal output"),
	stateful: true,
	stateId: "aba0b41e-d196-11e5-88f3-175cc496a170",
	columns: [{
		xtype: "whitespacecolumn",
		text: _("Message"),
		sortable: true,
		dataIndex: "message",
		stateId: "message",
		flex: 1
	}],
	rpcParams: {
		id: "apt_term"
	},
	rpcFields: [
		{ name: "rownum", type: "int" },
		{ name: "message", type: "string" }
	]
});

/**
 * @class OMV.module.admin.diagnostic.log.plugin.apt.History
 * @derived OMV.module.admin.diagnostic.log.plugin.Plugin
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.apt.History", {
	extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",
	alias: "omv.plugin.diagnostic.log.apt.history",
	requires: [
		"OMV.grid.column.WhiteSpace"
	],

	id: "apt_history",
	text: _("Update Management - History"),
	stateful: true,
	stateId: "d0a40806-d196-11e5-87f2-ebedf5cb64a0",
	columns: [{
		xtype: "whitespacecolumn",
		text: _("Message"),
		sortable: true,
		dataIndex: "message",
		stateId: "message",
		flex: 1
	}],
	rpcParams: {
		id: "apt_history"
	},
	rpcFields: [
		{ name: "rownum", type: "int" },
		{ name: "message", type: "string" }
	]
});
