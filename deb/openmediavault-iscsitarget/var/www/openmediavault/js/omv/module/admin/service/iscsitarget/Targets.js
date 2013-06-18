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
// require("js/omv/form/Panel.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/Tab.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/module/admin/service/iscsitarget/AuthUsers.js")

/**
 * @class OMV.module.admin.service.iscsitarget.target.General
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.iscsitarget.target.General", {
	extend: "OMV.form.Panel",

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			items: [{
				xtype: "textfield",
				name: "identifier",
				fieldLabel: _("Identifier"),
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The identifier of the target.")
				}]
			},{
				xtype: "textfield",
				name: "alias",
				fieldLabel: _("Alias"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The optional alias of the target.")
				}]
			},{
				xtype: "combo",
				name: "headerdigest",
				fieldLabel: _("Header digest"),
				queryMode: "local",
				store: [
					[ "None", _("None") ],
					[ "CRC32C", "CRC32C" ]
				],
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				plugins: [{
					ptype: "fieldinfo",
					text: _("If set to 'CRC32C' and the initiator is configured accordingly, the integrity of an iSCSI PDU's header segments will be protected by a CRC32C checksum."),
				}],
				value: "None"
			},{
				xtype: "combo",
				name: "datadigest",
				fieldLabel: _("Data digest"),
				queryMode: "local",
				store: [
					[ "None", _("None") ],
					[ "CRC32C", "CRC32C" ]
				],
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				plugins: [{
					ptype: "fieldinfo",
					text: _("If set to 'CRC32C' and the initiator is configured accordingly, the integrity of an iSCSI PDU's data segment will be protected by a CRC32C checksum."),
				}],
				value: "None"
			},{
				xtype: "numberfield",
				name: "maxconnections",
				fieldLabel: _("Max. connections"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The number of connections within a session."),
				}],
				value: 1
			},{
				xtype: "numberfield",
				name: "maxsessions",
				fieldLabel: _("Max. sessions"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The maximum number of sessions for this target."),
				}],
				value: 0
			},{
				xtype: "checkbox",
				name: "initialr2t",
				fieldLabel: _("Initial R2T"),
				checked: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("If enabled, the initiator has to wait for the target to solicit SCSI data before sending it. Disabling it allows the initiator to send a burst of N bytes unsolicited right after and/or together with the command. Thus disabling it may improve performance.")
				}]
			},{
				xtype: "checkbox",
				name: "immediatedata",
				fieldLabel: _("Immediate data"),
				checked: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("This allows the initiator to append unsolicited data to a command. To achieve better performance, this should be enabled.")
				}]
			},{
				xtype: "numberfield",
				name: "maxrecvdatasegmentlength",
				fieldLabel: _("Max. receive data segment length"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Sets the maximum data segment length that can be received."),
				}],
				value: 8192
			},{
				xtype: "numberfield",
				name: "maxxmitdatasegmentlength",
				fieldLabel: _("Max. transmit data segment length"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Sets the maximum data segment length that can be sent."),
				}],
				value: 8192
			},{
				xtype: "numberfield",
				name: "maxburstlength",
				fieldLabel: _("Max. burst length"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Sets the maximum amount of either unsolicited or solicited data the initiator may send in a single burst."),
				}],
				value: 262144
			},{
				xtype: "numberfield",
				name: "firstburstlength",
				fieldLabel: _("First burst length"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Sets the amount of unsolicited data the initiator may transmit in the first burst of a transfer either with and/or right after the command."),
				}],
				value: 65536
			},{
				xtype: "numberfield",
				name: "maxoutstandingr2t",
				fieldLabel: _("Max. outstanding R2T"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Controls the maximum number of data transfers the target may request at once."),
				}],
				value: 1
			},{
				xtype: "checkbox",
				name: "datapduinorder",
				fieldLabel: _("Data PDU in order"),
				checked: true
			},{
				xtype: "checkbox",
				name: "datasequenceinorder",
				fieldLabel: _("Data sequence in order"),
				checked: true
			},{
				xtype: "numberfield",
				name: "errorrecoverylevel",
				fieldLabel: _("Error recovery level"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 0
			},{
				xtype: "numberfield",
				name: "nopinterval",
				fieldLabel: _("NOP interval"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("If value is non-zero, the initiator will be 'ping'ed during phases of inactivity (i.e. no data transfers) every N seconds to verify the connection is still alive."),
				}],
				value: 0
			},{
				xtype: "numberfield",
				name: "noptimeout",
				fieldLabel: _("NOP timeout"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("If a non-zero 'NOP interval' is used to periodically 'ping' the initiator during phases of inactivity (i.e. no data transfers), the initiator must respond within N seconds, otherwise the connection will be closed."),
				}],
				value: 0
			},{
				xtype: "numberfield",
				name: "wthreads",
				fieldLabel: _("IO threads"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The number of threads to perform block I/O to the device."),
				}],
				value: 8
			},{
				xtype: "numberfield",
				name: "queuedcommands",
				fieldLabel: _("Queued commands"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The number of commands an initiator may send and that will be buffered by the target."),
				}],
				value: 32
			},{
				xtype: "textfield",
				name: "comment",
				fieldLabel: _("Comment"),
				allowBlank: true
			},{
				xtype: "textarea",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true
			},{
				xtype: "hidden",
				name: "activation"
			}]
		});
		me.callParent(arguments);
	}
});

/**
 * @class OMV.module.admin.service.iscsitarget.target.LUN
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.iscsitarget.target.LUN", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],

	mode: "local",
	width: 500,
	plugins: [{
		ptype: "configobject"
	}],

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "combo",
			name: "type",
			fieldLabel: _("Transfer mode"),
			queryMode: "local",
			store: [
//				[ "fileio", _("File IO") ],
				[ "blockio", _("Block IO") ]
			],
			allowBlank: false,
			editable: false,
			readOnly: me.isNew(),
			triggerAction: "all",
			value: "blockio"
		},{
			xtype: "combo",
			name: "devicefile",
			fieldLabel: _("Device"),
			emptyText: _("Select an device ..."),
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
			readOnly: me.isNew(),
			triggerAction: "all"
		},{
			xtype: "textfield",
			name: "scsiid",
			fieldLabel: _("SCSI Id."),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Assign a unique identifier to the iSCSI volume optionally.")
			}]
		},{
			xtype: "textfield",
			name: "scsisn",
			fieldLabel: _("SCSI serial no."),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Assign a unique serial number to the iSCSI volume optionally.")
			}]
		},{
			xtype: "combo",
			name: "iomode",
			fieldLabel: _("R/W mode"),
			queryMode: "local",
			store: [
				[ "wt", _("Write-through") ],
				[ "wb", _("Write-back") ],
				[ "ro", _("Read-only") ]
			],
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "wt"
		},{
			xtype: "hidden",
			name: "id"
		}];
	}
});

/**
 * @class OMV.module.admin.service.iscsitarget.target.LUNs
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.iscsitarget.target.LUNs", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.util.Format"
	],
	uses: [
		"OMV.module.admin.service.iscsitarget.target.LUN"
	],

	mode: "local",
	stateful: true,
	stateId: "3107db90-c1e9-11e0-90c8-00221568ca88",
	columns: [{
		text: _("Id"),
		sortable: true,
		dataIndex: "id",
		stateId: "id"
	},{
		text: _("Device"),
		sortable: true,
		dataIndex: "devicefile",
		stateId: "devicefile"
	},{
		text: _("SCSI Id."),
		sortable: true,
		dataIndex: "scsiid",
		stateId: "scsiid"
	},{
		text: _("SCSI serial no."),
		sortable: true,
		dataIndex: "scsisn",
		stateId: "scsisn"
	},{
		text: _("R/W mode"),
		sortable: true,
		dataIndex: "iomode",
		stateId: "iomode",
		renderer: OMV.util.Format.arrayRenderer([
			[ "wt", _("Write-through") ],
			[ "wb", _("Write-back") ],
			[ "ro", _("Read-only") ]
		])
	},{
		text: _("Transfer mode"),
		sortable: true,
		dataIndex: "type",
		stateId: "type",
		renderer: OMV.util.Format.arrayRenderer([
			[ "fileio", _("File IO") ],
			[ "blockio", _("Block IO") ]
		])
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: false,
				model: OMV.data.Model.createImplicit({
					idProperty: "uuid",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "id", type: "string" },
						{ name: "devicefile", type: "string" },
						{ name: "iomode", type: "string" },
						{ name: "type", type: "string" },
						{ name: "scsiid", type: "string" },
						{ name: "scsisn", type: "string" }
					]
				}),
				proxy: {
					type: "memory",
					reader: {
						type: "json"
					}
				},
				sorters: [{
					direction: "ASC",
					property: "id"
				}]
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		Ext.create("OMV.module.admin.service.iscsitarget.target.LUN", {
			title: _("Add LUN"),
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				scope: me,
				submit: function(wnd, values) {
					var store = this.getStore();
					// Update the row number.
					var lastRowNum = store.getCount();
					values.id = lastRowNum;
					// Create and insert new record.
					var newRecord = new store.model;
					newRecord.beginEdit();
					newRecord.set(values);
					newRecord.endEdit();
					store.insert(lastRowNum, [ newRecord ]);
				}
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		var wnd = Ext.create("OMV.module.admin.service.iscsitarget.target.LUN", {
			title: _("Edit LUN"),
			uuid: record.get("uuid"),
			listeners: {
				scope: me,
				submit: function(wnd, values) {
					record.beginEdit();
					record.set(values);
					record.endEdit();
				}
			}
		});
		wnd.setValues(record.data);
		wnd.show();
	},

	afterDeletion: function() {
		var me = this;
		// Renumber all configured LUNs.
		me.store.each(function(record, index) {
			record.beginEdit();
			record.set("id", index);
			record.endEdit();
		});
	},

	isValid: function() {
		var me = this;
		return (me.store.getCount() > 0);
	},

	setValues: function(values) {
		var me = this;
		return me.callParent(values.luns.lun);
	},

	getValues: function() {
		var me = this;
		var values = me.callParent(arguments);
		return {
			luns: values
		};
	}
});

/**
 * @class OMV.module.admin.service.iscsitarget.target.Target
 * @derived OMV.workspace.window.Tab
 */
