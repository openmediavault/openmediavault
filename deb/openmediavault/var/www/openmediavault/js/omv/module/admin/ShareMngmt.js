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
// require("js/omv/grid/GridPanel.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Privileges");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("privileges", "sharedfolder", {
	text: "Shared Folders",
	icon: "images/folder-remote.png",
	position: 30
});

/**
 * @class OMV.Module.Privileges.SharedFolderGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Privileges.SharedFolderGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "9ab0d7f9-73e0-4815-8960-84157d4b85e5",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			},{
				header: "Volume",
				sortable: true,
				dataIndex: "volume",
				id: "volume"
			},{
				header: "Used",
				sortable: true,
				dataIndex: "_used",
				id: "_used",
				renderer: OMV.util.Format.booleanRenderer(),
				scope: this
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.SharedFolderGridPanel.superclass.constructor.
	  call(this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.SharedFolderGridPanel,
  OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("ShareMgmt", "getList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "name" },
					{ name: "comment" },
					{ name: "volume" },
					{ name: "_used" }
    			]
			})
		});
		OMV.Module.Privileges.SharedFolderGridPanel.superclass.
		  initComponent.apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Privileges.SharedFolderGridPanel.superclass.
		  initToolbar.apply(this);
		tbar.insert(2, {
			id: this.getId() + "-privileges",
			xtype: "button",
			text: "Privileges",
			icon: "images/privileges.gif",
			handler: this.cbPrivilegesBtnHdl.createDelegate(this),
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Privileges.SharedFolderGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons
		var records = model.getSelections();
		var tbarPrivilegesCtrl = this.getTopToolbar().findById(
			this.getId() + "-privileges");
		if (records.length <= 0) {
			tbarPrivilegesCtrl.disable();
		} else if (records.length == 1) {
			tbarPrivilegesCtrl.enable();
		} else {
			tbarPrivilegesCtrl.disable();
		}
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Privileges.SharedFolderPropertyDialog({
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Privileges.SharedFolderPropertyDialog({
			uuid: record.get("uuid"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbPrivilegesBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Privileges.PrivilegesPropertyDialog({
			uuid: record.get("uuid")
		});
		wnd.show();
	},

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "ShareMgmt",
		  "delete", [ record.get("uuid"), false ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("privileges", "sharedfolder", {
	cls: OMV.Module.Privileges.SharedFolderGridPanel
});

/**
 * @class OMV.Module.Privileges.SharedFolderPropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Privileges.SharedFolderPropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "ShareMgmt",
		rpcGetMethod: "get",
		rpcSetMethod: "set",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " shared folder",
		width: 550,
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.SharedFolderPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.SharedFolderPropertyDialog,
  OMV.CfgObjectDialog, {
	initComponent : function() {
		OMV.Module.Privileges.SharedFolderPropertyDialog.superclass.
		  initComponent.apply(this, arguments);
		// Register event handler
		this.on("load", this._updateFormFields, this);
	},

	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: "Name",
			allowBlank: false,
			vtype: "sharename"
		},{
			xtype: "combo",
			name: "mntentref",
			hiddenName: "mntentref",
			fieldLabel: "Volume",
			emptyText: "Select a volume ...",
			allowBlank: false,
			allowNone: false,
			editable: false,
			triggerAction: "all",
			displayField: "description",
			valueField: "uuid",
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy("ShareMgmt",
				  "getCandidates"),
				reader: new Ext.data.JsonReader({
					idProperty: "uuid",
					fields: [
						{ name: "uuid" },
						{ name: "description" }
					]
				})
			})
		},{
			xtype: "textfield",
			name: "dirpath",
			fieldLabel: "Directory",
			readOnly: true,
			hidden: (this.uuid == OMV.UUID_UNDEFINED) ? true : false,
			submitValue: false
		},{
			xtype: "combo",
			name: "umask",
			hiddenName: "umask",
			fieldLabel: "Permissions",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "700","Administrator: read/write, Users: no access" ],
					[ "755","Administrator: read/write, Users: read-only" ],
					[ "777","Everyone: read/write" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "777",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The privileges can be defined more detailed later."
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		if (this.uuid == OMV.UUID_UNDEFINED)
			return;
		var fields = [ "name", "mntentref" ];
		for (var i = 0; i < fields.length; i++) {
			var field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.setReadOnly(true);
			}
		}
	}
});

/**
 * @class OMV.Module.Privileges.PrivilegesPropertyDialog
 * @config uuid The UUID of the shared folder to process.
 * @config readOnly TRUE to set the dialog to read-only.
 * Defaults to FALSE.
 */
