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
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/workspace/grid/Panel.js")

/**
 * @class OMV.module.admin.service.iscsitarget.AuthUser
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.iscsitarget.AuthUser", {
	extend: "OMV.workspace.window.Form",

	mode: "local",
	width: 500,
	plugins: [{
		ptype: "configobject"
	}],

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormItems: function() {
		var me = this;
		return [{
			xtype: "combo",
			name: "type",
			fieldLabel: _("Transfer mode"),
			queryMode: "local",
			store: [
				[ "incoming", _("Incoming") ],
				[ "outgoing", _("Outgoing") ]
			],
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			readOnly: me.isNew(),
			value: "incoming"
		},{
			xtype: "textfield",
			name: "username",
			fieldLabel: _("Username"),
			allowBlank: false,
			vtype: "username",
			readOnly: me.isNew()
		},{
			xtype: "passwordfield",
			name: "password",
			fieldLabel: _("Password"),
			allowBlank: true
		}];
	}
});

/**
 * @class OMV.module.admin.service.iscsitarget.AuthUsers
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.iscsitarget.AuthUsers", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format"
	],
	uses: [
		"OMV.module.admin.service.iscsitarget.AuthUser"
	],

	mode: "local",
	hideAddButton: false,
	hideEditButton: false,
	hideDeleteButton: false,
	hideRefreshButton: true,
	hidePagingToolbar: false,
	stateful: true,
	stateId: "54af0f59-ef4e-4c1f-a3c3-1efaa1ea02fd",
	columns: [{
		text: _("Type"),
		sortable: true,
		dataIndex: "type",
		stateId: "type",
		renderer: OMV.util.Format.arrayRenderer([
			[ "incoming", _("Incoming") ],
			[ "outgoing", _("Outgoing") ]
		])
	},{
		text: _("Username"),
		sortable: true,
		dataIndex: "username",
		stateId: "username"
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
						{ name: "type", type: "string" },
						{ name: "username", type: "string" },
						{ name: "password", type: "string" }
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
					property: "username"
				}]
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.iscsitarget.AuthUser", {
			title: _("Add user"),
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				scope: me,
				submit: function(wnd, values) {
					var store = this.getStore();
					// Do some checks before adding the new user.
					if(values.type === "outgoing") {
						var qr = store.query("type", "outgoing");
						if(qr.getCount() > 0) {
							OMV.MessageBox.failure(null, _("Only one outgoing user per target is supported."));
							return;
						}
					}
					var newRecord = new store.model;
					newRecord.beginEdit();
					newRecord.set(values);
					newRecord.endEdit();
					store.add(newRecord);
				}
			}
		}).show();
	},

	onEditButton: function() {
		var me = this;
		var record = me.getSelected();
		var wnd = Ext.create("OMV.module.admin.service.iscsitarget.AuthUser", {
			title: _("Edit user"),
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
		wnd.setValues(record.getData());
		wnd.show();
	}
});
