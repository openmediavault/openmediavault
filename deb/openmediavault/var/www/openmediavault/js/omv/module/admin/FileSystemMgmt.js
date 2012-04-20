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
// require("js/omv/grid/EditorGridPanel.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/FormPanelDialog.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Storage");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("storage", "filesystems", {
	text: _("Filesystems"),
	icon: "images/filesystem.png",
	position: 30
});

/**
 * @class OMV.Module.Storage.FileSystemGridPanel
 * @derived OMV.grid.TBarGridPanel
 * Display list of configured filesystems.
 */
OMV.Module.Storage.FileSystemGridPanel = function(config) {
	var initialConfig = {
		hideAdd: true,
		hideEdit: true,
		hidePagingToolbar: false,
		disableLoadMaskOnLoad: true,
		autoReload: true,
		stateId: "efea99a0-95d1-4bc9-8207-d21fe514f069",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Device"),
				sortable: true,
				dataIndex: "devicefile",
				id: "devicefile",
				width: 50,
				renderer: OMV.util.Format.emptyRenderer(),
				scope: this
			},{
				header: _("Label"),
				sortable: true,
				dataIndex: "label",
				id: "label"
			},{
				header: _("Filesystem"),
				sortable: true,
				dataIndex: "type",
				id: "type"
			},{
				header: _("Available"),
				sortable: true,
				dataIndex: "available",
				id: "available",
				width: 50,
				renderer: OMV.util.Format.binaryUnitRenderer()
			},{
				header: _("Used"),
				sortable: true,
				dataIndex: "used",
				id: "used",
				renderer: this.usedRenderer,
				scope: this
			},{
				header: _("Mounted"),
				sortable: true,
				dataIndex: "mounted",
				id: "mounted",
				renderer: OMV.util.Format.booleanRenderer(),
				scope: this
			},{
				header: _("Status"),
				sortable: true,
				dataIndex: "status",
				id: "status",
				renderer: this.statusRenderer,
				scope: this
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.FileSystemGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.FileSystemGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "FileSystemMgmt",
				"method": "getList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "devicefile" },
					{ name: "label" },
					{ name: "type" },
					{ name: "used" },
					{ name: "available" },
					{ name: "status" },
					{ name: "percentage" },
					{ name: "mounted" },
					{ name: "mountpoint" },
					{ name: "_used" }
    			]
			})
		});
		OMV.Module.Storage.FileSystemGridPanel.superclass.initComponent.
		  apply(this, arguments);
		// Register event handler
		// Reselect previous selected rows after the store has been
		// reloaded, e.g. to make sure toolbar is updated depending on
		// the latest row record values.
		this.getSelectionModel().previousSelections = [];
		this.store.on("beforeload", function(store, options) {
			  var sm = this.getSelectionModel();
			  var records = sm.getSelections();
			  sm.previousSelections = [];
			  Ext.each(records, function(record, index) {
				  sm.previousSelections.push(record.get("uuid"));
			  }, this);
		  }, this);
		this.store.on("load", function(store, records, options) {
			  var sm = this.getSelectionModel();
			  var rows = [];
			  if (Ext.isDefined(sm.previousSelections)) {
				  for (var i = 0; i < sm.previousSelections.length; i++) {
					  var index = store.findExact("uuid",
						sm.previousSelections[i]);
					  if (index !== -1) {
						  rows.push(index);
					  }
				  }
			  }
			  if (rows.length > 0) {
				  sm.selectRows(rows);
			  }
		  }, this);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Storage.FileSystemGridPanel.superclass.
		  initToolbar.apply(this);
		// Add 'Create' button to top toolbar
		tbar.insert(1, {
			id: this.getId() + "-create",
			xtype: "button",
			text: _("Create"),
			icon: "images/filesystem.png",
			handler: this.cbCreateBtnHdl,
			scope: this,
			disabled: false
		});
		// Add 'Resize' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-resize",
			xtype: "button",
			text: _("Resize"),
			icon: "images/filesystem-resize.png",
			handler: this.cbResizeBtnHdl,
			scope: this,
			disabled: true
		});
		// Add 'Quota' button to top toolbar
		tbar.insert(3, {
			id: this.getId() + "-quota",
			xtype: "button",
			text: _("Quota"),
			icon: "images/filesystem-quota.png",
			handler: this.cbQuotaBtnHdl,
			scope: this,
			disabled: true
		});
		// Add 'Mount' button to top toolbar
		tbar.insert(4, {
			id: this.getId() + "-mount",
			xtype: "button",
			text: _("Mount"),
			icon: "images/filesystem-mount.png",
			handler: this.cbMountBtnHdl,
			scope: this,
			disabled: true
		});
		// Add 'Unmount' button to top toolbar
		tbar.insert(5, {
			id: this.getId() + "-unmount",
			xtype: "button",
			text: _("Unmount"),
			icon: "images/filesystem-umount.png",
			handler: this.cbUnmountBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Storage.FileSystemGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons.
		var tbarBtnName = [ "resize", "quota", "delete", "mount", "unmount" ];
		var tbarBtnDisabled = {
			"resize": true,
			"quota": true,
			"delete": false,
			"mount": true,
			"unmount": true
		};
		var records = model.getSelections();
		if (records.length <= 0) {
			tbarBtnDisabled["resize"] = true;
			tbarBtnDisabled["quota"] = true;
			tbarBtnDisabled["delete"] = true;
			tbarBtnDisabled["mount"] = true;
			tbarBtnDisabled["unmount"] = true;
		} else if (records.length == 1) {
			tbarBtnDisabled["resize"] = false;
			tbarBtnDisabled["quota"] = false;
			tbarBtnDisabled["delete"] = false;
			tbarBtnDisabled["mount"] = true;
			tbarBtnDisabled["unmount"] = true;
			// Disable the 'Resize' button if filesystem is not supported.
			if ([ "ext","ext2","ext3","ext4","xfs","jfs" ].indexOf(
			  records[0].get("type")) == -1) {
				tbarBtnDisabled["resize"] = true;
			}
			// Disable the 'Quota' button if the filesystem does not have
			// a mount point.
			if (Ext.isEmpty(records[0].get("mountpoint"))) {
				tbarBtnDisabled["quota"] = true;
			}
			// Disable/enable the mount/unmount buttons depending on whether
			// the selected filesystem is mounted.
			if (true === records[0].get("mounted")) {
				tbarBtnDisabled["unmount"] = false;
			} else {
				tbarBtnDisabled["mount"] = false;
			}
			// If the filesystem is in usage, then also disable the unmount
			// button.
			if (true === records[0].get("_used")) {
				tbarBtnDisabled["unmount"] = true;
			}
			// Finally disable buttons if a selected filesystem is
			// initialized at the moment.
			if ([ 2,3 ].indexOf(records[0].get("status")) !== -1) {
				tbarBtnDisabled["resize"] = true;
				tbarBtnDisabled["quota"] = true;
				tbarBtnDisabled["delete"] = true;
				tbarBtnDisabled["mount"] = true;
			}
		} else {
			tbarBtnDisabled["resize"] = true;
			tbarBtnDisabled["quota"] = true;
			tbarBtnDisabled["delete"] = false;
			tbarBtnDisabled["mount"] = true;
			tbarBtnDisabled["unmount"] = true;
			// Disable button if one of the selected filesystems is
			// initialized at the moment.
			for (var i = 0; i < records.length; i++) {
				if (2 == records[i].get("status")) {
					tbarBtnDisabled["delete"] = true;
				}
			}
		}
		// Disable 'Delete' button if a selected filesystem is in usage
		// or readonly.
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

	cbCreateBtnHdl : function() {
		var wnd = new OMV.Module.Storage.FileSystemCreateDialog({
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbResizeBtnHdl : function() {
		var msg = _("Do you really want to resize the selected filesystem? You have to do that after a RAID has been grown for example.");
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: msg,
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if (answer == "no")
					return;
				var selModel = this.getSelectionModel();
				var record = selModel.getSelected();
				// Execute RPC
				OMV.Ajax.request(function(id, response, error) {
					  if (error === null) {
						  this.doReload();
					  } else {
						  OMV.MessageBox.error(null, error);
					  }
				  }, this, "FileSystemMgmt", "resize",
				  { "id": record.get("uuid") });
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	},

	cbQuotaBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.QuotaPropertyDialog({
			uuid: record.get("uuid")
		});
		wnd.show();
	},

	cbMountBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		OMV.Ajax.request(function(id, response, error) {
			if (error === null) {
				this.doReload();
			} else {
				OMV.MessageBox.error(null, error);
			}
		}, this, "FileSystemMgmt", "mount", { "id": record.get("uuid") });
	},

	cbUnmountBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		// Prefer the filesystem UUID, but in some cases a filesystem does not
		// have a UUID, then use the devicefile instead.
		var id = record.get("uuid");
		if (Ext.isEmpty(id))
			id = record.get("devicefile");
		OMV.Ajax.request(function(id, response, error) {
			if (error === null) {
				this.doReload();
			} else {
				OMV.MessageBox.error(null, error);
			}
		}, this, "FileSystemMgmt", "umount", { "id": id });
	},

	doDeletion : function(record) {
		// Prefer the filesystem UUID, but in some cases a filesystem does not
		// have a UUID, then use the devicefile instead.
		var id = record.get("uuid");
		if (Ext.isEmpty(id))
			id = record.get("devicefile");
		OMV.Ajax.request(this.cbDeletionHdl, this, "FileSystemMgmt",
		  "delete", { "id": id });
	},

	usedRenderer : function(val, cell, record, row, col, store) {
		var percentage = parseInt(record.get("percentage"));
		if (-1 == percentage) {
			return val;
		}
		var id = Ext.id();
		(function(){
			new Ext.ProgressBar({
				renderTo: id,
				value: percentage / 100,
				text: val
			});
		}).defer(25)
		return '<div id="' + id + '"></div>';
	},

	statusRenderer : function(val, cell, record, row, col, store) {
		switch (val) {
		case 1:
			val = "Online";
			break;
		case 2:
			val = "<img border='0' src='images/wait.gif'> Initializing";
			break;
		default:
			val = "Missing";
			break;
		}
		return val;
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "filesystems", {
	cls: OMV.Module.Storage.FileSystemGridPanel
});

/**
 * @class OMV.Module.Storage.FileSystemCreateDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Storage.FileSystemCreateDialog = function(config) {
	var initialConfig = {
		rpcService: "FileSystemMgmt",
		rpcSetMethod: "create",
		title: _("Create filesystem"),
		autoHeight: true,
		hideReset: true,
		width: 500
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.FileSystemCreateDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.FileSystemCreateDialog, OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "devicefile",
			hiddenName: "devicefile",
			fieldLabel: _("Device"),
			emptyText: _("Select a device ..."),
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "FileSystemMgmt",
					"method": "getCandidates"
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile" },
						{ name: "description" }
					]
				})
			}),
			displayField: "description",
			valueField: "devicefile",
			allowBlank: false,
			editable: false,
			triggerAction: "all"
		},{
			xtype: "textfield",
			name: "label",
			fieldLabel: _("Label"),
			allowBlank: true,
			maxLength: 16,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The volume label for the filesystem."),
			vtype: "fslabel"
		},{
			xtype: "combo",
			name: "type",
			hiddenName: "type",
			fieldLabel: _("Filesystem"),
			emptyText: _("Select a filesystem ..."),
			mode: "local",
			store: [
				[ "ext3","EXT3" ],
				[ "ext4","EXT4" ],
				[ "xfs","XFS" ],
				[ "jfs","JFS" ]
			],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "ext4"
		}];
	},

	doSubmit : function() {
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to format this device? All data on it will be deleted. Please note that the filesystem creation may take some time."),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if (answer === "no") {
					return;
				}
				OMV.Module.Storage.FileSystemCreateDialog.
				  superclass.doSubmit.apply(this, arguments);
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.Module.Storage.QuotaPropertyDialog
 * @config uuid The UUID of the filesystem to process.
 * @config readOnly TRUE to set the dialog to read-only.
 * Defaults to FALSE.
 */
OMV.Module.Storage.QuotaPropertyDialog = function(config) {
	var initialConfig = {
		title: _("Edit quota"),
		width: 500,
		height: 305,
		layout: "fit",
		modal: true,
		border: false,
		buttonAlign: "center",
		readOnly: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.QuotaPropertyDialog.superclass.constructor.
	  call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires after the submission has been finished successful.
		 */
		"submit"
	);
};
Ext.extend(OMV.Module.Storage.QuotaPropertyDialog, Ext.Window, {
	initComponent : function() {
		this.grid = new OMV.grid.EditorGridPanel({
			bodyCssClass: "x-grid3-without-dirty-cell",
			stateId: "24f018e6-8de6-41e1-b6c4-db0edd49a73b",
			selModel: new Ext.grid.RowSelectionModel(),
			colModel: new Ext.grid.ColumnModel({
				columns: [{
					header: _("Type"),
					sortable: true,
					dataIndex: "type",
					id: "type",
					align: "center",
					width: 50,
					renderer: function(val, cell, record, row, col, store) {
						switch (val) {
						case "user":
							val = "<img border='0' src='images/user.png'>";
							break;
						case "group":
							val = "<img border='0' src='images/group.png'>";
							break;
						}
						return val;
					}
				},{
					header: _("Name"),
					sortable: true,
					dataIndex: "name",
					id: "name"
				},{
					header: _("Used capacity"),
					sortable: true,
					dataIndex: "bused",
					id: "bused",
					renderer: function(val, cell, record, row, col, store) {
						if (val <= 0) {
							val = "--";
						}
						return val;
					}
				},{
					header: _("Quota"),
					sortable: true,
					dataIndex: "bhardlimit",
					id: "bhardlimit",
					align: "right",
					editor: new Ext.form.NumberField({
						allowBlank: true,
						allowDecimals: true,
						allowNegative: false
					}),
					renderer: function(val, cell, record, row, col, store) {
						if ((val <= 0) || Ext.isEmpty(val)) {
							val = "--";
						}
						return val;
					}
				},{
					header: _("Unit"),
					sortable: true,
					dataIndex: "bunit",
					id: "bunit",
					width: 60,
					editor: new Ext.form.ComboBox({
						mode: "local",
						store: [ "KiB", "MiB", "GiB", "TiB" ],
						allowBlank: false,
						triggerAction: "all",
						editable: false,
						forceSelection: true
					})
				}]
			}),
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "Quota",
					"method": "get",
					"extraParams": { "uuid": this.uuid },
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "name",
					fields: [
						{ name: "type" },
						{ name: "name" },
						{ name: "bused" },
						{ name: "bhardlimit" },
						{ name: "bunit" }
					]
				})
			}),
			listeners: {
				scope: this,
				beforeedit: function(e) {
					switch (e.field) {
					case "bhardlimit":
						// Display a empty number field if value is 0.
						if (e.value == 0) {
							e.record.set("bhardlimit", "");
						}
						break;
					}
				},
				validateedit: function(e) {
					var bunit = e.record.get("bunit");
					var bhardlimit = e.record.get("bhardlimit");
					switch (e.field) {
					case "bhardlimit":
						bhardlimit = !Ext.isEmpty(e.value) ? e.value : 0;
						break;
					case "bunit":
						bunit = e.value;
						break;
					}
					// Validate quota with max. possible value (4TiB).
					bhardlimit = bhardlimit.binaryConvert(bunit, "B");
					if (bhardlimit > 4 * Math.pow(2, 40)) {
						OMV.MessageBox.failure(null, _("The specified quota exceeds the max. possible value of 4TiB."));
						return false;
					}
				}
			}
		});
		Ext.apply(this, {
			buttons: [{
				text: _("OK"),
				handler: this.cbOkBtnHdl,
				scope: this,
				disabled: this.readOnly
			},{
				text: _("Cancel"),
				handler: this.cbCancelBtnHdl,
				scope: this
			}],
			items: [ this.grid ]
		});
		OMV.Module.Storage.QuotaPropertyDialog.superclass.
		  initComponent.apply(this, arguments);
	},

	/**
	 * @method cbOkBtnHdl
	 * Method that is called when the 'OK' button is pressed.
	 */
	cbOkBtnHdl : function() {
		this.doSubmit();
	},

	/**
	 * @method cbCancelBtnHdl
	 * Method that is called when the 'Cancel' button is pressed.
	 */
	cbCancelBtnHdl : function() {
		this.close();
	},

	/**
	 * @method doLoad
	 * Load the grid content.
	 */
	doLoad : function() {
		this.grid.store.load();
	},

	doSubmit : function() {
		// Display waiting dialog
		OMV.MessageBox.wait(null, _("Saving ..."));
		// Prepare RPC content
		var records = this.grid.store.getRange();
		var values = {
			uuid: this.uuid,
			quota: []
		};
		for (var i = 0; i < records.length; i++) {
			var bhardlimit = records[i].get("bhardlimit");
			// Only submit useful settings.
			if ((bhardlimit == 0) || Ext.isEmpty(bhardlimit))
				continue;
			values.quota.push({
				"type": records[i].get("type"),
				"name": records[i].get("name"),
				"bhardlimit": bhardlimit,
				"bunit": records[i].get("bunit")
			});
		}
		OMV.Ajax.request(this.cbSubmitHdl, this, "Quota", "set", values);
	},

	cbSubmitHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			this.fireEvent("submit", this);
			this.close();
		} else {
			OMV.MessageBox.error(null, error);
		}
	}
});
