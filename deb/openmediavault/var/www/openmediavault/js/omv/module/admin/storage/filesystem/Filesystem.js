/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/Grid.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/RpcBg.js")
// require("js/omv/util/Format.js")
// require("js/omv/window/Execute.js")

/**
 * @class OMV.module.admin.storage.filesystem.Create
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.storage.filesystem.Create", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.RpcBg",
		"OMV.window.Execute"
	],

	mode: "local",
	title: _("Create file system"),
	okButtonText: _("OK"),
	hideResetButton: true,
	width: 500,

	getFormConfig: function() {
		return {
			plugins: [{
				ptype: "linkedfields",
				correlations: [{
					name: "label",
					conditions: [
						{ name: "type", value: "xfs" }
					],
					properties: function(valid, field) {
						// Update the max. length of the file system label
						// depending on the selected file system type.
						// XFS = 12, Other = 16
						field.maxLength = valid ? 12 : 16
					}
				}]
			}]
		};
	},

	getFormItems: function() {
		return [{
			xtype: "combo",
			name: "devicefile",
			fieldLabel: _("Device"),
			emptyText: _("Select a device ..."),
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "devicefile", type: "string" },
						{ name: "description", type: "string" }
					]
				}),
				proxy: {
					type: "rpcbg",
					appendSortParams: false,
					rpcData: {
						service: "FileSystemMgmt",
						method: "getCandidatesBg"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
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
			plugins: [{
				ptype: "fieldinfo",
				text: _("The volume label for the file system."),
			}],
			vtype: "fslabel"
		},{
			xtype: "combo",
			name: "type",
			fieldLabel: _("File system"),
			emptyText: _("Select a file system ..."),
			queryMode: "local",
			store: [
				[ "btrfs", "BTRFS" ],
				[ "ext3", "EXT3" ],
				[ "ext4", "EXT4" ],
				[ "f2fs", "F2FS" ],
				[ "xfs", "XFS" ],
				[ "jfs", "JFS" ]
			],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "ext4"
		}];
	},

	doSubmit: function() {
		var me = this;
		OMV.MessageBox.confirm(null,
			_("Do you really want to format this device? All data on it will be deleted. Please note that the file system creation may take some time."),
			function(answer) {
				if (answer === "no")
					return;
				// Display dialog showing the file system creation progress.
				var params = me.getRpcSetParams();
				Ext.create("OMV.window.Execute", {
					title: _("Create file system"),
					rpcService: "FileSystemMgmt",
					rpcMethod: "create",
					rpcParams: params,
					hideStartButton: true,
					hideStopButton: true,
					adaptButtonState: false,
					listeners: {
						start: function(wnd) {
							wnd.appendValue(Ext.String.format("{0}\n",
							  _("Creating the file system, please wait ...")));
							wnd.setButtonDisabled("close", false);
							wnd.show();
						},
						finish: function(wnd, response) {
							wnd.appendValue(_("The file system creation has completed successfully."));
							wnd.setButtonDisabled("close", false);
						},
						exception: function(wnd, error) {
							OMV.MessageBox.error(null, error);
						}
					}
				}).start();
				me.superclass.doSubmit.apply(this, arguments);
			}, me);
	}
});

/**
 * @class OMV.module.admin.storage.filesystem.Quota
 * @derived OMV.workspace.window.Grid
 * @param uuid The UUID of the file system to process.
 * @param readOnly TRUE to set the dialog to read-only.
 * Defaults to FALSE.
 */