OMV.Module.Privileges.PrivilegesPropertyDialog = function(config) {
	var initialConfig = {
		title: "Edit shared folder privileges",
		width: 500,
		height: 300,
		layout: "fit",
		modal: true,
		border: true,
		buttonAlign: "center",
		readOnly: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.PrivilegesPropertyDialog.superclass.
	  constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires after the submission has been finished successful.
		 */
		"submit"
	);
};
Ext.extend(OMV.Module.Privileges.PrivilegesPropertyDialog, Ext.Window, {
	initComponent : function() {
		this.grid = new OMV.grid.GridPanel({
			bodyCssClass: "x-grid3-without-dirty-cell",
			stateId: "474eacf4-cadb-4ae4-b545-4f7f47d7aed9",
			colModel: new Ext.grid.ColumnModel({
				columns: [{
					header: "Type",
					sortable: true,
					dataIndex: "type",
					id: "type",
					align: "center",
					width: 50,
					renderer: this.typeRenderer,
					scope: this
				},{
					header: "Name",
					sortable: true,
					dataIndex: "name",
					id: "name"
				},{
					header: "Read/Write",
					dataIndex: "writeable",
					id: "writeable",
					align: "center",
					renderer: this.checkBoxRenderer,
					scope: this
				},{
					header: "Read-only",
					dataIndex: "readonly",
					id: "readonly",
					align: "center",
					renderer: this.checkBoxRenderer,
					scope: this
				},{
					header: "No access",
					dataIndex: "deny",
					id: "deny",
					align: "center",
					renderer: this.checkBoxRenderer,
					scope: this
				}]
			}),
			store: new OMV.data.Store({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy("ShareMgmt",
				  "getPrivileges", this.uuid, false),
				reader: new Ext.data.JsonReader({
					idProperty: "uuid",
					fields: [
						{ name: "type" },
						{ name: "uuid" },
						{ name: "name" },
						{ name: "perms" }
					]
				}),
				listeners: {
					"load": function(store, records, options) {
						records.each(function(record) {
							// Set default values
							record.data.deny = false;
							record.data.readonly = false;
							record.data.writeable = false;
							// Update values depending on the permissions
							switch (record.get("perms")) {
							case 0:
								record.data.deny = true;
								break;
							case 5:
								record.data.readonly = true;
								break;
							case 7:
								record.data.writeable = true;
								break;
							}
							record.commit();
						}, this);
					},
					scope: this
				}
			})
		});
		Ext.apply(this, {
			buttons: [{
				text: "OK",
				handler: this.cbOkBtnHdl,
				scope: this,
				disabled: this.readOnly
			},{
				text: "Cancel",
				handler: this.cbCancelBtnHdl,
				scope: this
			}],
			items: [ this.grid ]
		});
		OMV.Module.Privileges.PrivilegesPropertyDialog.superclass.
		  initComponent.apply(this, arguments);
		// Register event handler
		this.grid.on("cellclick", this.onCellClick, this);
	},

	/**
	 * @method cbOkBtnHdl
	 * Method that is called when the 'OK' button is pressed.
	 */
	cbOkBtnHdl : function() {
		// Quit immediatelly if the permissions have not been modified.
		if (!this.grid.isDirty()) {
			this.close();
			return;
		}
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
		OMV.MessageBox.wait(null, "Saving ...");
		// Prepare RPC content
		var records = this.grid.store.getRange();
		var values = {
			uuid: this.uuid,
			privileges: []
		};
		for (var i = 0; i < records.length; i++) {
			var record = records[i];
			if ((true === record.get("deny")) ||
			  (true === record.get("readonly")) ||
			  (true === record.get("writeable"))) {
				var perms = 0;
				if (true === record.get("readonly"))
					perms = 5;
				else if (true === record.get("writeable"))
					perms = 7;
				values.privileges.push({
					type: record.get("type"),
					uuid: record.get("uuid"),
					perms: perms
				});
			}
		}
		OMV.Ajax.request(this.cbSubmitHdl, this, "ShareMgmt",
		  "setPrivileges", [ values ]);
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
	},

	/**
	 * Handle grid cell clicks. Only process columns with a checkbox.
	 */
	onCellClick : function(grid, rowIndex, columnIndex, e) {
		if (this.readOnly)
			return;
		var record = this.grid.store.getAt(rowIndex);
		var dataIndex = this.grid.getColumnModel().getDataIndex(columnIndex);
		var dataIndices = [ "readonly", "writeable", "deny" ];
		if (-1 !== dataIndices.indexOf(dataIndex)) {
			// Clear all selections
			for (var i = 0; i < dataIndices.length; i++) {
				// Skip current clicked record field, otherwise unselection
				// of cells will not work
				if (dataIndices[i] === dataIndex)
					continue;
				// Set to 'false' per default
				record.set(dataIndices[i], false);
			}
			// Set new selection
			record.set(dataIndex, !record.get(dataIndex));
		}
	},

	/**
	 * Render a user/group icon in the given grid cell.
	 */
	typeRenderer : function(val, cell, record, row, col, store) {
		switch (val) {
		case "user":
			val = "<img border='0' src='images/user.png'>";
			break;
		case "group":
			val = "<img border='0' src='images/group.png'>";
			break;
		}
		return val;
	},

	/**
	 * Render a checkbox in the given grid cell.
	 */
	checkBoxRenderer : function(val, cell, record, row, col, store) {
		cell.css += " x-grid3-check-col-td";
		return '<div class="x-grid3-check-col' + ((true === val) ? '-on' : '') +
		  ' x-grid3-cc-' + this.id + '">&#160;</div>';
	}
});
