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

/**
 * @class OMV.module.admin.diagnostic.log.plugin.Plugin
 * @derived Ext.Base
 * @param id The plugin identifier. Required.
 * @param text The text to show. Required.
 * @param columns An array of column definition objects which define all
 *   columns that appear in the grid. Required.
 * @param rpcService The RPC service name. Required.
 * @param rpcGetMethod The RPC method to get the log data. Required.
 * @param rpcClearMethod The RPC method to clear the data. Required.
 * @param rpcDownloadMethod The RPC method to download the data. Required.
 * @param rpcParams The RPC method parameters. Required.
 * @param rpcFields The fields for the RPC model. Required.
 * @param rpcRemoteSort TRUE if the sorting should be performed on the
 *   server side, FALSE if it is local only. Defaults to TRUE.
 */
Ext.define("OMV.module.admin.diagnostic.log.plugin.Plugin", {
	extend: "Ext.Base",

	columns: [],
	rpcService: "LogFile",
	rpcGetMethod: "getList",
	rpcClearMethod: "clear",
	rpcDownloadMethod: "getContent",
	rpcParams: undefined,
	rpcFields: [],
	rpcRemoteSort: true,
	isLogPlugin: true
});
