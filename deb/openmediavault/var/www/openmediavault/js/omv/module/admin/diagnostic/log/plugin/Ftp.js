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
 * @class OMV.module.admin.diagnostic.log.plugin.ftp.Syslog
 * @derived OMV.module.admin.diagnostic.log.plugin.Plugin
 * Class that implements the 'FTP' logfile diagnostics plugin.
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.ftp.Syslog", {
	extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",
	alias: "omv.plugin.diagnostic.log.ftp.syslog",
	requires: [
		"OMV.grid.column.WhiteSpace"
	],

	id: "proftpd",
	text: _("FTP"),
	stateful: true,
	stateId: "c9d06952-00da-11e1-aa29-00221568ca88",
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
		xtype: "whitespacecolumn",
		text: _("Message"),
		sortable: true,
		dataIndex: "message",
		stateId: "message",
		flex: 1
	}],
	rpcParams: {
		id: "proftpd"
	},
	rpcFields: [
		{ name: "rownum", type: "int" },
		{ name: "ts", type: "int" },
		{ name: "date", type: "string" },
		{ name: "hostname", type: "string" },
		{ name: "message", type: "string" }
	]
});

/**
 * @class OMV.module.admin.diagnostic.log.plugin.ftp.Xferlog
 * @derived OMV.module.admin.diagnostic.log.plugin.Plugin
 * Class that implements the 'FTP' logfile diagnostics plugin.
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.ftp.Xferlog", {
	extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",
	alias: "omv.plugin.diagnostic.log.xferlog",

	id: "proftpd_xferlog",
	text: _("FTP - Transfer log"),
	stateful: true,
	stateId: "26dc6a2a-36fc-11e3-b814-000c292545c1",
	columns: [{
		text: _("Date & Time"),
		sortable: true,
		dataIndex: "rownum",
		stateId: "date",
		renderer: function(value, metaData, record) {
			return record.get("date");
		}
	},{
		text: _("Remote host"),
		sortable: true,
		dataIndex: "remotehost",
		stateId: "remotehost"
	},{
		text: _("Filename"),
		sortable: true,
		dataIndex: "filename",
		stateId: "filename",
		flex: 1
	},{
		text: _("Filesize"),
		sortable: true,
		dataIndex: "filesize",
		stateId: "filesize",
		renderer: function(value, metaData, record) {
			value = parseInt(value);
			return value.binaryFormat();
		}
	},{
		text: _("Transfer time"),
		sortable: true,
		dataIndex: "transfertime",
		stateId: "transfertime",
		renderer: function(value, metaData, record) {
			return Ext.String.format("{0} sec", value);
		}
	},{
		text: _("Direction"),
		sortable: true,
		dataIndex: "direction",
		stateId: "direction",
		renderer: function(value, metaData, record) {
			var map = {
				o: _("Outgoing"),
				i: _("Incoming")
			};
			return map[value];
		}
	},{
		text: _("Access mode"),
		sortable: true,
		dataIndex: "accessmode",
		stateId: "accessmode",
		renderer: function(value, metaData, record) {
			var map = {
				a: _("Anonymous"),
				g: _("Guest"),
				r: _("Real")
			};
			return map[value];
		}
	},{
		text: _("Username"),
		sortable: true,
		dataIndex: "username",
		stateId: "username"
	},{
		text: _("Servicename"),
		sortable: true,
		dataIndex: "servicename",
		stateId: "servicename",
		hidden: true
	},{
		text: _("Transfer type"),
		sortable: true,
		dataIndex: "transfertype",
		stateId: "transfertype",
		renderer: function(value, metaData, record) {
			var map = {
				a: _("ASCII"),
				b: _("Binary")
			};
			return map[value];
		}
	},{
		text: _("Special action flag"),
		sortable: true,
		dataIndex: "specialactionflag",
		stateId: "specialactionflag",
		hidden: true,
		renderer: function(value, metaData, record) {
			var map = {
				C: _("Compressed"),
				U: _("Uncompressed"),
				T: _("TAR"),
				"_": _("No action")
			};
			return map[value];
		}
	},{
		text: _("Authentication method"),
		sortable: true,
		dataIndex: "authenticationmethod",
		stateId: "authenticationmethod",
		hidden: true,
		renderer: function(value, metaData, record) {
			var map = {
				0: _("None"),
				1: _("RFC931")
			};
			return map[value];
		}
	},{
		text: _("Authenticated user ID"),
		sortable: true,
		dataIndex: "authenticateduserid",
		stateId: "authenticateduserid",
		hidden: true,
		renderer: function(value, metaData, record) {
			if(value === "*")
				value =  _("n/a");
			return value;
		}
	},{
		text: _("Completion status"),
		sortable: true,
		dataIndex: "completionstatus",
		stateId: "completionstatus",
		renderer: function(value, metaData, record) {
			var map = {
				c: _("Complete"),
				i: _("Incomplete")
			};
			return map[value];
		}
	}],
	rpcParams: {
		id: "proftpd_xferlog"
	},
	rpcFields: [
		{ name: "rownum", type: "int" },
		{ name: "ts", type: "int" },
		{ name: "date", type: "string" },
		{ name: "transfertime", type: "string" },
		{ name: "remotehost", type: "string" },
		{ name: "filesize", type: "string" },
		{ name: "filename", type: "string" },
		{ name: "transfertype", type: "string" },
		{ name: "specialactionflag", type: "string" },
		{ name: "direction", type: "string" },
		{ name: "accessmode", type: "string" },
		{ name: "username", type: "string" },
		{ name: "servicename", type: "string" },
		{ name: "authenticationmethod", type: "string" },
		{ name: "authenticateduserid", type: "string" },
		{ name: "completionstatus", type: "string" }
	]
});
