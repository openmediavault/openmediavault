/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
 * @class OMV.module.admin.service.afp.Share
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.afp.Share", {
	extend: "OMV.workspace.window.Form",
	requires: [
	    "OMV.workspace.window.plugin.ConfigObject",
		"OMV.form.field.Password",
		"OMV.form.field.SharedFolderComboBox"
	],

	rpcService: "AFP",
	rpcGetMethod: "getShare",
	rpcSetMethod: "setShare",
	plugins: [{
		ptype: "configobject"
	}],
	width: 700,
	height: 400,

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
				text: _("The location of the files to share.")
			}]
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
			allowBlank: true,
			vtype: "comment"
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: _("Password"),
			allowBlank: true,
			maxLength: 8,
			plugins: [{
				ptype: "fieldinfo",
				text: _("If this option is set, then a password is required to access the share.")
			}]
		},{
			xtype: "checkbox",
			name: "ro",
			fieldLabel: _("Read only"),
			checked: false,
			boxLabel: _("Set read only."),
			plugins: [{
				ptype: "fieldinfo",
				text: _("If this option is set, then users may not create or modify files in the share.")
			}]
		},{
			xtype: "checkbox",
			name: "allowguest",
			fieldLabel: _("Guest login"),
			checked: false,
			boxLabel: _("Allow guest login."),
			plugins: [{
				ptype: "fieldinfo",
				text: _("Guests only have read access by default.")
			}]
		},{
			xtype: "checkbox",
			name: "guestrw",
			fieldLabel: _("Guest permissions"),
			checked: false,
			boxLabel: _("Allow guests to create or modify files.")
		},{
			xtype: "checkbox",
			name: "tm",
			fieldLabel: _("Time Machine support"),
			checked: false,
			boxLabel: _("Enable Time Machine support for this share.")
		},{
			xtype: "checkbox",
			name: "upriv",
			fieldLabel: _("Unix privileges"),
			checked: true,
			boxLabel: _("Use AFP3 unix privileges.")
		},{
			xtype: "checkbox",
			name: "invisibledots",
			fieldLabel: _("Hide dot files"),
			checked: false,
			boxLabel: _("Make dot files invisible.")
		},{
			xtype: "combo",
			name: "casefold",
			fieldLabel: _("Case folding"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: [
					[ "none", _("None") ],
					[ "tolower", _("Lowercases names in both directions") ],
					[ "toupper", _("Uppercases names in both directions") ],
					[ "xlatelower", _("Client sees lowercase, server sees uppercase") ],
					[ "xlateupper", _("Client sees uppercase, server sees lowercase") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "none",
			plugins: [{
				ptype: "fieldinfo",
				text: _("This option handles, if the case of filenames should be changed.")
			}]
		},{
			xtype: "numberfield",
			name: "volsizelimit",
			fieldLabel: _("Quota"),
			minValue: 0,
			allowDecimals: false,
			allowBlank: false,
			value: 0,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Limit the reported volume size to the given value in MiB, thus preventing TM from using the whole disk space for backup. Set this value to 0 to disable this option.")
			}]
		},{
			xtype: "textarea",
			name: "extraoptions",
			fieldLabel: _("Extra options"),
			allowBlank: true,
			plugins: [{
				ptype: "fieldinfo",
				text: _("Please check the <a href='http://netatalk.sourceforge.net/3.0/htmldocs/afp.conf.5.html' target='_blank'>manual page</a> for more details."),
			}]
		}];
	}
});

/**
 * @class OMV.module.admin.service.afp.Shares
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.afp.Shares", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.afp.Share"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "b2878122-c1e7-11e0-9bbc-00221568ca88",
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
	},{
		xtype: "textcolumn",
		text: _("Comment"),
		sortable: true,
		dataIndex: "comment",
		stateId: "comment"
	},{
		xtype: "booleantextcolumn",
		text: _("Read only"),
		sortable: true,
		dataIndex: "readonly",
		stateId: "readonly"
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
						{ name: "sharedfoldername", type: "string" },
						{ name: "comment", type: "string" },
						{ name: "ro", mapping: "options.ro", type: "boolean" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "AFP",
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
		Ext.create("OMV.module.admin.service.afp.Share", {
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
		Ext.create("OMV.module.admin.service.afp.Share", {
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
				service: "AFP",
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
	path: "/service/afp",
	text: _("Shares"),
	position: 20,
	className: "OMV.module.admin.service.afp.Shares"
});
