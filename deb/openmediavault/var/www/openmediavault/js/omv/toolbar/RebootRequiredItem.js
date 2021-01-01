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
// require("js/omv/SessionManager.js")
// require("js/omv/service/SystemInfo.js")

/**
 * @ingroup webgui
 * @class OMV.toolbar.RebootRequiredItem
 * @derived Ext.toolbar.TextItem
 * A toolbar that displays the hostname.
 */
Ext.define("OMV.toolbar.RebootRequiredItem", {
	extend: "Ext.toolbar.TextItem",
	alias: "widget.tbrebootrequired",
	requires: [
		"OMV.SessionManager",
		"OMV.service.SystemInfo",
	],

	hidden: true,

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			html: "<div class='x-fa fa-repeat fa-spin' data-qtip='" +
				_("The system must be rebooted.") + "'></div>"
		});
		me.callParent(arguments);
		if (OMV.SessionManager.isAdministrator()) {
			OMV.service.SystemInfo.on("refresh", me.onRefreshSystemInfo, me);
		}
	},

	destroy: function() {
		var me = this;
		if (OMV.SessionManager.isAdministrator()) {
			OMV.service.SystemInfo.un("refresh", me.onRefreshSystemInfo, me);
		}
		me.callParent();
	},

	onRefreshSystemInfo: function(c, info) {
		var me = this;
		if (me.isVisible() !== info.rebootRequired) {
			me.setVisible(info.rebootRequired);
		}
	}
});
