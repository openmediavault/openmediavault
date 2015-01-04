/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
// require("js/omv/workspace/form/Panel.js")

/**
 * @class OMV.module.admin.service.smb.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.smb.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "SMB",
	rpcGetMethod: "getSettings",
	rpcSetMethod: "setSettings",

	getFormItems: function() {
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
				name: "workgroup",
				fieldLabel: _("Workgroup"),
				allowBlank: false,
				value: "WORKGROUP",
				plugins: [{
					ptype: "fieldinfo",
					text: _("The workgroup the server will appear to be in when queried by clients.")
				}]
			},{
				xtype: "textfield",
				name: "serverstring",
				fieldLabel: _("Description"),
				allowBlank: false,
				value: "%h server",
				plugins: [{
					ptype: "fieldinfo",
					text: _("The NT description field.")
				}]
			},{
				xtype: "checkbox",
				name: "localmaster",
				fieldLabel: _("Local master browser"),
				checked: true,
				boxLabel: _("Allow this server to try and become a local master browser")
			},{
				xtype: "checkbox",
				name: "timeserver",
				fieldLabel: _("Time server"),
				checked: false,
				boxLabel: _("Allow this server to advertise itself as a time server to Windows clients")
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
				boxLabel: _("Enable user home directories")
			},{
				xtype: "checkbox",
				name: "homesbrowseable",
				fieldLabel: _("Browseable"),
				checked: true,
				boxLabel: _("Set browseable"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("This controls whether this share is seen in the list of available shares in a net view and in the browse list.")
				}]
			}]
		},{
			xtype: "fieldset",
			title: _("WINS"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "winssupport",
				fieldLabel: _("WINS support"),
				checked: false,
				boxLabel: _("Enable WINS server"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("Act as a WINS server.")
				}]
			},{
				xtype: "textfield",
				name: "winsserver",
				fieldLabel: _("WINS server"),
				allowBlank: true,
				value: "",
				plugins: [{
					ptype: "fieldinfo",
					text: _("Use the specified WINS server.")
				}]
			}]
		},{
			xtype: "fieldset",
			title: _("Advanced settings"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "combo",
				name: "loglevel",
				fieldLabel: _("Log level"),
				queryMode: "local",
				store: Ext.create("Ext.data.ArrayStore", {
					fields: [ "value", "text" ],
					data: [
						[ 0, _("None") ],
						[ 1, _("Minimum") ],
						[ 2, _("Normal") ],
						[ 3, _("Full") ],
						[ 10, _("Debug") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 0
			},{
				xtype: "checkbox",
				name: "nullpasswords",
				fieldLabel: _("Null passwords"),
				checked: false,
				boxLabel: _("Allow client access to accounts that have null passwords")
			},{
				xtype: "checkbox",
				name: "usesendfile",
				fieldLabel: _("Use sendfile"),
				checked: true,
				boxLabel: _("Use the more efficient sendfile system call for files that are exclusively oplocked"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("This may make more efficient use of the system CPU's and cause Samba to be faster. Samba automatically turns this off for clients that use protocol levels lower than NT LM 0.12 and when it detects a client is Windows 9x.")
				}]
			},{
				xtype: "checkbox",
				name: "aio",
				fieldLabel: _("Asynchronous I/O"),
				checked: true,
				boxLabel: _("Enable asynchronous I/O (AIO) support.")
			},{
				xtype: "textarea",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Please check the <a href='http://www.samba.org/samba/docs/man/manpages-3/smb.conf.5.html' target='_blank'>manual page</a> for more details."),
				}]
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/smb",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.smb.Settings"
});
