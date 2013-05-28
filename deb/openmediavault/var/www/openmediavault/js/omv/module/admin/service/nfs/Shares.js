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
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.nfs.Share
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.nfs.Share", {
	extend: "OMV.workspace.window.Form",
	uses: [
		"OMV.form.field.SharedFolderComboBox",
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "NFS",
	rpcGetMethod: "getShare",
	rpcSetMethod: "setShare",
	plugins: [{
		ptype: "configobject"
	}],
	height: 280,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			fieldLabel: _("Shared folder"),
			readOnly: (me.uuid !== OMV.UUID_UNDEFINED),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The location of the files to share. The share will be accessible at /export/<name>.")
			}]
		},{
			xtype: "textfield",
			name: "client",
			fieldLabel: _("Client"),
			allowBlank: false,
			vtype: "noBlank",
			plugins: [{
				ptype: "fieldinfo",
				text: _("Clients allowed to mount the filesystem, e.g. 192.168.178.0/24.")
			}]
		},{
			xtype: "combo",
			name: "options",
			fieldLabel: _("Privilege"),
			queryMode: "local",
			store: [
				[ "ro",_("Read only") ],
				[ "rw",_("Read/Write") ]
			],
			editable: false,
			triggerAction: "all",
			value: "ro"
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: _("Extra options"),
			allowBlank: true,
			value: "subtree_check,secure",
			plugins: [{
				ptype: "fieldinfo",
				text: _("Please check the <a href='http://linux.die.net/man/5/exports' target='_blank'>manual page</a> for more details.")
			}]
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true
		},{
			xtype: "hidden",
			name: "mntentref",
			value: OMV.UUID_UNDEFINED
		}];
	}
});

/**
 * @class OMV.module.admin.service.nfs.Shares
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.nfs.Shares", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.nfs.Share"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "4da5f715-4381-4c6b-8c83-ab23d284d0e3",
	columns: [{
		text: _("Shared folder"),
		sortable: true,
		dataIndex: "sharedfoldername",
		stateId: "sharedfoldername"
	},{
		text: _("Client"),
		sortable: true,
		dataIndex: "client",
		stateId: "client"
	},{
		text: _("Options"),
		sortable: true,
		stateId: "options",
		renderer: function(value, metaData, record, rowIndex, colIndex,
		  store, view) {
			value = record.get("options");
			var extraoptions = record.get("extraoptions");
			if(extraoptions.length > 0) {
				value = value + "," + extraoptions;
			}
			return value;
		}
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
					idProperty: "uuid",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "sharedfoldername", type: "string" },
						{ name: "client", type: "string" },
						{ name: "options", type: "string" },
						{ name: "extraoptions", type: "string" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "NFS",
						method: "getShareList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.nfs.Share", {
			  title: _("Add share"),
			  uuid: OMV.UUID_UNDEFINED,
			  listeners: {
				  scope: me,
				  submit: function() {
					  me.doReload();
				  }
			  }
		  }).show();
	},

	onEditButton: function() {
		var me = this;
		var selModel = this.getSelectionModel();
		var record = selModel.getSelection()[0];
		Ext.create("OMV.module.admin.service.nfs.Share", {
			  title: _("Edit share"),
			  uuid: record.get("uuid"),
			  listeners: {
				  scope: me,
				  submit: function() {
					  me.doReload();
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
				service: "NFS",
				method: "deleteShare",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "shares",
	path: "/service/nfs",
	text: _("Shares"),
	position: 20,
	className: "OMV.module.admin.service.nfs.Shares"
});
