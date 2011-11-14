/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
// require("js/omv/grid/GridPanel.js")

Ext.ns("OMV.grid");

/**
 * @class OMV.grid.TBarGridPanel
 * @derived OMV.grid.GridPanel
 * An enhanced grid panel. This grid provides 'Add', 'Edit' and 'Delete'
 * buttons in the toolbar by default. The basic delete functionality is also
 * implemented, simply overwrite the 'doDeletion' and 'afterDeletion' functions
 * to implement fit your requirements. To implement the 'Add' and 'Edit'
 * functionality overwrite the 'cbAddBtnHdl' and 'cbEditBtnHdl' callback
 * functions. A paging toolbar which is displayed at the bottom of the grid can
 * be displayed also. It is also possible to reload the grid automatically in a
 * given interval.
 * @config hideToolbar TRUE to hide the whole toolbar. Defaults to FALSE.
 * @config hidePagingToolbar TRUE to hide the paging toolbar at the bottom of
 * the grid. Defaults to TRUE.
 * @config hideAdd Hide the 'Add' button in the top toolbar. Defaults to FALSE.
 * @config hideEdit Hide the 'Edit' button in the top toolbar. Defaults to
 * FALSE.
 * @config hideDelete Hide the 'Delete' button in the top toolbar. Defaults
 * to FALSE.
 * @config hideUp Hide the 'Up' button in the top toolbar. Defaults to TRUE.
 * @config hideDown Hide the 'Down' button in the top toolbar. Defaults to TRUE.
 * @config hideRefresh Hide the 'Refresh' button in the top toolbar. Defaults
 * to TRUE.
 * @config addButtonText The 'Add' button text. Defaults to 'Add'.
 * @config editButtonText The 'Edit' button text. Defaults to 'Edit'.
 * @config deleteButtonText The 'Delete' button text. Defaults to 'Delete'.
 * @config upButtonText The 'Up' button text. Defaults to 'Up'.
 * @config downButtonText The 'Down' button text. Defaults to 'Down'.
 * @config refreshButtonText The 'Refresh' button text. Defaults to 'Refresh'.
 * @config deletionConfirmRequired Set to TRUE to force the user to confirm
 * the deletion request. Defaults to TRUE.
 * @config deletionWaitMsg The message displayed during the deletion process.
 * @config mode The mode how to retrieve the data displayed in the grid panel.
 * This can be 'local' or 'remote' which means the data is requested via RPC.
 * Defaults to 'remote'.
 * @config autoReload TRUE to reload the grid content automatically every n
 * milliseconds. Defaults to FALSE.
 * @config reloadInterval The frequency in milliseconds with which the grid
 * content should be reloaded. Defaults to 10 seconds.
 */
