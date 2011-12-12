/**
 * Created by JetBrains PhpStorm.
 * User: mbeck
 * Date: 28.11.11
 * Time: 20:53
 * To change this template use File | Settings | File Templates.
 */

// require("js/omv/NavigationPanel.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/form/MultiSelect.js")
// require("js/omv/util/Format.js")
// require("js/omv/ExecCmdDialog.js")

// require("js/omv/module/transmissionbt/util/Format.js")

// require("js/omv/module/transmissionbt/manage/dialog/upload.js")
// require("js/omv/module/transmissionbt/manage/dialog/delete.js")
// require("js/omv/module/transmissionbt/manage/dialog/addurl.js")

Ext.ns("OMV.Module.Services.TransmissionBT.Manage");

/**
 * @class OMV.Module.Services.TransmissionBT.Manage.TorrentListGrid
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.TransmissionBT.Manage.TorrentListGrid = function(config) {
	var initialConfig = {
		title: "Torrent List",
		autoReload: false,
		reloadInterval: 10000,
		hidePagingToolbar: true,
		hideAdd: true,
		hideEdit: true,
		hideDelete: true,
		resumeWaitMsg: "Resuming selected item(s)",
		pauseWaitMsg: "Pausing selected item(s)",
		deleteWaitMsg: "Deleting selected item(s)",
		queueMoveWaitMsg: "Queue moving selcted item(s)",
		stateId: "cb44cbf3-b1cb-b6ba-13548ab0dc7c246c",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "ID",
				sortable: true,
				dataIndex: "id",
				id: "id"
			},{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: "Status",
				sortable: true,
				dataIndex: "status",
				id: "status",
				renderer: this.statusRenderer,
				scope: this
			},{
				header: "Done",
				sortable: true,
				dataIndex: "percentDone",
				id: "percentDone",
				renderer: this.doneRenderer,
				scope: this
			},{
				header: "ETA",
				sortable: true,
				dataIndex: "eta",
				id: "eta",
				renderer: this.etaRenderer,
				scope: this
			},{
				header: "Peers",
				sortable: true,
				id: "peers",
				renderer: this.peersRenderer,
				scope: this
			},{
				header: "DL-Rate",
				sortable: true,
				dataIndex: "rateDownload",
				id: "rateDownload",
				renderer: this.rateRenderer,
				scope: this
			},{
				header: "UL-Rate",
				sortable: true,
				dataIndex: "rateUpload",
				id: "rateUpload",
				renderer: this.rateRenderer,
				scope: this
			},{
				header: "Date Added",
				sortable: true,
				dataIndex: "addedDate",
				id: "addedDate",
				renderer: this.timestampRenderer,
				scope: this
			},{
				header: "Date Done",
				sortable: true,
				dataIndex: "doneDate",
				id: "doneDate",
				renderer: this.timestampRenderer,
				scope: this
			},{
				header: "Ratio",
				sortable: true,
				dataIndex: "uploadRatio",
				id: "uploadRatio",
				renderer: this.ratioRenderer,
				scope: this
			},{
				header: "Queue",
				sortable: true,
				dataIndex: "queuePosition",
				id: "queuePosition"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBT.Manage.TorrentListGrid.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBT.Manage.TorrentListGrid, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: false,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("TransmissionBT", "getList",
				[ [ "userdefined" ] ]),
			reader: new Ext.data.JsonReader({
				idProperty: "id",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "id" },
					{ name: "name" },
					{ name: "status" },
					{ name: "totalSize" },
					{ name: "haveValid" },
					{ name: "percentDone" },
					{ name: "eta" },
					{ name: "peersConnected" },
					{ name: "peersSendingToUs" },
					{ name: "rateDownload" },
					{ name: "rateUpload" },
					{ name: "addedDate" },
					{ name: "doneDate" },
					{ name: "uploadRatio" },
					{ name: "queuePosition" }
    			]
			})
		});
		this.contextMenu = new Ext.menu.Menu({
			items: [{
				id: this.getId() + "-menu-delete",
				text: "Delete",
				handler: this.cbDeleteBtnHdl,
				scope: this
			},{
				id: this.getId() + "-menu-pause",
				text: "Pause",
				handler: this.cbPauseBtnHdl,
				scope: this
			},{
				id: this.getId() + "-menu-resume",
				text: "Resume",
				handler: this.cbResumeBtnHdl,
				scope: this
			},{
				id: this.getId() + "-menu-queue-top",
				text: "Queue Move Top",
				handler: this.cbQueueMoveMenuHdl,
				scope: this,
				action: 'top'
			},{
				id: this.getId() + "-menu-queue-up",
				text: "Queue Move Up",
				handler: this.cbQueueMoveMenuHdl,
				scope: this,
				action: 'up'
			},{
				id: this.getId() + "-menu-queue-down",
				text: "Queue Move Down",
				handler: this.cbQueueMoveMenuHdl,
				scope: this,
				action: 'down'
			},{
				id: this.getId() + "-menu-queue-bottom",
				text: "Queue Move Bottom",
				handler: this.cbQueueMoveMenuHdl,
				scope: this,
				action: 'bottom'
			}]
		});

		OMV.Ajax.request(this.enableReload, this, "TransmissionBT", "getStatus");

		OMV.Module.Services.TransmissionBT.Manage.TorrentListGrid.superclass.initComponent.apply(this,
		  arguments);
	},

	enableReload : function(id, response, error) {
		if (error !== null) {
			OMV.MessageBox.error(null, error);
		} else {
			if (!response[0].running) {
				this.autoReload = false;

				if (!Ext.isEmpty(this.reloadTask)) {
					Ext.TaskMgr.stop(this.reloadTask);
					delete this.reloadTask;
				}
			} else {
				this.autoReload = true;
				if (Ext.isEmpty(this.reloadTask)) {
					this.reloadTask = Ext.TaskMgr.start({
						run: this.doReload,
						scope: this,
						interval: this.reloadInterval
					});
				}
				this.on("beforedestroy", function(c) {
					if (!Ext.isEmpty(this.reloadTask)) {
						Ext.TaskMgr.stop(this.reloadTask);
						delete this.reloadTask;
					}
				}, this);
				this.store.reload();
				this.toggleButtons();
				this.toggleContextMenu();
			}
		}
	},

	getContextMenu : function(){
		return this.contextMenu;
	},

	initToolbar : function() {
		var tbar = OMV.Module.Services.TransmissionBT.Manage.TorrentListGrid.superclass.initToolbar.apply(
		  this);
		tbar.insert(0, {
			id: this.getId() + "-reload",
			xtype: "button",
			text: "Reload",
			icon: "images/reload.png",
			handler: this.cbReloadBtnHdl,
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
			id: this.getId() + "-add-url",
			xtype: "button",
			text: "Add URL",
			icon: "images/add.png",
			handler: this.cbAddURLBtnHdl,
			scope: this
		});
		tbar.insert(3, {
			id: this.getId() + "-delete",
			xtype: "button",
			text: "Delete",
			icon: "images/delete.png",
			handler: this.cbDeleteBtnHdl,
			scope: this,
			disabled: true
		});
		tbar.insert(4, {
			id: this.getId() + "-pause",
			xtype: "button",
			text: "Pause",
			icon: "images/transmissionbt_pause.png",
			handler: this.cbPauseBtnHdl,
			scope: this,
			disabled: true
		});
		tbar.insert(5, {
			id: this.getId() + "-resume",
			xtype: "button",
			text: "Resume",
			icon: "images/transmissionbt_resume.png",
			handler: this.cbResumeBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	listeners: {
		'rowcontextmenu' : function(grid, index, event) {
			this.showContextMenu(grid, index, event);
		}
	},

	showContextMenu : function(grid, index, event) {
		event.stopEvent();
		var records = new Array(grid.getStore().getAt(index));
		var sm = this.getSelectionModel();
		sm.selectRow(index);
		this.contextMenu.showAt(event.xy);
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Services.TransmissionBT.Manage.TorrentListGrid.superclass.cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons
		this.toggleButtons();
		this.toggleContextMenu();
	},

	toggleButtons : function()
	{
		var sm = this.getSelectionModel();
		var records = sm.getSelections();

		var tbarDeleteCtrl = this.getTopToolbar().findById(this.getId() + "-delete");
		var tbarPauseCtrl = this.getTopToolbar().findById(this.getId() + "-pause");
		var tbarResumeCtrl = this.getTopToolbar().findById(this.getId() + "-resume");

		if (records.length <= 0) {
			tbarDeleteCtrl.disable();
			tbarPauseCtrl.disable();
			tbarResumeCtrl.disable();
		} else if (records.length == 1) {
			var record = records.pop();
			var status = parseInt(record.get("status"));
			switch (status)
			{
				case 0: /* Torrent is stopped */
					tbarDeleteCtrl.enable();
					tbarPauseCtrl.disable();
					tbarResumeCtrl.enable();
					break;
				case 1: /* Queued to check files */
					tbarDeleteCtrl.enable();
					tbarPauseCtrl.enable();
					tbarResumeCtrl.disable();
					break;
				case 2: /* Checking files */
					tbarDeleteCtrl.enable();
					tbarPauseCtrl.enable();
					tbarResumeCtrl.disable();
					break;
				case 3: /* Queued to download */
					tbarDeleteCtrl.enable();
					tbarPauseCtrl.enable();
					tbarResumeCtrl.disable();
					break;
				case 4: /* Downloading */
					tbarDeleteCtrl.enable();
					tbarPauseCtrl.enable();
					tbarResumeCtrl.disable();
					break;
				case 5: /* Queued to seed */
					tbarDeleteCtrl.enable();
					tbarPauseCtrl.enable();
					tbarResumeCtrl.disable();
					break;
				case 6: /* Seeding */
					tbarDeleteCtrl.enable();
					tbarPauseCtrl.enable();
					tbarResumeCtrl.disable();
					break;
				default:
					break;
			}
		} else {
			tbarDeleteCtrl.enable();
			tbarPauseCtrl.enable();
			tbarResumeCtrl.enable();
		}
	},

	toggleContextMenu : function()
	{
		var sm = this.getSelectionModel();
		var records = sm.getSelections();

		var menuItemDeleteCtrl = this.getContextMenu().findById(this.getId() + "-menu-delete");
		var menuItemPauseCtrl = this.getContextMenu().findById(this.getId() + "-menu-pause");
		var menuItemResumeCtrl = this.getContextMenu().findById(this.getId() + "-menu-resume");

		/* Queue Menu Items */
		var menuItemQueueTopCtrl = this.getContextMenu().findById(this.getId() + "-menu-queue-top");
		var menuItemQueueUpCtrl = this.getContextMenu().findById(this.getId() + "-menu-queue-up");
		var menuItemQueueDownCtrl = this.getContextMenu().findById(this.getId() + "-menu-queue-down");
		var menuItemQueueBottomCtrl = this.getContextMenu().findById(this.getId() + "-menu-queue-bottom");

		if (records.length <= 0) {
			menuItemDeleteCtrl.disable();
			menuItemPauseCtrl.disable();
			menuItemResumeCtrl.disable();

			/* Queue Menu Items */
			menuItemQueueTopCtrl.disable();
			menuItemQueueUpCtrl.disable();
			menuItemQueueDownCtrl.disable();
			menuItemQueueBottomCtrl.disable();
		} else if (records.length == 1) {

			/* Queue Menu Items */
			menuItemQueueTopCtrl.enable();
			menuItemQueueUpCtrl.enable();
			menuItemQueueDownCtrl.enable();
			menuItemQueueBottomCtrl.enable();

			var record = records.pop();
			var status = parseInt(record.get("status"));
			switch (status)
			{
				case 0: /* Torrent is stopped */
					menuItemDeleteCtrl.enable();
					menuItemPauseCtrl.disable();
					menuItemResumeCtrl.enable();
					break;
				case 1: /* Queued to check files */
					menuItemDeleteCtrl.enable();
					menuItemPauseCtrl.enable();
					menuItemResumeCtrl.disable();
					break;
				case 2: /* Checking files */
					menuItemDeleteCtrl.enable();
					menuItemPauseCtrl.enable();
					menuItemResumeCtrl.disable();
					break;
				case 3: /* Queued to download */
					menuItemDeleteCtrl.enable();
					menuItemPauseCtrl.enable();
					menuItemResumeCtrl.disable();
					break;
				case 4: /* Downloading */
					menuItemDeleteCtrl.enable();
					menuItemPauseCtrl.enable();
					menuItemResumeCtrl.disable();
					break;
				case 5: /* Queued to seed */
					menuItemDeleteCtrl.enable();
					menuItemPauseCtrl.enable();
					menuItemResumeCtrl.disable();
					break;
				case 6: /* Seeding */
					menuItemDeleteCtrl.enable();
					menuItemPauseCtrl.enable();
					menuItemResumeCtrl.disable();
					break;
				default:
					break;
			}
		} else {
			menuItemDeleteCtrl.enable();
			menuItemPauseCtrl.enable();
			menuItemResumeCtrl.enable();

			/* Queue Menu Items */
			menuItemQueueTopCtrl.enable();
			menuItemQueueUpCtrl.enable();
			menuItemQueueDownCtrl.enable();
			menuItemQueueBottomCtrl.enable();
		}
	},

	/**
	 * @method doReload
	 * Reload the grid content.
	 */
	doReload : function() {
		OMV.Ajax.request(this.enableReload, this, "TransmissionBT", "getStatus");
	},

	cbReloadBtnHdl : function() {
		this.doReload();
	},

	cbUploadBtnHdl : function() {
		var wnd = new OMV.TransmissionBT.UploadDialog({
			title: "Upload torrent",
			service: "TransmissionBT",
			method: "upload",
			listeners: {
				success: function(wnd, response) {
					// The upload was successful, now resynchronize the
					// package index files from their sources.
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	/* ADD URL HANDLER */
	cbAddURLBtnHdl : function() {
		var wnd = new OMV.TransmissionBT.AddURLDialog({
			title: "Add Torrent by URL",
			listeners: {
				success: function(wnd, url, startDownload) {
					this.doAddURL(url, startDownload);
				},
				scope: this
			}
		});
		wnd.show();
	},
	doAddURL : function(url, startDownload) {
		OMV.Ajax.request(this.cbAddURLHdl, this, "TransmissionBT", "addUrl", [{ url: String(url), start_download: Boolean(startDownload) }] );
	},
	cbAddURLHdl : function(id, response, error) {
		if (error !== null) {
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			OMV.MessageBox.hide();
			this.doReload();
		}
	},
	/* /ADD URL HANDLER */

	/* DELETION HANDLER */
	cbDeleteBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var records = selModel.getSelections();
		var wnd = new OMV.TransmissionBT.DeleteDialog({
			title: "Upload torrent",
			service: "TransmissionBT",
			method: "upload",
			listeners: {
				success: function(wnd, delete_local_data) {
					this.startDelete(selModel, records, delete_local_data);
					this.cbReloadBtnHdl();
				},
				scope: this
			}
		});
		wnd.show();
	},
	startDelete: function(model, records, delete_local_data) {
		if (records.length <= 0)
			return;
		// Store selected records in a local variable
		this.deleteActionInfo = {
			records: records,
			count: records.length
		};
		// Get first record to be deleted
		var record = this.deleteActionInfo.records.pop();
		// Display progress dialog
		OMV.MessageBox.progress("", this.deleteWaitMsg, "");
		this.updateDeleteProgress();
		// Execute deletion function
		this.doDelete(record, delete_local_data);
	},
	doDelete : function(record, delete_local_data) {
		OMV.Ajax.request(this.cbDeleteHdl, this, "TransmissionBT", "delete", [{ id:  record.get("id"), deleteLocalData: delete_local_data }] );


	},
	updateDeleteProgress : function() {
		// Calculate percentage
		var p = (this.deleteActionInfo.count - this.deleteActionInfo.records.length) /
		  this.deleteActionInfo.count;
		// Create message text
		var text = Math.round(100 * p) + "% completed ...";
		// Update progress dialog
		OMV.MessageBox.updateProgress(p, text);
	},
	cbDeleteHdl : function(id, response, error) {
		if (error !== null) {
			// Remove temporary local variables
			delete this.deleteActionInfo;
			// Hide progress dialog
			OMV.MessageBox.hide();
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			if (this.deleteActionInfo.records.length > 0) {
				var record = this.deleteActionInfo.records.pop();
				// Update progress dialog
				this.updateDeleteProgress();
				// Execute deletion function
				this.doDelete(record);
			} else {
				// Remove temporary local variables
				delete this.deleteActionInfo;
				// Update and hide progress dialog
				OMV.MessageBox.updateProgress(1, "100% completed ...");
				OMV.MessageBox.hide();
				this.doReload();
			}
		}
	},
	/* /DELETION HANDLER */

	/* RESUME HANDLER */
	cbResumeBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var records = selModel.getSelections();

		this.startResume(selModel, records);
	},
	startResume: function(model, records) {
		if (records.length <= 0)
			return;
		// Store selected records in a local variable
		this.resumeActionInfo = {
			records: records,
			count: records.length
		};
		// Get first record to be deleted
		var record = this.resumeActionInfo.records.pop();
		// Display progress dialog
		OMV.MessageBox.progress("", this.resumeWaitMsg, "");
		this.updateResumeProgress();
		// Execute deletion function
		this.doResume(record);
	},
	doResume : function(record) {
		OMV.Ajax.request(this.cbResumeHdl, this, "TransmissionBT", "resume", [ record.get("id") ]);
		//cbResumeHdl(null, null, null);
	},
	updateResumeProgress : function() {
		// Calculate percentage
		var p = (this.resumeActionInfo.count - this.resumeActionInfo.records.length) /
		  this.resumeActionInfo.count;
		// Create message text
		var text = Math.round(100 * p) + "% completed ...";
		// Update progress dialog
		OMV.MessageBox.updateProgress(p, text);
	},
	cbResumeHdl : function(id, response, error) {
		if (error !== null) {
			// Remove temporary local variables
			delete this.resumeActionInfo;
			// Hide progress dialog
			OMV.MessageBox.hide();
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			if (this.resumeActionInfo.records.length > 0) {
				var record = this.resumeActionInfo.records.pop();
				// Update progress dialog
				this.updateResumeProgress();
				// Execute deletion function
				this.doResume(record);
			} else {
				// Remove temporary local variables
				delete this.resumeActionInfo;
				// Update and hide progress dialog
				OMV.MessageBox.updateProgress(1, "100% completed ...");
				OMV.MessageBox.hide();
				this.doReload();
			}
		}
	},
	/* /RESUME HANDLER */

	/* PAUSE HANDLER */
	cbPauseBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var records = selModel.getSelections();

		this.startPause(selModel, records);
	},
	startPause: function(model, records) {
		if (records.length <= 0)
			return;
		// Store selected records in a local variable
		this.pauseActionInfo = {
			records: records,
			count: records.length
		};
		// Get first record to be deleted
		var record = this.pauseActionInfo.records.pop();
		// Display progress dialog
		OMV.MessageBox.progress("", this.pauseWaitMsg, "");
		this.updatePauseProgress();
		// Execute deletion function
		this.doPause(record);
	},
	doPause : function(record) {
		OMV.Ajax.request(this.cbPauseHdl, this, "TransmissionBT", "pause", [ record.get("id") ]);
	},
	updatePauseProgress : function() {
		// Calculate percentage
		var p = (this.pauseActionInfo.count - this.pauseActionInfo.records.length) /
		  this.pauseActionInfo.count;
		// Create message text
		var text = Math.round(100 * p) + "% completed ...";
		// Update progress dialog
		OMV.MessageBox.updateProgress(p, text);
	},
	cbPauseHdl : function(id, response, error) {
		if (error !== null) {
			// Remove temporary local variables
			delete this.pauseActionInfo;
			// Hide progress dialog
			OMV.MessageBox.hide();
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			if (this.pauseActionInfo.records.length > 0) {
				var record = this.pauseActionInfo.records.pop();
				// Update progress dialog
				this.updatePauseProgress();
				// Execute deletion function
				this.doPause(record);
			} else {
				// Remove temporary local variables
				delete this.pauseActionInfo;
				// Update and hide progress dialog
				OMV.MessageBox.updateProgress(1, "100% completed ...");
				OMV.MessageBox.hide();
				this.doReload();
			}
		}
	},
	/* /PAUSE HANDLER */

	/* QUEUE MOVE HANDLER */
	cbQueueMoveMenuHdl : function(item, event) {
		var selModel = this.getSelectionModel();
		var records = selModel.getSelections();

		this.startQueueMove(selModel, records, item.action);
	},

	startQueueMove: function(model, records, action) {
		if (records.length <= 0)
			return;
		// Store selected records in a local variable
		this.queueMoveActionInfo = {
			records: records,
			count: records.length,
			action: action
		};
		// Get first record to be deleted
		var record = this.queueMoveActionInfo.records.pop();
		// Display progress dialog
		OMV.MessageBox.progress("", this.queueMoveWaitMsg, "");
		this.updateQueueMoveProgress();
		// Execute deletion function
		this.doQueueMove(record, action);
	},
	doQueueMove : function(record, action) {
		OMV.Ajax.request(this.cbQueueMoveHdl, this, "TransmissionBT", "queueMove", [ { id : record.get("id"), action : action } ]);
	},
	updateQueueMoveProgress : function() {
		// Calculate percentage
		var p = (this.queueMoveActionInfo.count - this.queueMoveActionInfo.records.length) /
		  this.queueMoveActionInfo.count;
		// Create message text
		var text = Math.round(100 * p) + "% completed ...";
		// Update progress dialog
		OMV.MessageBox.updateProgress(p, text);
	},
	cbQueueMoveHdl : function(id, response, error) {
		if (error !== null) {
			// Remove temporary local variables
			delete this.queueMoveActionInfo;
			// Hide progress dialog
			OMV.MessageBox.hide();
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			if (this.queueMoveActionInfo.records.length > 0) {
				var record = this.queueMoveActionInfo.records.pop();
				var action = this.queueMoveActionInfo.action;
				// Update progress dialog
				this.updateQueueMoveProgress();
				// Execute deletion function
				this.doQueueMove(record, action);
			} else {
				// Remove temporary local variables
				delete this.queueMoveActionInfo;
				// Update and hide progress dialog
				OMV.MessageBox.updateProgress(1, "100% completed ...");
				OMV.MessageBox.hide();
				this.doReload();
			}
		}
	},
	/* /QUEUE MOVE HANDLER */

	/* RENDERER */
	doneRenderer : function(val, cell, record, row, col, store) {
		var percentage = parseFloat(record.get("percentDone"));
		var totalSize = parseInt(record.get("totalSize"));
		var haveValid = parseInt(record.get("haveValid"));

		if (-1 == percentage) {
			return val;
		}
		var id = Ext.id();
		(function() {
			new Ext.ProgressBar({
				renderTo: id,
				value: percentage,
				text: OMV.Module.Services.TransmissionBT.util.Format.bytesToSize(haveValid) + '/' + OMV.Module.Services.TransmissionBT.util.Format.bytesToSize(totalSize) + ' (' + parseInt(percentage * 100) + '%)'
			});
		}).defer(25);
		return '<div id="' + id + '"></div>';
	},

	statusRenderer : function(val, cell, record, row, col, store) {
		switch (val) {
		case 0:
			val = "Torrent is stopped";
			break;
		case 1:
			val = "Queued to check files";
			break;
		case 2:
			val = "Checking files";
			break;
		case 3:
			val = "Queued to download";
			break;
		case 4:
			val = "Downloading";
			break;
		case 5:
			val = "Queued to seed";
			break;
		case 6:
			val = "Seeding";
			break;
		default:
			val = "Missing Status: " + val;
			break;
		}
		return val;
	},

	etaRenderer : function(val, cell, record, row, col, store) {
		switch (val) {
		case -1:
			val = "Not available";
			break;
		case -2:
			val = "Unknown";
			break;
		default:
			val = OMV.Module.Services.TransmissionBT.util.Format.timeInterval(val);
			break;
		}
		return val;
	},

	peersRenderer : function(val, cell, record, row, col, store) {
		var peersConnected = parseInt(record.get("peersConnected"));
		var peersSendingToUs = parseInt(record.get("peersSendingToUs"));

		val = peersSendingToUs + ' / ' + peersConnected;

		return val;
	},

	rateRenderer : function(val, cell, record, row, col, store) {
		val = OMV.Module.Services.TransmissionBT.util.Format.rate(val);
		return val;
	},

	timestampRenderer: function(val, cell, record, row, col, store) {
		if (val <= 0)
			return;
		var dt = Date.parseDate(val, "U");
		return Ext.util.Format.date(dt, 'Y-m-d H:i:s');
	},

	ratioRenderer: function(val, cell, record, row, col, store) {
		switch (val)
		{
			case -1:
				val = "Not available";
				break;
			case -2:
				val = "Infinite";
				break;
		}
		return val;
	}
	/* /RENDERER */
});