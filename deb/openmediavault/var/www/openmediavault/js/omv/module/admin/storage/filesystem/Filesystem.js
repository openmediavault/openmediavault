/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/util/Format.js")

/**
 * @class OMV.module.admin.storage.filesystem.Create
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.storage.filesystem.Create", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],

	rpcService: "FileSystemMgmt",
	rpcSetMethod: "create",
	title: _("Create filesystem"),
	hideResetButton: true,
	width: 500,
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: "label",
			conditions: [
				{ name: "type", value: "xfs" }
			],
			properties: function(valid, field) {
				// Update the max. length of the filesystem label
				// depending on the selected filesystem type.
				// XFS = 12, Other = 16
				field.maxLength = valid ? 12 : 16
			}
		}]
	}],

	getFormItems: function() {
		var me = this;
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
					type: "rpc",
					appendSortParams: false,
					rpcData: {
						service: "FileSystemMgmt",
						method: "getCandidates"
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
				text: _("The volume label for the filesystem."),
			}],
			vtype: "fslabel"
		},{
			xtype: "combo",
			name: "type",
			fieldLabel: _("Filesystem"),
			emptyText: _("Select a filesystem ..."),
			queryMode: "local",
			store: [
				[ "ext3", "EXT3" ],
				[ "ext4", "EXT4" ],
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
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: _("Do you really want to format this device? All data on it will be deleted. Please note that the filesystem creation may take some time."),
			buttons: Ext.Msg.YESNO,
			fn: function(answer) {
				if(answer === "no")
					return;
				me.superclass.doSubmit.apply(this, arguments);
			},
			scope: me,
			icon: Ext.Msg.QUESTION
		});
	}
});

/**
 * @class OMV.module.admin.storage.filesystem.Quota
 * @derived OMV.workspace.window.Grid
 * @param uuid The UUID of the filesystem to process.
 * @param readOnly TRUE to set the dialog to read-only.
 * Defaults to FALSE.
 */
