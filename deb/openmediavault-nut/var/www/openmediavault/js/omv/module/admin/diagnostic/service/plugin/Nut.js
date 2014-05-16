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
// require("js/omv/workspace/tab/Panel.js")
// require("js/omv/workspace/panel/Textarea.js")
// require("js/omv/workspace/panel/RrdGraph.js")

/**
 * @ingroup webgui
 * @class OMV.module.admin.diagnostic.service.plugin.Variables
 * @derived OMV.workspace.panel.Textarea
 */
Ext.define("OMV.module.admin.diagnostic.service.plugin.nut.Variables", {
	extend: "OMV.workspace.panel.Textarea",

	title: _("Variables"),
	rpcService: "Nut",
	rpcMethod: "getStats"
});

/**
 * @ingroup webgui
 * @class OMV.module.admin.diagnostic.service.plugin.nut.Charge
 * @derived OMV.workspace.panel.RrdGraph
 */
Ext.define("OMV.module.admin.diagnostic.service.plugin.nut.Charge", {
	extend: "OMV.workspace.panel.RrdGraph",

	title: _("Charge"),
	rrdGraphName: "nut-charge"
});

/**
 * @ingroup webgui
 * @class OMV.module.admin.diagnostic.service.plugin.nut.Load
 * @derived OMV.workspace.panel.RrdGraph
 */
Ext.define("OMV.module.admin.diagnostic.service.plugin.nut.Load", {
	extend: "OMV.workspace.panel.RrdGraph",

	title: _("Load"),
	rrdGraphName: "nut-load"
});

/**
 * @ingroup webgui
 * @class OMV.module.admin.diagnostic.service.plugin.nut.Temperature
 * @derived OMV.workspace.panel.RrdGraph
 */
Ext.define("OMV.module.admin.diagnostic.service.plugin.nut.Temperature", {
	extend: "OMV.workspace.panel.RrdGraph",

	title: _("Temperature"),
	rrdGraphName: "nut-temperature"
});

/**
 * @ingroup webgui
 * @class OMV.module.admin.diagnostic.service.plugin.nut.Voltage
 * @derived OMV.workspace.panel.RrdGraph
 */
Ext.define("OMV.module.admin.diagnostic.service.plugin.nut.Voltage", {
	extend: "OMV.workspace.panel.RrdGraph",

	title: _("Voltage"),
	rrdGraphName: "nut-voltage"
});

/**
 * @ingroup webgui
 * @class OMV.module.admin.diagnostic.service.plugin.Nut
 * @derived OMV.workspace.tab.Panel
 */
Ext.define("OMV.module.admin.diagnostic.service.plugin.Nut", {
	extend: "OMV.workspace.tab.Panel",
	alias: "omv.plugin.diagnostic.service.nut",

	title: _("UPS"),

	initComponent: function() {
		var me = this;
		me.items = [
			Ext.create("OMV.module.admin.diagnostic.service.plugin.nut.Variables"),
			Ext.create("OMV.module.admin.diagnostic.service.plugin.nut.Charge"),
			Ext.create("OMV.module.admin.diagnostic.service.plugin.nut.Load"),
			Ext.create("OMV.module.admin.diagnostic.service.plugin.nut.Temperature"),
			Ext.create("OMV.module.admin.diagnostic.service.plugin.nut.Voltage")
		];
		me.callParent(arguments);
	}
});