OMV.grid.TBarGridPanel = function(config) {
	var initialConfig = {
		hideToolbar: false,
		hidePagingToolbar: true,
		hideAdd: false, // Hide the 'Add' button in the top toolbar
		hideEdit: false, // Hide the 'Edit' button in the top toolbar
		hideDelete: false, // Hide the 'Delete' button in the top toolbar
		hideUp: true, // Hide the 'Up' button in the top toolbar
		hideDown: true, // Hide the 'Down' button in the top toolbar
		hideRefresh: true, // Hide the 'Refresh' button in the top toolbar
		addButtonText: "Add",
		editButtonText: "Edit",
		deleteButtonText: "Delete",
		upButtonText: "Up",
		downButtonText: "Down",
		refreshButtonText: "Refresh",
		deletionConfirmRequired: true,
		deletionWaitMsg: "Deleting selected item(s)",
		mode: "remote",
		autoReload: false,
		reloadInterval: 10000
	};
	Ext.apply(initialConfig, config);
	OMV.grid.TBarGridPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.grid.TBarGridPanel, OMV.grid.GridPanel, {
	initComponent : function() {
		// Initialize toolbars
		if (!this.hideToolbar) {
			this.tbar = this.initToolbar();
		}
		if (!this.hidePagingToolbar) {
			this.bbar = new Ext.PagingToolbar({
				store: this.store,
				displayInfo: true,
				displayMsg: "Displaying items {0} - {1} of {2}",
				emptyMsg: "No items to display"
			});
		}
		OMV.grid.TBarGridPanel.superclass.initComponent.apply(this,
		  arguments);
		// Register event handler
		this.getSelectionModel().on("selectionchange",
		  this.cbSelectionChangeHdl, this);
		if (!this.hideToolbar && !this.hideEdit) {
			this.on("rowdblclick", this.cbEditBtnHdl, this);
		}
		if (this.autoReload === true) {
			this.on("render", function(c) {
				  if (Ext.isEmpty(this.reloadTask)) {
					  this.reloadTask = Ext.TaskMgr.start({
						  run: this.doReload,
						  scope: this,
						  interval: this.reloadInterval
					  });
				  }
			  }, this);
			this.on("beforedestroy", function(c) {
				  if (!Ext.isEmpty(this.reloadTask)) {
					  Ext.TaskMgr.stop(this.reloadTask);
					  delete this.reloadTask;
				  }
			  }, this);
		}
	},

	/**
	 * @method initToolbar
	 * Override this method if you want to customize the toolbar.
	 */
	initToolbar : function() {
		return new Ext.Toolbar({
			items: [{
				id: this.getId() + "-add",
				xtype: "button",
				text: this.addButtonText,
				icon: "images/add.png",
				hidden: this.hideAdd,
				handler: this.cbAddBtnHdl,
				scope: this
			},{
				id: this.getId() + "-edit",
				xtype: "button",
				text: this.editButtonText,
				icon: "images/edit.png",
				hidden: this.hideEdit,
				handler: this.cbEditBtnHdl,
				scope: this,
				disabled: true
			},{
				id: this.getId() + "-delete",
				xtype: "button",
				text: this.deleteButtonText,
				icon: "images/delete.png",
				hidden: this.hideDelete,
				handler: this.cbDeleteBtnHdl,
				scope: this,
				disabled: true
			},{
				id: this.getId() + "-up",
				xtype: "button",
				text: this.upButtonText,
				icon: "images/arrow-up.png",
				hidden: this.hideUp,
				handler: this.cbUpBtnHdl,
				scope: this,
				disabled: true
			},{
				id: this.getId() + "-down",
				xtype: "button",
				text: this.downButtonText,
				icon: "images/arrow-down.png",
				hidden: this.hideDown,
				handler: this.cbDownBtnHdl,
				scope: this,
				disabled: true
			},{
				id: this.getId() + "-refresh",
				xtype: "button",
				text: this.refreshButtonText,
				icon: "images/reload.png",
				hidden: this.hideRefresh,
				handler: this.cbRefreshBtnHdl,
				scope: this
			}]
		});
	},

	/**
	 * @method cbSelectionChangeHdl
	 * Handler that is called whenever the selection in the grid has
	 * been changed. The top toolbar buttons will be enabled/disabled
	 * depending on how much rows has been selected.
	 * @param model The selection model
	 */
	cbSelectionChangeHdl : function(model) {
		if (this.hideToolbar)
			return;
		var tbarBtnName = [ "edit", "delete", "up", "down" ];
		var tbarBtnDisabled = {
			"edit": false,
			"delete": false,
			"up": true,
			"down": true
		};
		var records = model.getSelections();
		// Enable/disable buttons depending on the number of selected rows.
		if (records.length <= 0) {
			tbarBtnDisabled["edit"] = true;
			tbarBtnDisabled["delete"] = true;
			tbarBtnDisabled["up"] = true;
			tbarBtnDisabled["down"] = true;
		} else if (records.length == 1) {
			tbarBtnDisabled["edit"] = false;
			tbarBtnDisabled["delete"] = false;
			tbarBtnDisabled["up"] = false;
			tbarBtnDisabled["down"] = false;
		} else {
			tbarBtnDisabled["edit"] = true;
			tbarBtnDisabled["delete"] = false;
			tbarBtnDisabled["up"] = false;
			tbarBtnDisabled["down"] = false;
		}
		// Disable 'Delete' button if a selected row is in usage or readonly.
		for (var i = 0; i < records.length; i++) {
			if ((true == records[i].get("_used")) ||
			  (true == records[i].get("_readOnly"))) {
				tbarBtnDisabled["delete"] = true;
			}
		}
		// Update the button controls.
		for (var i = 0; i < tbarBtnName.length; i++) {
			var tbarBtnCtrl = this.getTopToolbar().findById(this.getId() +
			  "-" + tbarBtnName[i]);
			if (!Ext.isEmpty(tbarBtnCtrl)) {
				if (true == tbarBtnDisabled[tbarBtnName[i]]) {
					tbarBtnCtrl.disable();
				} else {
					tbarBtnCtrl.enable();
				}
			}
		}
	},

	/**
	 * @method doLoad
	 * Load the grid content.
	 */
	doLoad : function() {
		if (this.mode === "remote") {
			this.store.load();
		}
	},

	/**
	 * @method doReload
	 * Reload the grid content.
	 */
	doReload : function() {
		if (this.mode === "remote") {
			this.store.reload();
		}
	},

	/**
	 * @method cbAddBtnHdl
	 * Handler that is called when the 'Add' button in the top toolbar
	 * is pressed. Override this method to customize the default behaviour.
	 */
	cbAddBtnHdl : function() {
		// Nothing to do here
	},

	/**
	 * @method cbEditBtnHdl
	 * Handler that is called when the 'Edit' button in the top toolbar
	 * is pressed. Override this method to customize the default behaviour.
	 */
	cbEditBtnHdl : function() {
		// Nothing to do here
	},

	/**
	 * @method cbUpBtnHdl
	 * Handler that is called when the 'Up' button in the top toolbar
	 * is pressed. Override this method to customize the default behaviour.
	 */
	cbUpBtnHdl : function() {
		var sm = this.getSelectionModel();
		var records = sm.getSelections();
		if (records.length > 0) {
			// Find the smallest index of the selected rows
			var ltIndex = this.store.indexOf(records[0]);
			for (var i = 1; i < records.length; i++) {
				var index = this.store.indexOf(records[i])
				if (ltIndex > index)
					ltIndex = index;
			}
			// Calculate the index where to insert the rows
			var index = ltIndex - records.length;
			if (index < 0) {
				index = 0;
			}
			this.doMoveRows(records, index);
		}
	},

	/**
	 * @method cbDownBtnHdl
	 * Handler that is called when the 'Down' button in the top toolbar
	 * is pressed.
	 */
	cbDownBtnHdl : function() {
		var sm = this.getSelectionModel();
		var records = sm.getSelections();
		if (records.length > 0) {
			// Find the smallest index of the selected rows
			var ltIndex = this.store.indexOf(records[0]);
			for (var i = 1; i < records.length; i++) {
				var index = this.store.indexOf(records[i])
				if (ltIndex > index)
					ltIndex = index;
			}
			// Calculate the index where to insert the rows
			var index = ltIndex + records.length;
			var count = this.store.getCount() - 1;
			if (index > count) {
				index = count;
			}
			this.doMoveRows(records, index);
		}
	},

	/**
	 * @method cbRefreshBtnHdl
	 * Handler that is called when the 'Refresh' button in the top toolbar
	 * is pressed. Override this method to customize the default behaviour.
	 */
	cbRefreshBtnHdl : function() {
		this.doReload();
	},

	/**
	 * @method doMoveRows
	 * Function that is called after the selected rows have been moved.
	 * Override this method to customize the default behaviour.
	 * @param records The records to move.
	 * @param index The index where to insert the rows to be moved.
	 */
	doMoveRows : function(records, index) {
		if (!Ext.isNumber(index))
			return;
		for (var i = 0; i < records.length; i++) {
			var record = this.store.getById(records[i].id);
			this.store.remove(record);
			this.store.insert(index, record);
		}
		this.afterMoveRows(records, index);
	},

	/**
	 * @method afterMoveRows
	 * Function that is called after the selected rows have been moved.
	 * Override this method to customize the default behaviour.
	 * @param records The records that have been move.
	 * @param index The index where the rows have been inserted.
	 */
	afterMoveRows : function(records, index) {
		var sm = this.getSelectionModel();
		sm.selectRecords(records);
	},

	/**
	 * @method cbDeleteBtnHdl
	 * Handler that is called when the 'Delete' button in the top toolbar
	 * is pressed.
	 */
	cbDeleteBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var records = selModel.getSelections();
		if (this.deletionConfirmRequired === true) {
			var msg = "Do you really want to delete the selected " +
			  (records.length == 1 ? "item" : "items") + "?";
			OMV.MessageBox.show({
				title: "Confirmation",
				msg: msg,
				buttons: Ext.Msg.YESNO,
				fn: function(answer) {
					if (answer == "no")
						return;
					this.startDeletion(selModel, records);
				},
				scope: this,
				icon: Ext.Msg.QUESTION
			});
		} else {
			this.startDeletion(selModel, records);
		}
	},

	/**
	 * @private
	 * @method startDeletion
	 * Private method that is called when the deletion of the selected records
	 * has been aggreed.
	 * @param model The grid selection model.
	 * @param records The selected grid items.
	 */
	startDeletion: function(model, records) {
		if (records.length <= 0)
			return;
		// Store selected records in a local variable
		this.delActionInfo = {
			records: records,
			count: records.length
		}
		// Get first record to be deleted
		var record = this.delActionInfo.records.pop();
		// Display progress dialog
		OMV.MessageBox.progress("", this.deletionWaitMsg, "");
		this.updateDelProgress();
		// Execute deletion function
		this.doDeletion(record);
	},

	/**
	 * @method doDeletion
	 * The method that is called to delete a selected record. Override this
	 * method to customize the default behaviour. This is necessary in
	 * 'remote' mode.
	 */
	doDeletion : function(record) {
		if (this.mode === "local") {
			// Remove record from store
			this.store.remove(record);
			// Continue deletion process
			this.cbDeletionHdl(null, null, null);
		}
	},

	/**
	 * @method afterDeletion
	 * Function that is called after the deletion has been finished successful.
	 */
	afterDeletion : function() {
		if (this.mode === "remote") {
			this.doReload();
		}
	},

	/**
	 * @method deletionHdl
	 * The method that is called by the 'deletion' RPC. The progress bar
	 * will be updated and the deletion progress will be continued if
	 * there are still records to delete.
	 */
	cbDeletionHdl : function(id, response, error) {
		if (error !== null) {
			// Remove temporary local variables
			delete this.delActionInfo;
			// Hide progress dialog
			OMV.MessageBox.hide();
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			if (this.delActionInfo.records.length > 0) {
				var record = this.delActionInfo.records.pop();
				// Update progress dialog
				this.updateDelProgress();
				// Execute deletion function
				this.doDeletion(record);
			} else {
				// Remove temporary local variables
				delete this.delActionInfo;
				// Update and hide progress dialog
				OMV.MessageBox.updateProgress(1, "100% completed ...");
				OMV.MessageBox.hide();
				this.afterDeletion();
			}
		}
	},

	/**
	 * @method updateDelProgress
	 * Private helper function to update the progress dialog.
	 */
	updateDelProgress : function() {
		// Calculate percentage
		var p = (this.delActionInfo.count - this.delActionInfo.records.length) /
		  this.delActionInfo.count;
		// Create message text
		var text = Math.round(100 * p) + "% completed ...";
		// Update progress dialog
		OMV.MessageBox.updateProgress(p, text);
	}
});
Ext.reg("tbargrid", OMV.grid.TBarGridPanel);
