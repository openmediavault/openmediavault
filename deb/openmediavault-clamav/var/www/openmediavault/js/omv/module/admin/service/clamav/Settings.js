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

/**
 * @class OMV.module.admin.service.clamav.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.clamav.Settings", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "ClamAV",
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
				name: "databasemirror",
				fieldLabel: _("Database mirror"),
				allowBlank: false,
				value: "db.local.clamav.net",
				plugins: [{
					ptype: "fieldinfo",
					text: _("Server name where database updates are downloaded from. Get a complete list of database mirrors <a href='http://www.clamav.net/mirrors.html' target='_blank'>here</a>.")
				}]
			},{
				xtype: "numberfield",
				name: "checks",
				fieldLabel: _("Database checks"),
				minValue: 0,
				maxValue: 50,
				allowDecimals: false,
				allowBlank: false,
				value: 24,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Number of database update checks per day. Set to 0 to disable.")
				}]
			},{
				xtype: "checkbox",
				name: "logclean",
				fieldLabel: _("Log clean files"),
				boxLabel: _("Log clean files. This drastically increases the log size."),
				checked: false
			},{
				xtype: "checkbox",
				name: "scanpe",
				fieldLabel: _("Scan Portable Executable"),
				boxLabel: _("Perform a deeper analysis of executable files."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanole2",
				fieldLabel: _("Scan OLE2"),
				boxLabel: _("Enable scanning of OLE2 files, such as Microsoft Office documents and .msi files."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanhtml",
				fieldLabel: _("Scan HTML"),
				boxLabel: _("Enable HTML detection and normalisation."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanpdf",
				fieldLabel: _("Scan PDF"),
				boxLabel: _("Enable scanning within PDF files."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanelf",
				fieldLabel: _("Scan ELF"),
				boxLabel: _("Enable scanning of ELF files."),
				checked: true
			},{
				xtype: "checkbox",
				name: "scanarchive",
				fieldLabel: _("Scan archives"),
				boxLabel: _("Enable archive scanning."),
				checked: true
			},{
				xtype: "checkbox",
				name: "detectbrokenexecutables",
				fieldLabel: _("Detect broken executables"),
				boxLabel: _("Enable the detection of broken executables (both PE and ELF)."),
				checked: false
			},{
				xtype: "checkbox",
				name: "detectpua",
				fieldLabel: _("Detect PUA"),
				boxLabel: _("Enable the detection of possibly unwanted applications."),
				checked: false
			},{
				xtype: "checkbox",
				name: "algorithmicdetection",
				fieldLabel: _("Algorithmic detection"),
				boxLabel: _("Enable the algorithmic detection."),
				checked: true
			},{
				xtype: "checkbox",
				name: "followdirectorysymlinks",
				fieldLabel: _("Follow directory symlinks"),
		   		boxLabel: _("Follow directory symlinks."),
				checked: false
			},{
				xtype: "checkbox",
				name: "followfilesymlinks",
				fieldLabel: _("Follow file symlinks"),
				boxLabel: _("Follow regular file symlinks."),
				checked: false
			},{
				xtype: "textfield",
				name: "extraoptions",
				fieldLabel: _("Extra options"),
				allowBlank: true,
				plugins: [{
					ptype: "fieldinfo",
					text: _("Please check the <a href='http://linux.die.net/man/5/clamd.conf' target='_blank'>manual page</a> for more details."),
				}]
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "settings",
	path: "/service/clamav",
	text: _("Settings"),
	position: 10,
	className: "OMV.module.admin.service.clamav.Settings"
});
