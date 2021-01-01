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
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.clamav.OnAccessScan
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.clamav.OnAccessScan", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.form.field.SharedFolderComboBox",
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "ClamAV",
	rpcGetMethod: "getOnAccessPath",
	rpcSetMethod: "setOnAccessPath",
	plugins: [{
		ptype: "configobject"
	}],
	width: 400,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormItems: function() {
		return [{
			xtype: "checkbox",
			name: "enable",
			fieldLabel: _("Enable"),
			checked: true
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			fieldLabel: _("Shared folder"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The location of the files to scan on-access.")
			}]
		}];
	}
});

/**
 * @class OMV.module.admin.service.clamav.OnAccessScans
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.clamav.OnAccessScans", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.clamav.OnAccessScan"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "ff76df7a-8cf5-11e7-8a62-0800278dc04d",
	columns: [{
		xtype: "enabledcolumn",
		text: _("Enabled"),
		sortable: true,
		dataIndex: "enable",
		stateId: "enable"
	},{
		xtype: "textcolumn",
		text: _("Shared folder"),
		sortable: true,
		dataIndex: "sharedfoldername",
		stateId: "sharedfoldername"
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
						{ name: "enable", type: "boolean" },
						{ name: "sharedfoldername", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "ClamAV",
						method: "getOnAccessPathList"
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
		Ext.create("OMV.module.admin.service.clamav.OnAccessScan", {
			title: _("Add on-access scan"),
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
		Ext.create("OMV.module.admin.service.clamav.OnAccessScan", {
			title: _("Edit on-access scan"),
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
				service: "ClamAV",
				method: "deleteOnAccessPath",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "onaccessscans",
	path: "/service/clamav",
	text: _("On Access Scans"),
	position: 20,
	className: "OMV.module.admin.service.clamav.OnAccessScans"
});
