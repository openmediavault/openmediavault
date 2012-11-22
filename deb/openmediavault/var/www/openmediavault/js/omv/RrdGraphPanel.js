/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
// require("js/omv/MessageBox.js")
// require("js/omv/data/Connection.js")

Ext.ns("OMV");

/**
 * @class OMV.RrdGraphPanel
 * @derived Ext.Panel
 * Panel that is displaying RRD graphs.
 * @config rrdGraphName (required) The name of the RRD graph file. Such a file
 * looks like xxxxx-(hour|day|week|month|year).png.
 */
OMV.RrdGraphPanel = function(config) {
	var initialConfig = {
		autoScroll: true
	};
	Ext.apply(initialConfig, config);
	OMV.RrdGraphPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.RrdGraphPanel, Ext.Panel, {
	initComponent : function() {
		var tpl = this.getTemplate();
		Ext.apply(this, {
			html: tpl.apply({
				name: this.rrdGraphName,
				time: new Date().dateFormat("U")
			}),
			tbar: new Ext.Toolbar({
				items: [{
					id: this.getId() + "-refresh",
					xtype: "button",
					text: _("Refresh"),
					icon: "images/reload.png",
					handler:this.cbRefreshBtnHdl,
					scope: this
				}]
			})
		});
		OMV.RrdGraphPanel.superclass.initComponent.apply(this, arguments);
	},

	getTemplate : function() {
		return new Ext.XTemplate(
		  '<div class="x-panel-rrdgraph">',
		  '  <img src="rrd.php?name={name}-hour.png&time={time}" alt="RRD graph - by hour"/><br/>',
		  '  <img src="rrd.php?name={name}-day.png&time={time}" alt="RRD graph - by day"/><br/>',
		  '  <img src="rrd.php?name={name}-week.png&time={time}" alt="RRD graph - by week"/><br/>',
		  '  <img src="rrd.php?name={name}-month.png&time={time}" alt="RRD graph - by month"/><br/>',
		  '  <img src="rrd.php?name={name}-year.png&time={time}" alt="RRD graph - by year"/>',
		  '</div>');
	},

	/**
	 * @method cbRefreshBtnHdl
	 * Handler that is called when the 'Refresh' button in the top toolbar
	 * is pressed. Override this method to customize the default behaviour.
	 */
	cbRefreshBtnHdl : function() {
		OMV.MessageBox.wait(null, _("Generating graphs ..."));
		OMV.Ajax.request(function(id, response, error) {
			  if (error !== null) {
				  OMV.MessageBox.hide();
				  OMV.MessageBox.error(null, error);
			  } else {
				  this.cmdId = response;
				  OMV.Ajax.request(this.cbIsRunningHdl, this, "Exec",
					"isRunning", { "id": this.cmdId });
			  }
		  }, this, "Rrd", "generate");
	},

	cbIsRunningHdl : function(id, response, error) {
		if (error !== null) {
			delete this.cmdId;
			OMV.MessageBox.hide();
			OMV.MessageBox.error(null, error);
		} else {
			if (response === true) {
				(function() {
				  OMV.Ajax.request(this.cbIsRunningHdl, this, "Exec",
					"isRunning", { "id": this.cmdId });
				}).defer(1000, this);
			} else {
				delete this.cmdId;
				var tpl = this.getTemplate();
				this.update(tpl.apply({
					name: this.rrdGraphName,
					time: new Date().dateFormat("U")
				}));
				OMV.MessageBox.hide();
			}
		}
	}
});
