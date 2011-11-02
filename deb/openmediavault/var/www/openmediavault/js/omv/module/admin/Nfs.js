/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "nfs", {
	text: "NFS",
	icon: "images/nfs.png"
});

/**
 * @class OMV.Module.Services.NFSSettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.NFSSettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "NFS",
 		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NFSSettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.NFSSettingsPanel, OMV.FormPanelExt, {
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
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "nfs", {
	cls: OMV.Module.Services.NFSSettingsPanel,
	title: "Settings",
	position: 10
});

/**
 * @class OMV.Module.Services.NFSSharesGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Services.NFSSharesGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "4da5f715-4381-4c6b-8c83-ab23d284d0e3",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Shared folder",
				sortable: true,
				dataIndex: "sharedfoldername",
				id: "sharedfoldername"
			},{
				header: "Client",
				sortable: true,
				dataIndex: "client",
				id: "client"
			},{
				header: "Options",
				sortable: true,
				id: "options",
				renderer: this.optionsRenderer.createDelegate(this)
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NFSSharesGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.NFSSharesGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("NFS", "getShareList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "sharedfoldername" },
					{ name: "client" },
					{ name: "options" },
					{ name: "extraoptions" },
					{ name: "comment" }
    			]
			})
		});
		OMV.Module.Services.NFSSharesGridPanel.superclass.initComponent.apply(
		  this, arguments);
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Services.NFSSharePropertyDialog({
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
		var wnd = new OMV.Module.Services.NFSSharePropertyDialog({
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
		OMV.Ajax.request(this.cbDeletionHdl, this, "NFS", "deleteShare",
		  [ record.get("uuid") ]);
	},

	optionsRenderer : function(val, cell, record, row, col, store) {
		val = record.get("options");
		var extraoptions = record.get("extraoptions");
		if (extraoptions.length > 0) {
			val = val + "," + extraoptions;
		}
		return val;
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "nfs", {
	cls: OMV.Module.Services.NFSSharesGridPanel,
	title: "Shares",
	position: 20
});

/**
 * @class OMV.Module.Services.NFSSharePropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.Services.NFSSharePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "NFS",
		rpcGetMethod: "getShare",
		rpcSetMethod: "setShare",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " share",
		autoHeight: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.NFSSharePropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.NFSSharePropertyDialog,
  OMV.CfgObjectDialog, {
	initComponent : function() {
		OMV.Module.Services.NFSSharePropertyDialog.superclass.initComponent.apply(
		  this, arguments);
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
			xtype: "sharedfoldercombo",
			name: "sharedfolderref",
			hiddenName: "sharedfolderref",
			fieldLabel: "Shared folder",
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "The location of the files to share. The share will be accessible at /export/<name>."
		},{
			xtype: "textfield",
			name: "client",
			fieldLabel: "Client",
			allowBlank: false,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Clients allowed to mount the filesystem, e.g. 192.168.178.0/24."
		},{
			xtype: "combo",
			name: "options",
			hiddenName: "options",
			fieldLabel: "Privilege",
			mode: "local",
			store: [
				["ro","Read only"],
				["rw","Read/Write"]
			],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: "ro"
		},{
			xtype: "textfield",
			name: "extraoptions",
			fieldLabel: "Extra options",
			allowBlank: true,
			value: "subtree_check,secure"
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: "Comment",
			allowBlank: true
		},{
			xtype: "hidden",
			name: "mntentref",
			value: OMV.UUID_UNDEFINED
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		var field = this.findFormField("sharedfolderref");
		if ((this.uuid !== OMV.UUID_UNDEFINED) && Ext.isDefined(field)) {
			field.setReadOnly(true);
		}
	}
});
