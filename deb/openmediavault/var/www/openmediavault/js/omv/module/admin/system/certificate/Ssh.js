/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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
// require("js/omv/form/field/SshPublicKey.js")

/**
 * @class OMV.module.admin.system.certificate.ssh.Edit
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.system.certificate.ssh.Edit", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.form.field.SshPublicKey",
	    "OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "CertificateMgmt",
	rpcGetMethod: "getSsh",
	rpcSetMethod: "setSsh",
	plugins: [{
		ptype: "configobject"
	}],
	width: 630,

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormConfig: function() {
		return {
			layout: {
				type: "vbox",
				align: "stretch"
			}
		};
	},

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "textarea",
			name: "privatekey",
			fieldLabel: _("Private key"),
			cls: "x-form-textarea-monospaced",
			allowBlank: false,
			// Hide and do not submit/validate this field when an existing
			// certificate is processed.
			hidden: !me.isNew(),
			disabled: !me.isNew(),
			height: 150,
			flex: 1,
			plugins: [{
				ptype: "fieldinfo",
				text: _("The private RSA key in X.509 PEM format.")
			}]
		},{
			xtype: "sshpublickeyfield",
			name: "publickey",
			fieldLabel: _("Public key"),
			allowBlank: false,
			// Set this field to read-only when an existing certificate
			// is processed.
			readOnly: !me.isNew(),
			editable: me.isNew(),
			plugins: [{
				ptype: "fieldinfo",
				text: _("The RSA public key in OpenSSH format.")
			}]
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment")
		}];
	}
});

/**
 * @class OMV.module.admin.system.certificate.ssh.Certificates
 * @derived OMV.workspace.grid.Panel
 * Display list of installed SSH certificates.
 */
Ext.define("OMV.module.admin.system.certificate.ssh.Certificates", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.system.certificate.ssh.Edit"
	],

	hidePagingToolbar: false,
	stateful: true,
	stateId: "bdd0b2ca-1016-11e5-be00-0002b3a176b4",
	columns: [{
		xtype: "textcolumn",
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
						{ name: "_used", type: "boolean" },
						{ name: "uuid", type: "string" },
						{ name: "comment", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "CertificateMgmt",
						method: "getSshList"
					}
				},
				remoteSort: true,
				sorters: [{
					direction: "ASC",
					property: "comment"
				}]
			})
		});
		me.callParent(arguments);
	},

	getTopToolbarItems: function() {
		var me = this;
		var items = me.callParent(arguments);
		// Replace the default 'Add' button.
		Ext.Array.erase(items, 0, 1);
		Ext.Array.insert(items, 0, [{
			id: me.getId() + "-add",
			xtype: "splitbutton",
			text: _("Add"),
			icon: "images/add.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
			handler: function() {
				this.showMenu();
			},
			menu: Ext.create("Ext.menu.Menu", {
				items: [
					{ text: _("Create"), value: "create" },
					{ text: _("Import"), value: "import" }
				],
				listeners: {
					scope: me,
					click: function(menu, item, e, eOpts) {
						this.onAddButton(item.value);
					}
				}
			})
		}]);
		return items;
	},

	onAddButton: function(action) {
		var me = this;
		var className, title;
		switch (action) {
		case "import":
			Ext.create("OMV.module.admin.system.certificate.ssh.Edit", {
				title: _("Import SSH certificate"),
				uuid: OMV.UUID_UNDEFINED,
				listeners: {
					scope: me,
					submit: function() {
						this.doReload();
					}
				}
			}).show();
			break;
		case "create":
			Ext.create("OMV.workspace.window.Form", {
				title: "Create SSH certificate",
				mode: "local",
				hideResetButton: true,
				closeIfNotDirty: false,
				formItems: [{
					xtype: "textfield",
					name: "comment",
					fieldLabel: _("Comment"),
					allowBlank: false
				}],
				listeners: {
					scope: me,
					submit: function(c, values) {
						OMV.Rpc.request({
							scope: me,
							callback: function(id, success, response) {
								this.doReload();
							},
							relayErrors: false,
							rpcData: {
								service: "CertificateMgmt",
								method: "createSsh",
								params: {
									comment: values.comment
								}
							}
						});
					}
				}
			}).show();
			break;
		}
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		Ext.create("OMV.module.admin.system.certificate.ssh.Edit", {
			title: _("Edit SSH certificate"),
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
				  service: "CertificateMgmt",
				  method: "deleteSsh",
				  params: {
					  uuid: record.get("uuid")
				  }
			  }
		  });
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "ssh",
	path: "/system/certificate",
	text: _("SSH"),
	position: 10,
	className: "OMV.module.admin.system.certificate.ssh.Certificates"
});
