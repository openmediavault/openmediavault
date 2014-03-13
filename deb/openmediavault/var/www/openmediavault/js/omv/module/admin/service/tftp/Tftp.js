/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.tftp.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.tftp.Settings", {
	extend: "OMV.workspace.form.Panel",
	requires: [
		"OMV.form.field.SharedFolderComboBox"
	],

	rpcService: "TFTP",
	plugins: [{
		ptype: "linkedfields",
		correlations: [{
			name: "sharedfolderref",
			conditions: [
				{ name: "enable", value: true }
			],
			properties: "!allowBlank"
		}]
	}],

	getFormItems: function() {
		var me = this;
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
				value: 69,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Port to listen on.")
				}]
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				fieldLabel: _("Shared folder"),
				allowNone: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("The location of the files to share.")
				}]
			},{
				xtype: "numberfield",
				name: "blocksize",
				fieldLabel: _("Blocksize"),
				minValue: 512,
				maxValue: 65464,
				allowDecimals: false,
				allowBlank: true,
				value: 512,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Specifies the maximum permitted block size. The permitted range for this parameter is from 512 to 65464.")
				}]
			},{
				xtype: "numberfield",
				name: "retransmit",
				fieldLabel: _("Retry timeout"),
				minValue: 0,
				allowDecimals: false,
				allowBlank: false,
				value: 1000000,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Determine the default timeout, in microseconds, before the first packet is retransmitted.")
				}]
			},{
				xtype: "checkbox",
				name: "allownewfiles",
				fieldLabel: _("Allow new files"),
				checked: false,
				boxLabel: _("Allow new files to be created."),
				plugins: [{
					ptype: "fieldinfo",
					text: _("By default, only files that already exist are allowed to be uploaded.")
				}]
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true
			}]
		}];
	}
});

OMV.WorkspaceManager.registerNode({
	id: "tftp",
	path: "/service",
	text: _("TFTP"),
	icon16: "images/tftp.png",
	iconSvg: "images/tftp.svg"
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/tftp",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.tftp.Settings"
});
