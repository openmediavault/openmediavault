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
// require("js/omv/PluginMgr.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/DiagPanel.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Services.AFP");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "afp", {
	text: "Apple Filing",
	icon: "images/afp.png"
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
			title: "General settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "enable",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: "Extra options",
				allowBlank: true,
				width: 400
			}]
		},{
			xtype: "fieldset",
			title: "Authentication",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "allowguests",
				fieldLabel: "Allow guest logins",
				checked: false,
				inputValue: 1
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "afp", {
	cls: OMV.Module.Services.AFP.SettingsPanel,
	title: "Settings",
	position: 10
});

/**
 * @class OMV.Module.Services.AFP.SharesGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.AFP.SharesGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "b2878122-c1e7-11e0-9bbc-00221568ca88",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Shared folder",
				sortable: true,
				dataIndex: "sharedfoldername",
				id: "sharedfoldername"
			},{
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
				header: "Read only",
				sortable: true,
				dataIndex: "readonly",
				id: "readonly",
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
			remoteSort: false,
			proxy: new OMV.data.DataProxy("AFP", "getShareList"),
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

	cbAddBtnHdl : function() {
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

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "AFP", "deleteShare",
		  [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "afp", {
	cls: OMV.Module.Services.AFP.SharesGridPanel,
	title: "Shares",
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
		title: ((config.uuid == OMV.UUID_UNDEFINED) ?
		  "Add" : "Edit") + " share",
		width: 700,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.AFP.SharePropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.AFP.SharePropertyDialog, OMV.CfgObjectDialog, {
	getFormConfig : function() {
		return {
			autoScroll: true,
			defaults: {
				anchor: "-" + Ext.getScrollBarWidth(),
				labelSeparator: ""
			}
		};
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "name",
			fieldLabel: "Name",
			allowBlank: false,
			vtype: "sharename",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The name of the share."
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true
		},{
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			hiddenName: "sharedfolderref",
			fieldLabel: "Shared folder",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The location of the files to share."
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: "Password",
			allowBlank: true,
			maxLength: 8,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If this option is set, then a password is required to access the share."
		},{
			xtype: "checkbox",
			name: "ro",
			fieldLabel: "Read only",
			checked: false,
			inputValue: 1,
			boxLabel: "Set read only.",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "If this option is set, then users may not create or modify files in the share."
		},{
			xtype: "checkbox",
			name: "tm",
			fieldLabel: "Time Machine support",
			checked: false,
			inputValue: 1,
			boxLabel: "Enable Time Machine support for this share."
		},{
			xtype: "checkbox",
			name: "upriv",
			fieldLabel: "Unix privileges",
			checked: true,
			inputValue: 1,
			boxLabel: "Use AFP3 unix privileges."
		},{
			xtype: "checkbox",
			name: "usedots",
			fieldLabel: "Use dots",
			checked: true,
			inputValue: 1,
			boxLabel: "Don't do :hex translation for dot files.",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "When this option gets set, certain file names become illegal. These are .Parent and anything that starts with .Apple."
		},{
			xtype: "checkbox",
			name: "invisibledots",
			fieldLabel: "Hide dot files",
			checked: false,
			inputValue: 1,
			boxLabel: "Make dot files invisible."
		},{
			xtype: "checkbox",
			name: "mswindows",
			fieldLabel: "Forces filename restrictions",
			checked: false,
			inputValue: 1,
			boxLabel: "This forces filenames to be restricted to the character set used by Windows."
		},{
			xtype: "combo",
			name: "casefold",
			hiddenName: "casefold",
			fieldLabel: "Case folding",
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "none","None" ],
					[ "tolower","Lowercases names in both directions" ],
					[ "toupper","Uppercases names in both directions" ],
					[ "xlatelower","Client sees lowercase, server sees uppercase" ],
					[ "xlateupper","Client sees uppercase, server sees lowercase" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "none",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "This option handles, if the case of filenames should be changed."
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: "Extra options",
			allowBlank: true
		}];
	}
});