Ext.define("OMV.module.admin.storage.filesystem.Quota", {
	extend: "OMV.workspace.window.Grid",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.workspace.window.plugin.ConfigObject"
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
			plugins: Ext.create("Ext.grid.plugin.CellEditing", {
				clicksToEdit: 1
			}),
			columns: [{
				text: _("Type"),
				sortable: true,
				dataIndex: "type",
				stateId: "type",
				align: "center",
				width: 60,
				resizable: false,
				renderer: function(value) {
					switch(value) {
					case "user":
						value = "<img border='0' src='images/user.png'>";
						break;
					case "group":
						value = "<img border='0' src='images/group.png'>";
						break;
					}
					return value;
				}
			},{
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
					if(value <= 0)
						value = "--";
					return value;
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
					if((value <= 0) || Ext.isEmpty(value))
						value = "--";
					return value;
				}
			},{
				text: _("Unit"),
				sortable: true,
				dataIndex: "bunit",
				stateId: "bunit",
				width: 60,
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
					idProperty: "name",
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
				}
			}),
			listeners: {
				scope: me,
				beforeedit: function(editor, e, eOpts) {
					switch(e.field) {
					case "bhardlimit":
						// Display a empty number field if value is 0.
						if(e.value == 0)
							e.record.set("bhardlimit", "");
						break;
					}
				},
				validateedit: function(editor, e, eOpts) {
					var bunit = e.record.get("bunit");
					var bhardlimit = e.record.get("bhardlimit");
					switch(e.field) {
					case "bhardlimit":
						bhardlimit = !Ext.isEmpty(e.value) ? e.value : 0;
						break;
					case "bunit":
						bunit = e.value;
						break;
					}
					// Validate quota with max. possible value (4TiB).
					bhardlimit = bhardlimit.binaryConvert(bunit, "B");
					if(bhardlimit > 4 * Math.pow(2, 40)) {
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
		var records = me.getValues();
		Ext.Array.each(records, function(record) {
			var bhardlimit = record.get("bhardlimit");
			// Only submit useful settings.
			if((bhardlimit == 0) || Ext.isEmpty(bhardlimit))
				return;
			quota.push({
				"type": record.get("type"),
				"name": record.get("name"),
				"bhardlimit": bhardlimit,
				"bunit": record.get("bunit")
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
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.storage.filesystem.Create",
		"OMV.module.admin.storage.filesystem.Quota"
	],

	autoReload: true,
	rememberSelected: true,
	hideAddButton: true,
	hideEditButton: true,
	hidePagingToolbar: true,
	disableLoadMaskOnLoad: true,
	stateful: true,
	stateId: "efea99a0-95d1-4bc9-8207-d21fe514f069",
	columns: [{
		xtype: "emptycolumn",
		text: _("Device"),
		sortable: true,
		dataIndex: "devicefile",
		stateId: "devicefile"
	},{
		text: _("Label"),
		sortable: true,
		dataIndex: "label",
		stateId: "label"
	},{
		text: _("Filesystem"),
		sortable: true,
		dataIndex: "type",
		stateId: "type"
	},{
		xtype: "binaryunitcolumn",
		text: _("Capacity"),
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
			if(-1 == percentage)
				return _("n/a");
			var renderer = OMV.util.Format.progressBarRenderer(
				  percentage / 100, value);
			return renderer.apply(this, arguments);
		}
	},{
		xtype: "booleantextcolumn",
		text: _("Mounted"),
		sortable: true,
		dataIndex: "mounted",
		stateId: "mounted"
	},{
		text: _("Status"),
		sortable: true,
		dataIndex: "status",
		stateId: "status",
		renderer: function(value) {
			switch(value) {
			case 1:
				value = _("Online");
				break;
			case 2:
				value = "<img border='0' src='images/wait.gif'> " +
				  _("Initializing");
				break;
			default:
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
					idProperty: "num",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "devicefile", type: "string" },
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
						{ name: "_used", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "FileSystemMgmt",
						method: "getList",
						options: {
							updatelastaccess: false
						}
					}
				}
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
			icon: "images/filesystem.png",
			handler: me.onCreateButton,
			scope: me,
			disabled: false
		},{
			id: me.getId() + "-resize",
			xtype: "button",
			text: _("Resize"),
			icon: "images/filesystem-resize.png",
			handler: me.onResizeButton,
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-quota",
			xtype: "button",
			text: _("Quota"),
			icon: "images/filesystem-quota.png",
			handler: me.onQuotaButton,
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-mount",
			xtype: "button",
			text: _("Mount"),
			icon: "images/filesystem-mount.png",
			handler: me.onMountButton,
			scope: me,
			disabled: true
		},{
			id: me.getId() + "-unmount",
			xtype: "button",
			text: _("Unmount"),
			icon: "images/filesystem-umount.png",
			handler: me.onUnmountButton,
			scope: me,
			disabled: true
		}]);
		return items;
	},

	onSelectionChange: function(model, records) {
		var me = this;
		me.callParent(arguments);
		// Process additional buttons.
		var tbarBtnName = [ "resize", "quota", "delete", "mount", "unmount" ];
		var tbarBtnDisabled = {
			"resize": true,
			"quota": true,
			"delete": false,
			"mount": true,
			"unmount": true
		};
		if(records.length <= 0) {
			tbarBtnDisabled["resize"] = true;
			tbarBtnDisabled["quota"] = true;
			tbarBtnDisabled["delete"] = true;
			tbarBtnDisabled["mount"] = true;
			tbarBtnDisabled["unmount"] = true;
		} else if(records.length == 1) {
			tbarBtnDisabled["resize"] = false;
			tbarBtnDisabled["quota"] = false;
			tbarBtnDisabled["delete"] = false;
			tbarBtnDisabled["mount"] = true;
			tbarBtnDisabled["unmount"] = true;
			// Disable the 'Resize' button if filesystem is not supported.
			if([ "ext","ext2","ext3","ext4","xfs","jfs" ].indexOf(
			  records[0].get("type")) == -1) {
				tbarBtnDisabled["resize"] = true;
			}
			// Disable the 'Quota' button if the filesystem does not have
			// a mount point.
			if(Ext.isEmpty(records[0].get("mountpoint"))) {
				tbarBtnDisabled["quota"] = true;
			}
			// Disable/enable the mount/unmount buttons depending on whether
			// the selected filesystem is mounted.
			if(true === records[0].get("mounted")) {
				tbarBtnDisabled["unmount"] = false;
			} else {
				tbarBtnDisabled["mount"] = false;
				// Disable the 'Mount' button if the filesystem does not
				// provide a UUID.
				if(Ext.isEmpty(records[0].get("uuid"))) {
					tbarBtnDisabled["mount"] = true;
				}
			}
			// If the filesystem is in usage, then also disable the unmount
			// button.
			if(true === records[0].get("_used")) {
				tbarBtnDisabled["unmount"] = true;
			}
			// Finally disable buttons if a selected filesystem is
			// initialized at the moment.
			if([ 2,3 ].indexOf(records[0].get("status")) !== -1) {
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
			for(var i = 0; i < records.length; i++) {
				if(2 == records[i].get("status")) {
					tbarBtnDisabled["delete"] = true;
				}
			}
		}
		// Disable 'Delete' button if a selected filesystem is in usage
		// or readonly.
		for(var i = 0; i < records.length; i++) {
			if((true == records[i].get("_used")) ||
			  (true == records[i].get("_readOnly"))) {
				tbarBtnDisabled["delete"] = true;
			}
		}
		// Update the button controls.
		for(var i = 0; i < tbarBtnName.length; i++) {
			var tbarBtnCtrl = me.queryById(me.getId() + "-" + tbarBtnName[i]);
			if(!Ext.isEmpty(tbarBtnCtrl)) {
				if(true == tbarBtnDisabled[tbarBtnName[i]]) {
					tbarBtnCtrl.disable();
				} else {
					tbarBtnCtrl.enable();
				}
			}
		}
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
		var msg = _("Do you really want to resize the selected filesystem? You have to do that after a RAID has been grown for example.");
		OMV.MessageBox.show({
			title: _("Confirmation"),
			msg: msg,
			buttons: Ext.Msg.YESNO,
			icon: Ext.Msg.QUESTION,
			scope: me,
			fn: function(answer) {
				if(answer == "no")
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
			}
		});
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
					id: record.get("uuid"),
					fstab: true
				}
			}
		});
	},

	onUnmountButton: function() {
		var me = this;
		var record = me.getSelected();
		// Prefer the filesystem UUID, but in some cases a filesystem does not
		// have a UUID, then use the devicefile instead.
		var id = record.get("uuid");
		if(Ext.isEmpty(id))
			id = record.get("devicefile");
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
					id: id,
					fstab: true
				}
			}
		});
	},

	doDeletion: function(record) {
		var me = this;
		// Prefer the filesystem UUID, but in some cases a filesystem does not
		// have a UUID, then use the devicefile instead.
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
	text: _("Filesystems"),
	icon16: "images/filesystem.png",
	position: 40
});

OMV.WorkspaceManager.registerPanel({
	id: "filesystems",
	path: "/storage/filesystem",
	text: _("Filesystems"),
	position: 10,
	className: "OMV.module.admin.storage.filesystem.Filesystems"
});
