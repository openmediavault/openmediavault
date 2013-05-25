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
// require("js/omv/ModuleManager.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/form/field/SharedFolderComboBox.js")
// require("js/omv/form/field/plugin/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Services.AFP");

// Register the menu.
OMV.ModuleManager.registerMenu("services", "afp", {
	text: _("Apple Filing"),
	icon16: "images/afp.png"
});

/**
 * @class OMV.Module.Services.AFP.SettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.AFP.SettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "AFP",
 		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.AFP.SettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.AFP.SettingsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("General settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: _("Enable"),
				checked: false
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true
			}]
		},{
			xtype: "fieldset",
			title: _("Home directories"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "homesenable",
				fieldLabel: _("Enable"),
				checked: false,
				boxLabel: _("Enable user home directories.")
			}]
		},{
			xtype: "fieldset",
			title: _("Advanced settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "allowclrtxt",
				fieldLabel: _("Allow plain passwords"),
				checked: false,
				boxLabel: _("Allow logins with passwords transmitted in the clear.")
			}]
		}];
	}
});
OMV.ModuleManager.registerPanel("services", "afp", {
	cls: OMV.Module.Services.AFP.SettingsPanel,
	title: _("Settings"),
	position: 10
});

/**
 * @class OMV.Module.Services.AFP.SharesGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.AFP.SharesGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateful: true,
		stateId: "b2878122-c1e7-11e0-9bbc-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				text: _("Shared folder"),
				sortable: true,
				dataIndex: "sharedfoldername",
				stateId: "sharedfoldername"
			},{
				text: _("Name"),
				sortable: true,
				dataIndex: "name",
				stateId: "name"
			},{
				text: _("Comment"),
				sortable: true,
				dataIndex: "comment",
				stateId: "comment"
			},{
				text: _("Read only"),
				sortable: true,
				dataIndex: "readonly",
				stateId: "readonly",
				renderer: OMV.util.Format.booleanRenderer()
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.AFP.SharesGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.AFP.SharesGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			proxy: new OMV.data.DataProxy({
				"rpcOptions": {
					"rpcData": {
						"service": "AFP",
						"method": "getShareList"
					}
				}
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "sharedfoldername" },
					{ name: "name" },
					{ name: "comment" },
					{ name: "ro", mapping: "options.ro" }
    			]
			})
		});
		OMV.Module.Services.AFP.SharesGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	onAddButton : function() {
		var wnd = new OMV.Module.Services.AFP.SharePropertyDialog({
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

	onEditButton : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelection()[0];
		var wnd = new OMV.Module.Services.AFP.SharePropertyDialog({
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

	doDeletion : function(record) {
		OMV.Ajax.request({
			  "scope": this,
			  "callback": this.onDeletion,
			  "rpcData": {
				  "service": "AFP",
				  "method": "deleteShare",
				  "params": {
					  "uuid": record.get("uuid")
				  }
			  }
		  });
	}
});
OMV.ModuleManager.registerPanel("services", "afp", {
	cls: OMV.Module.Services.AFP.SharesGridPanel,
	title: _("Shares"),
	position: 20
});

/**
 * @class OMV.Module.Services.AFP.SharePropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.AFP.SharePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "AFP",
		rpcGetMethod: "getShare",
		rpcSetMethod: "setShare",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Add share") : _("Edit share"),
		width: 700,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.AFP.SharePropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.AFP.SharePropertyDialog, OMV.CfgObjectDialog, {
	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: _("Name"),
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
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			fieldLabel: _("Shared folder"),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The location of the files to share.")
			}]
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
			fieldLabel: _(" "),
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
			name: "usedots",
			fieldLabel: _("Use dots"),
			checked: true,
			boxLabel: _("Don't do :hex translation for dot files."),
			plugins: [{
				ptype: "fieldinfo",
				text: _("When this option gets set, certain file names become illegal. These are .Parent and anything that starts with .Apple.")
			}]
		},{
			xtype: "checkbox",
			name: "invisibledots",
			fieldLabel: _("Hide dot files"),
			checked: false,
			boxLabel: _("Make dot files invisible.")
		},{
			xtype: "checkbox",
			name: "mswindows",
			fieldLabel: _("Forces filename restrictions"),
			checked: false,
			boxLabel: _("This forces filenames to be restricted to the character set used by Windows.")
		},{
			xtype: "combo",
			name: "casefold",
			fieldLabel: _("Case folding"),
			queryMode: "local",
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value","text" ],
				data: [
					[ "none",_("None") ],
					[ "tolower",_("Lowercases names in both directions") ],
					[ "toupper",_("Uppercases names in both directions") ],
					[ "xlatelower",_("Client sees lowercase, server sees uppercase") ],
					[ "xlateupper",_("Client sees uppercase, server sees lowercase") ]
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
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: _("Extra options"),
			allowBlank: true
		}];
	}
});
