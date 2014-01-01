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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/tab/Panel.js")
// require("js/omv/module/admin/service/rsyncd/Settings.js")
// require("js/omv/module/admin/service/rsyncd/Modules.js")

/**
 * @class OMV.module.admin.service.rsyncd.Rsyncd
 * @derived OMV.workspace.tab.Panel
 */
Ext.define("OMV.module.admin.service.rsyncd.Rsyncd", {
	extend: "OMV.workspace.tab.Panel",
	requires: [
		"OMV.module.admin.service.rsyncd.Settings",
		"OMV.module.admin.service.rsyncd.Modules"
	],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			items: [
				Ext.create("OMV.module.admin.service.rsyncd.Settings", {
					title: _("Settings")
				}),
				Ext.create("OMV.module.admin.service.rsyncd.Modules", {
					title: _("Modules")
				})
			]
		});
		me.callParent(arguments);
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "server",
	path: "/service/rsync",
	text: _("Server"),
	position: 20,
	className: "OMV.module.admin.service.rsyncd.Rsyncd"
});
