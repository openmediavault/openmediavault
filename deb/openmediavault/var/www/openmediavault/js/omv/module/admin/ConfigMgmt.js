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
// require("js/omv/data/DataProxy.js")
// require("js/omv/form/FormPanel.js")

Ext.ns("OMV.Module.System");

// Register the menu.
/*
OMV.NavigationPanelMgr.registerMenu("system", "backuprestore", {
	text: "Backup/Restore",
	icon: "images/config.png",
	position: 100
});
*/

/**
 * @class OMV.Module.System.ConfigMgmt
 * @derived OMV.form.FormPanel
 */
OMV.Module.System.ConfigMgmt = function(config) {
	var initialConfig = {
		hideLabel: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.ConfigMgmt.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.System.ConfigMgmt, OMV.form.FormPanel, {
	initComponent : function() {
		this.items = [{
			xtype: "fieldset",
			title: "Backup configuration",
			items: [{
				xtype: "displayfield",
				hideLabel: true,
				value: "Click the 'Backup' button to download the current system configuration."
			},{
				xtype: "button",
				text: "Backup",
				handler: function() {
					OMV.Download.request("Config", "backup");
				}
			}]
		},{
			xtype: "fieldset",
			title: "Restore configuration",
			items: [{
				xtype: "displayfield",
				hideLabel: true,
				value: "To restore a previously backed up configuration click the 'Restore' button. The system will reboot automatically after import has been finished successfully."
			},{
				xtype: "button",
				text: "Restore",
				handler: this.cbRestoreBtnHdl,
				scope: this
			}]
		}];
		OMV.Module.System.ConfigMgmt.superclass.initComponent.apply(this,
		  arguments);
	},

	cbRestoreBtnHdl : function() {
		new OMV.UploadDialog({
			title: "Restore configuration",
			service: "Config",
			method: "restore",
			listeners: {
				success: function(wnd, response) {
					this.cmdId = response;
					OMV.MessageBox.wait(null, "Restore configuration...");
					OMV.Ajax.request(this.cbIsRunningHdl, this, "Exec",
					  "isRunning", [ this.cmdId ]);
				},
				scope: this
			}
		}).show();
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
					"isRunning", [ this.cmdId ]);
				}).defer(1000, this);
			} else {
				delete this.cmdId;
				OMV.MessageBox.hide();
				OMV.Ajax.request(function(id, response, error) {
					  if (error !== null) {
						  OMV.MessageBox.error(null, error);
					  } else {
						  OMV.MessageBox.info(null,
							"The system will reboot now.");
					  }
				  }, this, "System", "reboot");
			}
		}
	}
});
/*
OMV.NavigationPanelMgr.registerPanel("system", "backuprestore", {
	cls: OMV.Module.System.ConfigMgmt
});
*/
