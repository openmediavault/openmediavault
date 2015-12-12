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
 * @class OMV.module.admin.service.ssh.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.ssh.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "SSH",

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
				xtype: "numberfield",
				name: "port",
				fieldLabel: _("Port"),
				vtype: "port",
				minValue: 0,
				maxValue: 65535,
				allowDecimals: false,
				allowBlank: false,
				value: 22
			},{
				xtype: "checkbox",
				name: "permitrootlogin",
				fieldLabel: _("Permit root login"),
				checked: true,
				boxLabel: _("Specifies whether it is allowed to login as superuser.")
			},{
				xtype: "checkbox",
				name: "passwordauthentication",
				fieldLabel: _("Password authentication"),
				checked: true,
				boxLabel: _("Enable keyboard-interactive authentication")
			},{
				xtype: "checkbox",
				name: "pubkeyauthentication",
				fieldLabel: _("Public key authentication"),
				checked: true,
				boxLabel: _("Enable public key authentication")
			},{
				xtype: "checkbox",
				name: "tcpforwarding",
				fieldLabel: _("TCP forwarding"),
				boxLabel: _("Permit to do SSH tunneling")
			},{
				xtype: "checkbox",
				name: "compression",
				fieldLabel: _("Compression"),
				boxLabel: _("Enable compression"),
				plugins: [{
					ptype: "fieldinfo",
					text: _("Compression is worth using if your connection is slow. The efficiency of the compression depends on the type of the file, and varies widely. Useful for internet transfer only.")
				}]
			},{
				xtype: "textarea",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Please check the <a href='http://www.openbsd.org/cgi-bin/man.cgi?query=sshd_config&sektion=5' target='_blank'>manual page</a> for more details."),
				}]
			}]
		}];
	}
});

OMV.WorkspaceManager.registerNode({
	id: "ssh",
	path: "/service",
	text: _("SSH"),
	icon16: "images/terminal.png",
	iconSvg: "images/terminal.svg"
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/ssh",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.ssh.Settings"
});
