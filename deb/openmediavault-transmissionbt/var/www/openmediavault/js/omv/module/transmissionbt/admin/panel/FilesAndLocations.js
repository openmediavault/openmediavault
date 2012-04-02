/**
 * Created by JetBrains PhpStorm.
 * User: mbeck
 * Date: 28.11.11
 * Time: 20:41
 * To change this template use File | Settings | File Templates.
 */

// require("js/omv/FormPanelExt.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services.TransmissionBT.Admin");

/**
 * @class OMV.Module.Services.TransmissionBT.Admin.FilesAndLocationsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBT.Admin.FilesAndLocationsPanel = function(config) {
	var initialConfig = {
		title: _("Files and Locations"),
		rpcService: "TransmissionBT",
		rpcGetMethod: "getLocationsAndFiles",
		rpcSetMethod: "setLocationsAndFiles"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBT.Admin.FilesAndLocationsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBT.Admin.FilesAndLocationsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: _("Locations"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "download-dir",
				fieldLabel: _("Download directory"),
				allowBlank: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Directory to keep downloads. If incomplete is enabled, only complete downloads will be stored here.")
			},{
				xtype: "checkbox",
				name: "incomplete-dir-enabled",
				fieldLabel: _("Incomplete"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable incomplete directory.")
			},{
				xtype: "textfield",
				name: "incomplete-dir",
				fieldLabel: _("Incomplete directory"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Directory to keep files in until torrent is complete.")
			},{
				xtype: "checkbox",
				name: "watch-dir-enabled",
				fieldLabel: _("Watch"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Enable Watch directory.")
			},{
				xtype: "textfield",
				name: "watch-dir",
				fieldLabel: _("Watch directory"),
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Watch a directory for torrent files and add them to transmission")
			}]
		},{
			xtype: "fieldset",
			title: _("Files"),
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "combo",
				name: "preallocation",
				hiddenName: "preallocation",
				fieldLabel: _("Preallocation"),
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,_("Off") ],
						[ 1,_("Fast") ],
						[ 2,_("Full") ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: _("Mode for preallocating files.")
			},{
				xtype: "checkbox",
				name: "rename-partial-files",
				fieldLabel: _("Postfix"),
				checked: true,
				inputValue: 1,
				boxLabel: _("Postfix partially downloaded files with .part.")
			},{
				xtype: "checkbox",
				name: "start-added-torrents",
				fieldLabel: _("Start Torrents"),
				checked: true,
				inputValue: 1,
				boxLabel: _("Start torrents as soon as they are added.")
			},{
				xtype: "checkbox",
				name: "trash-original-torrent-files",
				fieldLabel: _("Trash original"),
				checked: false,
				inputValue: 1,
				boxLabel: _("Delete torrents added from the watch directory.")
			}]
		}];
	}
});