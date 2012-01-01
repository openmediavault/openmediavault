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
// require("js/omv/MessageBox.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/FormPanelDialog.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/CfgObjectDialog.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.System");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "certificates", {
	text: "Certificates",
	icon: "images/certificate.png",
	position: 60
});

/**
 * @class OMV.Module.System.CertificateGridPanel
 * @derived OMV.grid.TBarGridPanel
 * Display list of installed certificates.
 */
OMV.Module.System.CertificateGridPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		stateId: "af67e357-d388-4b92-a6d1-076f834c1a0f",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Name",
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: "Valid to",
				sortable: true,
				dataIndex: "validto",
				id: "validto",
				renderer: OMV.util.Format.localeTimeRenderer()
			},{
				header: "Comment",
				sortable: true,
				dataIndex: "comment",
				id: "comment"
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.CertificateGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.CertificateGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("CertificateMgmt", "getList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "_used" },
					{ name: "uuid" },
					{ name: "comment" },
					{ name: "name" },
					{ name: "validto" }
    			]
			})
		});
		OMV.Module.System.CertificateGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.System.CertificateGridPanel.superclass.
		  initToolbar.apply(this);
		// Add 'Detail' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-detail",
			xtype: "button",
			text: "Detail",
			icon: "images/detail.png",
			handler: this.cbDetailBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.System.CertificateGridPanel.superclass.
		  cbSelectionChangeHdl.apply(this, arguments);
		// Process additional buttons
		var tbarBtnName = [ "detail" ];
		var tbarBtnDisabled = {
			"detail": true
		};
		var records = model.getSelections();
		if (records.length <= 0) {
			tbarBtnDisabled["detail"] = true;
		} else if (records.length == 1) {
			tbarBtnDisabled["detail"] = false;
		} else {
			tbarBtnDisabled["detail"] = true;
		}
		for (var i = 0; i < tbarBtnName.length; i++) {
			var tbarBtnCtrl = this.getTopToolbar().findById(this.getId() +
			  "-" + tbarBtnName[i]);
			if (!Ext.isEmpty(tbarBtnCtrl)) {
				if (true == tbarBtnDisabled[tbarBtnName[i]]) {
					tbarBtnCtrl.disable();
				} else {
					tbarBtnCtrl.enable();
				}
			}
		}
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.System.CertificatePropertyDialog({
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
		var wnd = new OMV.Module.System.CertificatePropertyDialog({
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

	cbDetailBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.System.CertificateDetailDialog({
			rpcGetParams: [ record.get("uuid") ]
		});
		wnd.show();
	},

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "CertificateMgmt",
		  "delete", [ record.get("uuid") ]);
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "certificates", {
	cls: OMV.Module.System.CertificateGridPanel
});

/**
 * @class OMV.Module.System.CertificatePropertyDialog
 */
OMV.Module.System.CertificatePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "CertificateMgmt",
		rpcGetMethod: "get",
		rpcSetMethod: "set",
		title: ((config.uuid == OMV.UUID_UNDEFINED) ? "Add" : "Edit") +
		  " SSL certificate",
		width: 650,
		height: 440
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.CertificatePropertyDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.CertificatePropertyDialog, OMV.CfgObjectDialog, {
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
			xtype: "textarea",
			name: "privatekey",
			fieldLabel: "Private key",
			cls: "x-form-textarea-monospaced",
			allowBlank: false,
			height: 150,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Paste an private RSA key in X.509 PEM format here."
		},{
			xtype: "textarea",
			name: "certificate",
			fieldLabel: "Certificate",
			cls: "x-form-textarea-monospaced",
			allowBlank: false,
			height: 150,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: "Paste a RSA certificate in X.509 PEM format here."
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: "Comment",
			maxLength: 65,
			allowBlank: false
		}];
	}
});

/**
 * @class OMV.Module.System.CertificateDetailDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.System.CertificateDetailDialog = function(config) {
	var initialConfig = {
		hideOk: true,
		hideCancel: true,
		hideClose: false,
		hideReset: true,
		rpcService: "CertificateMgmt",
		rpcGetMethod: "getDetail",
		title: "Certificate details",
		width: 600,
		height: 400
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.CertificateDetailDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.CertificateDetailDialog, OMV.FormPanelDialog, {
	initForm : function() {
		return new Ext.Panel({
			layout: "fit",
			border: false,
			items: [{
				id: this.getId() + "-detail",
				xtype: "textarea",
				name: "detail",
				readOnly: true,
				cls: "x-form-textarea-monospaced",
				disabledClass: ""
			}]
		});
	},

	cbOkBtnHdl : function() {
		this.close();
	},

	cbLoadHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			var cmp = Ext.getCmp(this.getId() + "-detail");
			if (!Ext.isEmpty(cmp)) {
				cmp.setValue(response);
			}
		} else {
			OMV.MessageBox.error(null, error);
		}
	}
});
