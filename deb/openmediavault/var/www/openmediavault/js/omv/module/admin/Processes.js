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
// require("js/omv/NavigationPanel.js")
// require("js/omv/DiagPanel.js")

Ext.ns("OMV.Module.Diagnostics");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("diagnostics", "processes", {
	text: _("Processes"),
	icon: "images/monitor.png",
	position: 30
});

/**
 * @class OMV.Module.Diagnostics.Processes
 * @derived OMV.DiagPanel
 */
OMV.Module.Diagnostics.Processes = function(config) {
	var initialConfig = {
		layout: "fit",
		items: [{
			id: this.getId() + "-content",
			xtype: "textarea",
			name: "processes",
			readOnly: true,
			cls: "x-form-textarea-monospaced",
			disabledClass: ""
		}]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.Processes.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Diagnostics.Processes, OMV.DiagPanel, {
	doLoad : function() {
		OMV.MessageBox.wait(null, _("Loading ..."));
		OMV.Ajax.request(function(id, response, error) {
			  OMV.MessageBox.updateProgress(1);
			  OMV.MessageBox.hide();
			  if (error !== null) {
				  OMV.MessageBox.error(null, error);
			  } else {
				  var comp = this.getComponent(this.getId() + "-content");
				  if (!Ext.isEmpty(comp)) {
					  comp.setValue(response);
				  }
			  }
		  }, this, "System", "getTopInfo");
	}
});
OMV.NavigationPanelMgr.registerPanel("diagnostics", "processes", {
	cls: OMV.Module.Diagnostics.Processes
});
