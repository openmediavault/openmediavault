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
// require("js/omv/MessageBox.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.System");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "updatemanager", {
	text: "Update Manager",
	icon: "images/system-software-update.png",
	position: 80
});

/**
 * @class OMV.Module.System.UpdateMgmtGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.System.UpdateMgmtGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: true,
		hideAdd: true,
		hideEdit: true,
		hideDelete: true,
		stateId: "1a2ca00e-37ac-4aa4-8cbe-290d8f95bd1b",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: "Version",
				sortable: false,
				dataIndex: "newversion",
				id: "newversion",
				width: 140
			},{
				header: "Description",
				sortable: true,
				dataIndex: "description",
				id: "description",
				renderer: OMV.util.Format.whitespaceRenderer()
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.UpdateMgmtGridPanel.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.System.UpdateMgmtGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		// Set selection model and append checkbox to column model
		this.sm = new Ext.grid.CheckboxSelectionModel({
			singleSelect: false
		});
		this.colModel.columns.unshift(this.sm);
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("Apt", "getUpgraded"),
			reader: new Ext.data.JsonReader({
				idProperty: "name",
				fields: [
					{ name: "name" },
					{ name: "version" },
					{ name: "newversion" },
					{ name: "description" }
    			]
			})
		});
		OMV.Module.System.UpdateMgmtGridPanel.superclass.initComponent.apply(
		  this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.System.UpdateMgmtGridPanel.superclass.
		  initToolbar.apply(this);
		tbar.insert(0, {
			id: this.getId() + "-check",
			xtype: "button",
			text: "Check",
			icon: "images/reload.png",
			handler: this.cbCheckBtnHdl,
			scope: this
		});
		tbar.insert(1, {
			id: this.getId() + "-upload",
			xtype: "button",
			text: "Upload",
			icon: "images/upload.png",
			handler: this.cbUploadBtnHdl,
			scope: this
		});
		tbar.insert(2, {
			id: this.getId() + "-install",
			xtype: "button",
			text: "Install",
			icon: "images/yes.png",
			handler: this.cbInstallBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.System.UpdateMgmtGridPanel.superclass.cbSelectionChangeHdl.
		  apply(this, arguments);
		var records = model.getSelections();
		var tbarInstallCtrl = this.getTopToolbar().findById(this.getId() +
		  "-install");
		if (records.length <= 0) {
			tbarInstallCtrl.disable();
		} else {
			tbarInstallCtrl.enable();
		}
	},

	cbInstallBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var records = selModel.getSelections();
		var packages = [];
		for (var i = 0; i < records.length; i++) {
			var record = records[i];
			packages.push(record.get("name"));
		}
		var wnd = new OMV.ExecCmdDialog({
			title: "Install updates ...",
			rpcService: "Apt",
			rpcMethod: "upgrade",
			rpcArgs: packages,
			hideStart: true,
			hideStop: true,
			killCmdBeforeDestroy: false,
			listeners: {
				finish: function(wnd, response) {
					wnd.appendValue("\nDone ...");
					wnd.setButtonDisabled("close", false);
				},
				exception: function(wnd, error) {
					OMV.MessageBox.error(null, error);
					wnd.setButtonDisabled("close", false);
				},
				close: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.setButtonDisabled("close", true);
		wnd.show();
		wnd.start();
	},

	cbUploadBtnHdl : function() {
		var wnd = new OMV.UploadDialog({
			title: "Upload package",
			service: "Apt",
			method: "upload",
			listeners: {
				success: function(wnd, response) {
					// The upload was successful, now resynchronize the
					// package index files from their sources.
					this.cbCheckBtnHdl();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbCheckBtnHdl : function() {
		OMV.MessageBox.wait(null, "Checking for new updates ...");
		OMV.Ajax.request(function(id, response, error) {
			  if (error !== null) {
				  OMV.MessageBox.hide();
				  OMV.MessageBox.error(null, error);
			  } else {
				  this.cmdId = response;
				  OMV.Ajax.request(this.cbIsRunningHdl, this, "Exec",
					"isRunning", [ this.cmdId ]);
			  }
		  }, this, "Apt", "update");
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
				this.doReload();
			}
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "updatemanager", {
	cls: OMV.Module.System.UpdateMgmtGridPanel
});