Ext.define("OMV.module.admin.storage.filesystem.Quota", {
	extend: "OMV.workspace.window.Grid",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.workspace.window.plugin.ConfigObject",
		"Ext.grid.plugin.RowEditing"
	],

	rpcService: "Quota",
	rpcGetMethod: "get",
	rpcSetMethod: "set",
	plugins: [{
		ptype: "configobject"
	}],

	title: _("Edit quota"),
	width: 500,
	height: 305,

	getGridConfig: function() {
		var me = this;
		return {
			border: false,
			stateful: true,
			stateId: "24f018e6-8de6-41e1-b6c4-db0edd49a73b",
			plugins: [{
				ptype: "rowediting",
				clicksToEdit: 1
			}],
			columns: [{
				xtype: "fonticoncolumn",
				text: _("Type"),
				sortable: true,
				dataIndex: "type",
				stateId: "type",
				align: "center",
				width: 60,
				resizable: false,
				getFontIconCls: function(value) {
					if (value == "user")
						return "mdi mdi-account";
					return "mdi mdi-account-multiple";
				}
			},{
				xtype: "textcolumn",
				text: _("Name"),
				sortable: true,
				dataIndex: "name",
				stateId: "name",
				flex: 2
			},{
				text: _("Used capacity"),
				sortable: true,
				dataIndex: "bused",
				stateId: "bused",
				flex: 1,
				renderer: function(value) {
					if (value <= 0)
						value = "--";
					return Ext.String.htmlEncode(value);
				}
			},{
				text: _("Quota"),
				sortable: true,
				dataIndex: "bhardlimit",
				stateId: "bhardlimit",
				align: "right",
				editor: {
					xtype: "numberfield",
					allowBlank: true,
					allowDecimals: true,
					minValue: 0
				},
				flex: 1,
				renderer: function(value) {
					if ((value <= 0) || Ext.isEmpty(value))
						value = "--";
					return Ext.String.htmlEncode(value);
				}
			},{
				text: _("Unit"),
				sortable: true,
				dataIndex: "bunit",
				stateId: "bunit",
				width: 80,
				resizable: false,
				editor: {
					xtype: "combo",
					queryMode: "local",
					store: [
						[ "KiB", _("KiB") ],
						[ "MiB", _("MiB") ],
						[ "GiB", _("GiB") ],
						[ "TiB", _("TiB") ]
					],
					allowBlank: false,
					triggerAction: "all",
					editable: false,
					forceSelection: true
				}
			}],
			store: Ext.create("Ext.data.JsonStore", {
				model: OMV.data.Model.createImplicit({
					identifier: "uuid", // Populate 'id' field automatically.
					idProperty: "id",
					fields: [
						{ name: "type", type: "string" },
						{ name: "name", type: "string" },
						{ name: "bused", type: "int" },
						{ name: "bhardlimit", type: "int" },
						{ name: "bunit", type: "string" }
					]
				}),
				proxy: {
					type: "memory",
					reader: {
						type: "json"
					}
				},
				listeners: {
					scope: me,
					load: function(store, records) {
						// Records with client-side generated values for
						// idProperty are called phantom records. These
						// records need to be modified, otherwise the
						// 'Reset' button feature will remove them from
						// the store.
						Ext.Array.each(records, function(record) {
							record.phantom = false;
						});
					}
				}
			}),
			listeners: {
				scope: me,
				beforeedit: function(editor, e, eOpts) {
					switch (e.field) {
					case "bhardlimit":
						// Display a empty number field if value is 0.
						if (e.value == 0)
							e.record.set("bhardlimit", "");
						break;
					}
				},
				validateedit: function(editor, e, eOpts) {
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
		};
	},

	getRpcSetParams: function() {
		var me = this;
		var quota = [];
		var values = me.getValues();
		Ext.Array.each(values, function(value) {
			// Only submit useful settings.
			if ((value.bhardlimit == 0) || Ext.isEmpty(value.bhardlimit))
				return;
			quota.push({
				type: value.type,
				name: value.name,
				bhardlimit: value.bhardlimit,
				bunit: value.bunit
			});
		});
		return {
			quota: quota
		};
	}
});

/**
 * @class OMV.module.admin.storage.filesystem.Filesystems
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.storage.filesystem.Filesystems", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.RpcBg"
	],
	uses: [
		"OMV.module.admin.storage.filesystem.Create",
		"OMV.module.admin.storage.filesystem.Quota"
	],

	autoReload: true,
	rememberSelected: true,
	hideAddButton: true,
	hideEditButton: true,
	hidePagingToolbar: false,
	disableLoadMaskOnLoad: true,
	stateful: true,
	stateId: "efea99a0-95d1-4bc9-8207-d21fe514f069",
	columns: [{
		xtype: "textcolumn",
		text: _("Device"),
		sortable: true,
		hidden: true,
		dataIndex: "devicefile",
		stateId: "devicefile"
	},{
		xtype: "devicefilescolumn",
		text: _("Device(s)"),
		sortable: false,
		dataIndex: "devicefiles",
		stateId: "devicefiles"
	},{
		xtype: "emptycolumn",
		text: _("Identify As"),
		sortable: true,
		hidden: true,
		dataIndex: "id",
		stateId: "id"
	},{
		text: _("Label"),
		sortable: true,
		dataIndex: "label",
		stateId: "label"
	},{
		xtype: "emptycolumn",
		text: _("Parent Device"),
		sortable: true,
		hidden: true,
		dataIndex: "parentdevicefile",
		stateId: "parentdevicefile"
	},{
		text: _("Filesystem Type"),
		sortable: true,
		dataIndex: "type",
		stateId: "type"
	},{
		xtype: "binaryunitcolumn",
		text: _("Total"),
		sortable: true,
		dataIndex: "size",
		stateId: "size"
	},{
		xtype: "binaryunitcolumn",
		text: _("Available"),
		sortable: true,
		dataIndex: "available",
		stateId: "available"
	},{
		text: _("Used"),
		sortable: true,
		dataIndex: "used",
		stateId: "used",
		align: "center",
		renderer: function(value, metaData, record) {
			var percentage = parseInt(record.get("percentage"));
			if (-1 == percentage)
				return _("n/a");
			//var text = Ext.String.format("{0}% [{1}]",
			//	percentage, value);
			var renderer = OMV.util.Format.progressBarRenderer(
				percentage / 100, value, 0.85);
			return renderer.apply(this, arguments);
		}
	},{
		xtype: "booleantextcolumn",
		text: _("Mounted"),
		sortable: true,
		dataIndex: "mounted",
		stateId: "mounted"
	},{
		text: _("Mount Point"),
		sortable: true,
		hidden: true,
		dataIndex: "mountpoint",
		stateId: "mountpoint"
	},{
		xtype: "booleantextcolumn",
		text: _("Referenced"),
		sortable: true,
		dataIndex: "_used",
		stateId: "_used"
	},{
		text: _("Status"),
		sortable: true,
		dataIndex: "status",
		stateId: "status",
		renderer: function(value, metaData) {
			switch (value) {
			case 1:
				value = _("Online");
				break;
			case 2:
				value = "<img border='0' src='images/wait.gif'> " +
				  _("Initializing");
				break;
			default:
				metaData.tdCls += " x-color-error";
				value = _("Missing");
				break;
			}
			return value;
		}
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "id", type: "string", persist: false },
						{ name: "uuid", type: "string" },
						{ name: "devicefile", type: "string" },
						{ name: "devicefiles", type: "array" },
						{ name: "parentdevicefile", type: "string" },
						{ name: "label", type: "string" },
						{ name: "type", type: "string" },
						{ name: "blocks", type: "string" },
						{ name: "used", type: "string" },
						{ name: "available", type: "string" },
						{ name: "size", type: "string" },
						{ name: "status", type: "int" },
						{ name: "percentage", type: "string" },
						{ name: "mounted", type: "boolean" },
						{ name: "mountpoint", type: "string" },
						{ name: "propfstab", type: "boolean" },
						{ name: "propresize", type: "boolean" },
						{ name: "propquota", type: "boolean" },
						{ name: "propposixacl", type: "boolean" },
						{ name: "_used", type: "boolean" },
						{ name: "_readonly", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpcbg",
					rpcData: {
						service: "FileSystemMgmt",
						method: "getListBg",
						options: {
							updatelastaccess: false
						}
					},
					reader: {
						type: "rpcjson",
						transform: {
							fn: function(data) {
								data.data.map(function(item) {
									// Convert the 'id' field value.
									item.id = item.devicefile;
									if (!Ext.isEmpty(item.uuid)) {
										item.id = Ext.String.format(
											"UUID={0}", item.uuid);
									}
									return item;
								});
								return data;
							}
						}
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "devicefile"
				}]
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		Ext.Array.insert(items, 1, [{
			id: me.getId() + "-create",
			xtype: "button",
			text: _("Create"),
			iconCls: "x-fa fa-plus",
			handler: Ext.Function.bind(me.onCreateButton, me, [ me ]),
			scope: me,
			disabled: false
		},{
			id: me.getId() + "-resize",
			xtype: "button",
			text: _("Resize"),
			iconCls: "x-fa fa-expand",
			handler: Ext.Function.bind(me.onResizeButton, me, [ me ]),
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-quota",
			xtype: "button",
			text: _("Quota"),
			iconCls: "x-fa fa-users",
			handler: Ext.Function.bind(me.onQuotaButton, me, [ me ]),
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-mount",
			xtype: "button",
			text: _("Mount"),
			iconCls: "x-fa fa-play",
			handler: Ext.Function.bind(me.onMountButton, me, [ me ]),
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-unmount",
			xtype: "button",
			text: _("Unmount"),
			iconCls: "x-fa fa-eject",
			handler: Ext.Function.bind(me.onUnmountButton, me, [ me ]),
			scope: me,
			disabled: true
		}]);
		return items;
	},

	onSelectionChange: function(model, records) {
		var me = this;
		me.callParent(arguments);
		// Process additional buttons.
		var tbarBtnDisabled = {
			"resize": true,
			"quota": true,
			"delete": false,
			"mount": true,
			"unmount": true
		};
		if (records.length <= 0) {
			tbarBtnDisabled["resize"] = true;
			tbarBtnDisabled["quota"] = true;
			tbarBtnDisabled["delete"] = true;
			tbarBtnDisabled["mount"] = true;
			tbarBtnDisabled["unmount"] = true;
		} else if(records.length == 1) {
			var record = records[0];
			// Set default values.
			tbarBtnDisabled["resize"] = true;
			tbarBtnDisabled["quota"] = true;
			tbarBtnDisabled["delete"] = false;
			tbarBtnDisabled["mount"] = true;
			tbarBtnDisabled["unmount"] = true;
			// Enable the 'Resize' button if the filesystem supports online
			// resizing.
			if (true === record.get("propresize"))
				tbarBtnDisabled["resize"] = false;
			// Enable the 'Quota' button if the filesystem supports this
			// feature and if it is mounted.
			if (true === record.get("propquota"))
				tbarBtnDisabled["quota"] = !record.get("mounted");
			// Disable/enable the mount/unmount buttons depending on whether
			// the selected file system is mounted.
			if (true === record.get("mounted")) {
				tbarBtnDisabled["unmount"] = false;
			} else {
				tbarBtnDisabled["mount"] = false;
			}
			// If the file system is in use, then also disable the unmount
			// button.
			if (true === record.get("_used"))
				tbarBtnDisabled["unmount"] = true;
			// Disable buttons if a selected file system is
			// initialized (status=2) or missing (status=3) at the
			// moment.
			if ([ 2, 3 ].indexOf(record.get("status")) !== -1) {
				tbarBtnDisabled["resize"] = true;
				tbarBtnDisabled["quota"] = true;
				tbarBtnDisabled["mount"] = true;
				// Only disable the 'Delete' button if the filesystem
				// is initialized, otherwise missing filesystems can't
				// be removed.
				if (2 == record.get("status"))
					tbarBtnDisabled["delete"] = true;
			}
			// If the filesystem does not support fstab mount entries,
			// then disable the 'Delete', 'Mount' and 'Unmount' buttons.
			if (false === record.get("propfstab")) {
				tbarBtnDisabled["delete"] = true;
				tbarBtnDisabled["mount"] = true;
				tbarBtnDisabled["unmount"] = true;
			}
		} else {
			// Set default values.
			tbarBtnDisabled["resize"] = true;
			tbarBtnDisabled["quota"] = true;
			tbarBtnDisabled["delete"] = false;
			tbarBtnDisabled["mount"] = true;
			tbarBtnDisabled["unmount"] = true;
			// Disable button if one of the selected file systems is
			// initialized (status=2) at the moment.
			for (var i = 0; i < records.length; i++) {
				if (2 == records[i].get("status")) {
					tbarBtnDisabled["delete"] = true;
				}
			}
		}
		// Disable 'Delete' button if a selected file system is in use
		// or readonly.
		for (var i = 0; i < records.length; i++) {
			if ((true == records[i].get("_used")) ||
			  (true == records[i].get("_readonly"))) {
				tbarBtnDisabled["delete"] = true;
			}
			if (true == records[i].get("_readonly")) {
				tbarBtnDisabled["resize"] = true;
				tbarBtnDisabled["quota"] = true;
			}
		}
		// Update the button controls.
		Ext.Object.each(tbarBtnDisabled, function(key, value) {
			this.setToolbarButtonDisabled(key, value);
		}, me);
	},

	onCreateButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.storage.filesystem.Create", {
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onResizeButton: function() {
		var me = this;
		OMV.MessageBox.confirm(null,
			_("Do you really want to resize the selected file system? You have to do that after a RAID has been grown for example."),
			function(answer) {
				if (answer !== "yes")
					return;
				var record = me.getSelected();
				// Execute RPC.
				OMV.Rpc.request({
					scope: this,
					callback: function(id, success, response) {
						this.doReload();
					},
					relayErrors: false,
					rpcData: {
						service: "FileSystemMgmt",
						method: "resize",
						params: {
							id: record.get("uuid")
						}
					}
				});
			}, me);
	},

	onQuotaButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.storage.filesystem.Quota", {
			uuid: record.get("uuid")
		}).show();
	},

	onMountButton: function() {
		var me = this;
		var record = me.getSelected();
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				this.doReload();
			},
			relayErrors: false,
			rpcData: {
				service: "FileSystemMgmt",
				method: "mount",
				params: {
					id: record.get("devicefile"),
					fstab: true
				}
			}
		});
	},

	onUnmountButton: function() {
		var me = this;
		var record = me.getSelected();
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				this.doReload();
			},
			relayErrors: false,
			rpcData: {
				service: "FileSystemMgmt",
				method: "umount",
				params: {
					id: record.get("devicefile"),
					fstab: true
				}
			}
		});
	},

	startDeletion: function(records) {
		var me = this;
		if(records.length <= 0)
			return;
		OMV.MessageBox.show({
			title: _("Delete file system"),
			msg: _("Do you really want to delete the file system? All data will be lost."),
			icon: Ext.Msg.WARNING,
			buttonText: {
				yes: _("No"),
				no: _("Yes")
			},
			scope: me,
			fn: function(answer) {
				switch(answer) {
				case "no": // Attention, switched buttons.
					me.superclass.startDeletion.apply(this, [ records ]);
					break;
				default:
					break;
				}
			}
		});
	},

	doDeletion: function(record) {
		var me = this;
		// Prefer the file system UUID, but in some cases a file system does
		// not have a UUID, then use the devicefile instead.
		var id = record.get("uuid");
		if(Ext.isEmpty(id))
			id = record.get("devicefile");
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "FileSystemMgmt",
				method: "delete",
				params: {
					id: id
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerNode({
	id: "filesystem",
	path: "/storage",
	text: _("File Systems"),
	iconCls: "mdi mdi-folder-multiple",
	position: 40
});

OMV.WorkspaceManager.registerPanel({
	id: "filesystems",
	path: "/storage/filesystem",
	text: _("File Systems"),
	position: 10,
	className: "OMV.module.admin.storage.filesystem.Filesystems"
});
