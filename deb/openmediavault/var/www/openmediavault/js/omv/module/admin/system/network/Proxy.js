/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

/**
 * @class OMV.module.admin.system.network.Proxy
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.system.network.Proxy", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "Network",
	rpcGetMethod: "getProxy",
	rpcSetMethod: "setProxy",

	getFormItems: function() {
		return [{
			xtype: "textfield",
			fieldLabel: _("HTTP-Proxy"),
			name: "httpuri",
			allowBlank: true
		},{
			xtype: "textfield",
			fieldLabel: _("HTTPS-Proxy"),
			name: "httpsuri",
			allowBlank: true
		},{
			xtype: "textfield",
			fieldLabel: _("FTP-Proxy"),
			name: "ftpuri",
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("The URI must look like [USERNAME:PASSWORD@]HOST[:PORT].")
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "proxy",
	path: "/system/network",
	text: _("Proxy"),
	position: 30,
	className: "OMV.module.admin.system.network.Proxy"
});
