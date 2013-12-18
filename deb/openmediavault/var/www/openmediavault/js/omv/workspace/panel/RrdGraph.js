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
// require("js/omv/workspace/panel/Panel.js")
// require("js/omv/Rpc.js")
// require("js/omv/window/MessageBox.js")

/**
 * @ingroup webgui
 * @class OMV.workspace.panel.RrdGraph
 * @derived OMV.workspace.panel.Panel
 * Panel that is displaying RRD graphs.
 * @param rrdGraphName (required) The name of the RRD graph file. Such a
 *   file looks like xxxxx-(hour|day|week|month|year).png.
 */
Ext.define("OMV.workspace.panel.RrdGraph", {
	extend: "OMV.workspace.panel.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.window.MessageBox"
	],

	hideTopToolbar: false,
	autoLoadData: false,

	initComponent: function() {
		var me = this;
		var tpl = me.getTemplate();
		Ext.apply(me, {
			html: tpl.apply({
				name: me.rrdGraphName,
				time: Ext.Date.now()
			})
		});
		me.callParent(arguments);
	},

	getTemplate: function() {
		return new Ext.XTemplate(
		  '<div class="x-panel-rrdgraph">',
		  '  <img src="rrd.php?name={name}-hour.png&time={time}" alt="RRD graph - by hour"/><br/>',
		  '  <img src="rrd.php?name={name}-day.png&time={time}" alt="RRD graph - by day"/><br/>',
		  '  <img src="rrd.php?name={name}-week.png&time={time}" alt="RRD graph - by week"/><br/>',
		  '  <img src="rrd.php?name={name}-month.png&time={time}" alt="RRD graph - by month"/><br/>',
		  '  <img src="rrd.php?name={name}-year.png&time={time}" alt="RRD graph - by year"/>',
		  '</div>');
	},

	doLoad: function() {
		var me = this;
		OMV.RpcRunner.request({
			msg: _("Generating graphs ..."),
			rpcDelay: 1000,
			rpcData: {
				service: "Rrd",
				method: "generate"
			},
			scope: me,
			finish: function() {
				var tpl = this.getTemplate();
				this.update(tpl.apply({
					name: this.rrdGraphName,
					time: Ext.Date.now()
				}));
			}
		});
	}
});
