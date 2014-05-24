/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
// require("js/omv/form/field/Password.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.owncloud.Share
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.owncloud.Share", {
	extend: "OMV.workspace.window.Form",
	requires: [
	    "OMV.workspace.window.plugin.ConfigObject",
		"OMV.form.field.Password",
		"OMV.form.field.SharedFolderComboBox"
	],

	rpcService: "OwnCloud",
	rpcGetMethod: "getShare",
	rpcSetMethod: "setShare",
	plugins: [{
		ptype: "configobject"
	}],
	width: 700,

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
			plugins: [{
				ptype: "fieldinfo",
				text: _("The location of the files to share.")
			}],
			listeners: {
				scope: me,
				select: function(c, records, eOpts) {
					// Automatically set the share name, except it has
					// already been entered.
					var field = this.findField("name");
					if (Ext.isEmpty(field.getValue()))
						field.setValue(records[0].get("name"));
				}
			}
		},{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Folder name"),
			allowBlank: false,
			vtype: "sharename",
			plugins: [{
				ptype: "fieldinfo",
				text: _("The name of the share.")
			}]
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			vtype: "comment"
		}];
	}
});

/**
 * @class OMV.module.admin.service.owncloud.Shares
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.owncloud.Shares", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.owncloud.Share"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "0167afc4-366c-11e3-91e7-0002b3a176b4",
	columns: [{
		text: _("Shared folder"),
		sortable: true,
		dataIndex: "sharedfoldername",
		stateId: "sharedfoldername"
	},{
		text: _("Folder name"),
		sortable: true,
		dataIndex: "name",
		stateId: "name"
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
						{ name: "name", type: "string" },
						{ name: "comment", type: "string" },
						{ name: "ro", mapping: "options.ro", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "OwnCloud",
						method: "getShareList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "sharedfoldername"
				}]
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.owncloud.Share", {
			title: _("Add share"),
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
		Ext.create("OMV.module.admin.service.owncloud.Share", {
			title: _("Edit share"),
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
				service: "OwnCloud",
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
	path: "/service/owncloud",
	text: _("Shares"),
	position: 20,
	className: "OMV.module.admin.service.owncloud.Shares"
});