Ext.define("OMV.module.admin.service.iscsitarget.target.Target", {
	extend: "OMV.workspace.window.Tab",
	requires: [
		"OMV.module.admin.service.iscsitarget.AuthUsers",
		"OMV.module.admin.service.iscsitarget.target.General",
		"OMV.module.admin.service.iscsitarget.target.LUNs"
	],

	rpcService: "iSCSITarget",
	rpcGetMethod: "getTarget",
	rpcSetMethod: "setTarget",
	plugins: [{
		ptype: "configobject"
	}],
	width: 600,
	height: 450,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getTabItems: function() {
		return [
			Ext.create("OMV.module.admin.service.iscsitarget.target.General"),
			Ext.create("OMV.module.admin.service.iscsitarget.AuthUsers", {
				getValues: function() {
					var me = this;
					var values = me.callParent(arguments);
					return {
						authentication: values
					};
				},
				setValues: function(values) {
					var me = this;
					return me.callParent(values.authentication.user);
				}
			}),
			Ext.create("OMV.module.admin.service.iscsitarget.target.LUNs")
		]
	}
});

/**
 * @class OMV.module.admin.service.iscsitarget.target.Targets
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.iscsitarget.target.Targets", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.iscsitarget.target.Target"
	],

	stateful: true,
	stateId: "15e18b72-c1e9-11e0-a91c-00221568ca88",
	columns: [{
		text: _("IQN"),
		sortable: true,
		dataIndex: "iqn",
		stateId: "iqn"
	},{
		text: _("Alias"),
		sortable: true,
		dataIndex: "alias",
		stateId: "alias"
	},{
		text: _("Max. connections"),
		sortable: true,
		dataIndex: "maxconnections",
		stateId: "maxconnections"
	},{
		text: _("Comment"),
		sortable: true,
		dataIndex: "comment",
		stateId: "comment"
	}],

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "devicefile",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "iqn", type: "string" },
						{ name: "alias", type: "string" },
						{ name: "maxconnections", type: "int" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "iSCSITarget",
						method: "getTargetList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.iscsitarget.target.Target", {
			title: _("Add target"),
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.service.iscsitarget.target.Target", {
			title: _("Edit target"),
			uuid: record.get("uuid"),
			listeners: {
				scope: me,
				submit: function() {
					this.doReload();
				}
			}
		}).show();
	},

	doDeletion: function(record) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "iSCSITarget",
				method: "deleteTarget",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "targets",
	path: "/service/iscsitarget",
	text: _("Targets"),
	position: 20,
	className: "OMV.module.admin.service.iscsitarget.target.Targets"
});
