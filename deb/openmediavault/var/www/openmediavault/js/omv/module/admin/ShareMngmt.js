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
// require("js/omv/data/GroupingStore.js")
// require("js/omv/grid/PrivilegesGridPanel.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Privileges");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("privileges", "sharedfolder", {
	text: _("Shared Folders"),
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
				header: _("Name"),
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: _("Volume"),
				sortable: true,
				dataIndex: "volume",
				id: "volume"
			},{
				header: _("Path"),
				sortable: true,
				dataIndex: "reldirpath",
				id: "reldirpath"
			},{
				header: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			},{
				header: _("Used"),
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
			proxy: new OMV.data.DataProxy({
				"service": "ShareMgmt",
				"method": "getList"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "name" },
					{ name: "reldirpath" },
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
		// Add the 'Privileges' button.
		tbar.insert(2, {
			id: this.getId() + "-privileges",
			xtype: "button",
			text: _("Privileges"),
			icon: "images/privileges.gif",
			handler: this.cbPrivilegesBtnHdl,
			scope: this,
			disabled: true
		});
		// Add the 'ACL' button.
		tbar.insert(3, {
			id: this.getId() + "-acl",
			xtype: "button",
			text: _("ACL"),
			icon: "images/acl.png",
			handler: this.cbACLBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Privileges.SharedFolderGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons
		var tbarBtnName = [ "privileges", "acl" ];
		var tbarBtnDisabled = {
			"privileges": true,
			"acl": true
		};
		var records = model.getSelections();
		if (records.length <= 0) {
			tbarBtnDisabled["privileges"] = true;
			tbarBtnDisabled["acl"] = true;
		} else if (records.length == 1) {
			tbarBtnDisabled["privileges"] = false;
			tbarBtnDisabled["acl"] = false;
		} else {
			tbarBtnDisabled["privileges"] = true;
			tbarBtnDisabled["acl"] = true;
		}
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

	cbACLBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Privileges.SharedFolderACLDialog({
			rootText: record.get("name"),
			uuid: record.get("uuid")
		});
		wnd.show();
	},

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "ShareMgmt",
		  "delete", { "uuid": record.get("uuid") });
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
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add shared folder") : _("Edit shared folder"),
		width: 500,
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
			fieldLabel: _("Name"),
			allowBlank: false,
			vtype: "sharename"
		},{
			xtype: "combo",
			name: "mntentref",
			hiddenName: "mntentref",
			fieldLabel: _("Volume"),
			emptyText: _("Select a volume ..."),
			allowBlank: false,
			allowNone: false,
			editable: false,
			triggerAction: "all",
			displayField: "description",
			valueField: "uuid",
			store: new OMV.data.Store({
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "ShareMgmt",
					"method": "getCandidates",
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					idProperty: "uuid",
					fields: [
						{ name: "uuid" },
						{ name: "description" }
					]
				})
			})
		},{
			xtype: "trigger",
			name: "reldirpath",
			fieldLabel: _("Path"),
			allowBlank: false,
			triggerClass: "x-form-folder-trigger",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: (this.uuid !== OMV.UUID_UNDEFINED) ?
			  _("The path of the folder to share.") :
			  _("The path of the folder to share. The specified folder will be created if it does not already exist."),
			onTriggerClick: function() {
				// Get the UUID of the selected volume.
				var field = this.ownerCt.ownerCt.findFormField("mntentref");
				var value = field.getValue();
				if (Ext.isUUID(value)) {
					var wnd = new OMV.Module.Privileges.
					  SharedFolderDirChooserDialog({
						uuid: value,
						listeners: {
							select: function(dlg, value) {
								// Set the selected path.
								this.setValue(value);
							},
							scope: this
						}
					});
					wnd.show();
				} else {
					OMV.MessageBox.info(null, _("Please first select a volume."));
				}
			}
		},{
			xtype: "combo",
			name: "mode",
			hiddenName: "mode",
			fieldLabel: _("Permissions"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "700",_("Administrator: read/write, Users: no access, Others: no access") ],
					[ "750",_("Administrator: read/write, Users: read-only, Others: no access") ],
					[ "770",_("Administrator: read/write, Users: read/write, Others: no access") ],
					[ "755",_("Administrator: read/write, Users: read-only, Others: read-only") ],
					[ "775",_("Administrator: read/write, Users: read/write, Others: read-only") ],
					[ "777",_("Everyone: read/write") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			hidden: (this.uuid !== OMV.UUID_UNDEFINED),
			submitValue: (this.uuid == OMV.UUID_UNDEFINED),
			value: "775",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The file mode of the shared folder path.")
		},{
			xtype: "textarea",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		if (this.uuid == OMV.UUID_UNDEFINED)
			return;
		// Set various fields to read-only.
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
 * @derived Ext.Window
 * @config uuid The UUID of the shared folder to process.
 * @config readOnly TRUE to set the dialog to read-only.
 * Defaults to FALSE.
 */
OMV.Module.Privileges.PrivilegesPropertyDialog = function(config) {
	var initialConfig = {
		title: _("Edit shared folder privileges"),
		width: 550,
		height: 350,
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
		this.grid = new OMV.grid.PrivilegesGridPanel({
			stateId: "474eacf4-cadb-4ae4-b545-4f7f47d7aed9",
			store: new OMV.data.GroupingStore({
				autoLoad: true,
				remoteSort: false,
				proxy: new OMV.data.DataProxy({
					"service": "ShareMgmt",
					"method": "getPrivileges",
					"extraParams": { "uuid": this.uuid },
					"appendPagingParams": false
				}),
				reader: new Ext.data.JsonReader({
					fields: [{
						name: "type"
					},{
						name: "name"
					},{
						name: "deny",
						convert: function(v, record) {
							return (0 === record.perms);
						}
					},{
						name: "readonly",
						convert: function(v, record) {
							return (5 === record.perms);
						}
					},{
						name: "writeable",
						convert: function(v, record) {
							return (7 === record.perms);
						}
					}]
				}),
				sortInfo: {
					field: "name",
					direction: "ASC"
				}
			}),
			view: new Ext.grid.GroupingView({
				forceFit: true
			})
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
			items: [ this.grid ],
			bbar: {
				xtype: "toolbar",
				items: [{
					xtype: "tbitem",
					autoEl: { tag: "img", src: "images/info.png" },
					style: {
						margin: "1px"
					}
				},{
					xtype: "tbtext",
					style: {
						"white-space": "normal"
					},
					text: _("These settings are used by the services to configure the user access rights. Please note that these settings do no affect the file system permissions.")
				}]
			}
		});
		OMV.Module.Privileges.PrivilegesPropertyDialog.superclass.
		  initComponent.apply(this, arguments);
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

	doSubmit : function() {
		// Display waiting dialog
		OMV.MessageBox.wait(null, _("Saving ..."));
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
					name: record.get("name"),
					perms: perms
				});
			}
		}
		OMV.Ajax.request(this.cbSubmitHdl, this, "ShareMgmt",
		  "setPrivileges", values);
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

/**
 * @class OMV.Module.Privileges.SharedFolderACLDialog
 * @derived Ext.Window
 * @config uuid The UUID of the shared folder to process.
 * @config rootText The name of the shared folder.
 */
OMV.Module.Privileges.SharedFolderACLDialog = function(config) {
	var initialConfig = {
		title: _("Modify shared folder ACL"),
		width: 600,
		height: 520,
		layout: "border",
		modal: true,
		border: false,
		buttonAlign: "center",
		readOnly: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.SharedFolderACLDialog.superclass.
	  constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Privileges.SharedFolderACLDialog, Ext.Window, {
	initComponent : function() {
		this.tree = new Ext.tree.TreePanel({
			region: "west",
			title: _("Directory"),
			autoScroll: true,
			border: true,
			split: true,
			width: 210,
			collapsible: true,
			loader: new Ext.tree.TreeLoader({
				baseParams: {
					"uuid": this.uuid,
					"type": "sharedfolder"
				},
				load: function(node, callback, scope) {
					if (this.clearOnLoad) {
						while (node.firstChild) {
							node.removeChild(node.firstChild);
						}
					}
					if (this.fireEvent("beforeload", this, node,
					  callback) !== false) {
						// Get the relative directory path.
						var dir = "/";
						node.bubble(function(o) {
							if (this.isRoot !== true) {
								dir = "/" + this.text + dir;
							}
						});
						// Build the RPC parameter
						var params = Ext.apply({
							"dir": dir
						}, this.baseParams);
						OMV.Ajax.request(function(id, response, error) {
							  // Prepare data as expected by the tree loader
							  // implementation.
							  var resp = {
								  "responseData": null,
								  "argument": {
									  "callback": callback,
									  "node": node,
									  "scope": scope
								  }
							  };
							  if (error === null) {
								  resp.responseData = [];
								  response.each(function(text) {
									  // Create the node configuration objects.
									  // The field 'dir' contains the relative
									  // path to the directory within the
									  // shared folder.
									  resp.responseData.push({
										  "text": text,
										  "dir": dir + text
									  })
								  }, this);
								  this.handleResponse(resp);
							  } else {
								  resp.responseData = error;
								  this.handleFailure(resp);
							  }
						  }, this, "DirBrowser", "get", params);
					} else {
						this.runCallback(callback, scope || node, []);
					}
				},
				listeners: {
					"scope": this,
					"loadexception": function(tree, node, response) {
						OMV.MessageBox.error(null, response.responseData);
					}
				}
			}),
			rootVisible: true,
			root: new Ext.tree.AsyncTreeNode({
				"text": this.rootText,
				"dir": "/",
				"id": "0"
			}),
			listeners: {
				"scope": this,
				"click": function(node, e) {
					// Display load mask.
					this.grid.loadMask.show();
					// Load the ACL list.
					OMV.Ajax.request(function(id, response, error) {
						  if (error !== null) {
							  this.grid.loadMask.hide();
							  OMV.MessageBox.error(null, error);
						  } else {
							  // Set the form field values.
							  this.form.setValues({
								  "owner": response.owner,
								  "group": response.group,
								  "userperms": response.acl.user,
								  "groupperms": response.acl.group,
								  "otherperms": response.acl.other
							  });
							  // Set the grid values.
							  var data = [];
							  ["user","group"].each(function(type) {
								  response["acl"][type + "s"].each(
									function(r) {
									  data.push({
										  "type": type,
										  "name": r.name,
										  "perms": r.perms,
										  "system": r.system
									  });
								  }, this);
							  }, this);
							  this.grid.store.loadData(data);
							  // Disable load mask.
							  this.grid.loadMask.hide();
						  }
					  }, this, "ShareMgmt", "getFileACL", { "uuid": this.uuid,
					  "file": node.attributes.dir });
				}
			}
		});

		this.grid = new OMV.grid.PrivilegesGridPanel({
			title: _("User/Group permissions"),
			border: true,
			region: "center",
			stateId: "7ae170d2-647d-11e1-b329-00221568ca88",
			store: new OMV.data.GroupingStore({
				autoLoad: false,
				remoteSort: false,
				reader: new Ext.data.JsonReader({
					fields: [{
						name: "type"
					},{
						name: "name"
					},{
						name: "deny",
						convert: function(v, record) {
							return (0 === record.perms);
						}
					},{
						name: "readonly",
						convert: function(v, record) {
							return (5 === record.perms);
						}
					},{
						name: "writeable",
						convert: function(v, record) {
							return (7 === record.perms);
						}
					},{
						name: "system"
					}]
				}),
				sortInfo: {
					field: "name",
					direction: "ASC"
				},
				groupField: "system"
			}),
			view: new Ext.grid.GroupingView({
				forceFit: true
			}),
			extraColumns: [{
				header: _("System user/group"),
				sortable: true,
				groupable: true,
				hidden: true,
				dataIndex: "system",
				id: "system",
				align: "center",
				renderer: OMV.util.Format.booleanRenderer(),
				scope: this
			}]
		});

		this.form = new OMV.form.FormPanel({
			title: _("Extra options"),
			region: "south",
			split: true,
			collapsible: true,
			frame: true,
			border: false,
			autoHeight: true,
			defaults: {
				anchor: "100%"
			},
			items: [{
				xtype: "compositefield",
				fieldLabel: _("Owner"),
				items: [{
					xtype: "textfield",
					name: "owner",
					readOnly: true,
					submitValue: false,
					flex: 1,
					value: _("n/a")
				},{
					xtype: "combo",
					name: "userperms",
					hiddenName: "userperms",
					mode: "local",
					store: new Ext.data.SimpleStore({
						fields: [ "value","text" ],
						data: [
							[ 0,_("No access") ],
							[ 5,_("Read-only") ],
							[ 7,_("Read/Write") ]
						]
					}),
					displayField: "text",
					valueField: "value",
					allowBlank: false,
					editable: false,
					triggerAction: "all",
					flex: 1,
					value: 7
				}],
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Permissions of owner.")
			},{
				xtype: "compositefield",
				fieldLabel: _("Group"),
				items: [{
					xtype: "textfield",
					name: "group",
					readOnly: true,
					submitValue: false,
					flex: 1,
					value: _("n/a")
				},{
					xtype: "combo",
					name: "groupperms",
					hiddenName: "groupperms",
					mode: "local",
					store: new Ext.data.SimpleStore({
						fields: [ "value","text" ],
						data: [
							[ 0,_("No access") ],
							[ 5,_("Read-only") ],
							[ 7,_("Read/Write") ]
						]
					}),
					displayField: "text",
					valueField: "value",
					allowBlank: false,
					editable: false,
					triggerAction: "all",
					flex: 1,
					value: 7
				}],
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Permissions of group.")
			},{
				xtype: "combo",
				name: "otherperms",
				hiddenName: "otherperms",
				fieldLabel: _("Others"),
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,_("No access") ],
						[ 5,_("Read-only") ],
						[ 7,_("Read/Write") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 0,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Permissions of others (e.g. anonymous FTP users).")
			},{
				xtype: "checkbox",
				name: "replace",
				fieldLabel: _("Replace"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Replace all existing permissions")
			},{
				xtype: "checkbox",
				name: "recursive",
				fieldLabel: _("Recursive"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Apply permissions to files and subfolders")
			}]
		});

		Ext.apply(this, {
			buttons: [{
				text: _("Apply"),
				handler: this.cbApplyBtnHdl,
				scope: this,
				disabled: this.readOnly
			},{
				text: _("Close"),
				handler: this.close,
				scope: this
			}],
			items: [ this.tree, this.grid, this.form ]
		});
		OMV.Module.Privileges.SharedFolderACLDialog.superclass.
		  initComponent.apply(this, arguments);
		// Add event handler
		this.on("afterrender", function() {
			// Auto-select the root node and fire event to display the
			// directory ACL settings.
			var node = this.tree.getRootNode();
			node.select();
			this.tree.fireEvent("click", node);
		}, this);
	},

	/**
	 * @method cbApplyBtnHdl
	 * Method that is called when the 'Apply' button is pressed.
	 */
	cbApplyBtnHdl : function() {
		var node = this.tree.getSelectionModel().getSelectedNode();
		var records = this.grid.store.getRange();
		var options = this.form.getValues();
		var users = [];
		var groups = [];
		for (var i = 0; i < records.length; i++) {
			var record = records[i];
			if ((true === record.get("deny")) ||
			  (true === record.get("readonly")) ||
			  (true === record.get("writeable"))) {
				var object = {
					"name": record.get("name"),
					"perms": 0
				}
				if (true === record.get("readonly"))
					object.perms = 5;
				else if (true === record.get("writeable"))
					object.perms = 7;
				switch (record.get("type")) {
				case "user":
					users.push(object);
					break;
				case "group":
					groups.push(object);
					break;
				}
			}
		}
		var dlg = new OMV.ExecCmdDialog({
			title: _("Updating ACL settings"),
			width: 350,
			rpcService: "ShareMgmt",
			rpcMethod: "setFileACL",
			rpcArgs: {
				"uuid": this.uuid,
				"file": node.attributes.dir,
				"recursive": options.recursive,
				"replace": options.replace,
				"user": options.userperms,
				"group": options.groupperms,
				"other": options.otherperms,
				"users": users,
				"groups": groups
			},
			hideStart: true,
			hideStop: true,
			hideClose: true,
			progress: true,
			listeners: {
				start: function(c) {
					c.show();
				},
				finish: function(c) {
					var value = c.getValue();
					c.close();
					if (value.length > 0) {
						OMV.MessageBox.error(null, value);
					}
				},
				exception: function(c, error) {
					c.close();
					OMV.MessageBox.error(null, error);
				},
				scope: this
			}
		});
		dlg.start();
	}
});

/**
 * @class OMV.Module.Privileges.SharedFolderDirChooserDialog
 * @derived Ext.Window
 * @config uuid The UUID of the volume to process.
 */
OMV.Module.Privileges.SharedFolderDirChooserDialog = function(config) {
	var initialConfig = {
		title: _("Select a directory"),
		width: 300,
		height: 400,
		layout: "fit",
		modal: true,
		border: true,
		buttonAlign: "center",
		readOnly: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Privileges.SharedFolderDirChooserDialog.superclass.
	  constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires after the dialog has been closed by pressing the 'OK' button.
		 */
		"select"
	);
};
Ext.extend(OMV.Module.Privileges.SharedFolderDirChooserDialog, Ext.Window, {
	initComponent : function() {
		this.tree = new Ext.tree.TreePanel({
			autoScroll: true,
			border: false,
			loader: new Ext.tree.TreeLoader({
				baseParams: {
					"uuid": this.uuid,
					"type": "mntent"
				},
				load: function(node, callback, scope) {
					if (this.clearOnLoad) {
						while (node.firstChild) {
							node.removeChild(node.firstChild);
						}
					}
					if (this.fireEvent("beforeload", this, node,
					  callback) !== false) {
						// Get the relative directory path.
						var dir = "";
						node.bubble(function(o) {
							if (this.isRoot !== true) {
								dir = this.text + "/" + dir;
							}
						});
						// Build the RPC parameter
						var params = Ext.apply({
							"dir": dir
						}, this.baseParams);
						OMV.Ajax.request(function(id, response, error) {
							  // Prepare data as expected by the tree loader
							  // implementation.
							  var resp = {
								  "responseData": null,
								  "argument": {
									  "callback": callback,
									  "node": node,
									  "scope": scope
								  }
							  };
							  if (error === null) {
								  resp.responseData = [];
								  response.each(function(text) {
									  // Create the node configuration objects.
									  // The field 'dir' contains the relative
									  // path to the directory within the
									  // shared folder.
									  resp.responseData.push({
										  "text": text,
										  "dir": dir + text + "/"
									  })
								  }, this);
								  this.handleResponse(resp);
							  } else {
								  resp.responseData = error;
								  this.handleFailure(resp);
							  }
						  }, this, "DirBrowser", "get", params);
					} else {
						this.runCallback(callback, scope || node, []);
					}
				},
				listeners: {
					"scope": this,
					"loadexception": function(tree, node, response) {
						OMV.MessageBox.error(null, response.responseData);
					}
				}
			}),
			rootVisible: false,
			root: new Ext.tree.AsyncTreeNode(),
			listeners: {
				"scope": this,
				"click": function(node, e) {
					var btnCtrl = Ext.getCmp(this.getId() + "-ok");
					btnCtrl.setDisabled(node.isSelected());
				}
			}
		});
		Ext.apply(this, {
			buttons: [{
				id: this.getId() + "-ok",
				text: _("OK"),
				disabled: true,
				handler: this.cbOkBtnHdl,
				scope: this
			},{
				text: _("Cancel"),
				handler: function() {
					this.close();
				},
				scope: this
			}],
			items: [ this.tree ]
		});
		OMV.Module.Privileges.SharedFolderDirChooserDialog.superclass.
		  initComponent.apply(this, arguments);
	},

	/**
	 * @method cbOkBtnHdl
	 * Method that is called when the 'OK' button is pressed.
	 */
	cbOkBtnHdl : function() {
		var node = this.tree.getSelectionModel().getSelectedNode();
		this.fireEvent("select", this, node.attributes.dir);
		this.close();
	}
});
