/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    Marcel Beck <marcel.beck@mbeck.org>
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
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/PasswordField.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "transmissionbt", {
	text: "BitTorrent",
	icon: "images/transmissionbt.png"
});

/**
 * @class OMV.Module.Services.TransmissionBTSettingsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBTSettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "TransmissionBT",
		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBTSettingsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBTSettingsPanel, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.TransmissionBTSettingsPanel.superclass.
		  initComponent.apply(this, arguments);
		this.on("load", this._updateFormFields, this);
	},

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
				inputValue: 1,
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "sharedfoldercombo",
				name: "sharedfolderref",
				hiddenName: "sharedfolderref",
				fieldLabel: "Shared folder",
				allowNone: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Make sure the group 'debian-transmission' has read/write access to the shared folder."
			},{
				xtype: "checkbox",
				name: "pexenabled",
				fieldLabel: "Peer exchange",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable peer exchange (PEX)."
			},{
				xtype: "checkbox",
				name: "dhtenabled",
				fieldLabel: "Distributed hash table (DHT).",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable distributed hash table."
			},{
				xtype: "checkbox",
				name: "lpd-enabled",
				fieldLabel: "Local Peer Discovery (LPD).",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable local peer discovery."
			},{
				xtype: "checkbox",
				name: "utp-enabled",
				fieldLabel: "Micro Transport Protocol (&micro;TP).",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable micro transport protocol."
			},{
				xtype: "combo",
				name: "encryption",
				hiddenName: "encryption",
				fieldLabel: "Encryption",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,"Off" ],
						[ 1,"Preferred" ],
						[ 2,"Forced" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The peer connection encryption mode."
			},{
				xtype: "combo",
				name: "message-level",
				hiddenName: "message-level",
				fieldLabel: "Message Level",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,"None" ],
						[ 1,"Error" ],
						[ 2,"Info" ],
						[ 3,"Debug" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 2,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Set verbosity of transmission messages."
			},{
				xtype: "checkbox",
				name: "lazy-bitfield-enabled",
				fieldLabel: "Lazy Bitfield",
				checked: true,
				inputValue: 1,
				boxLabel: "May help get around some ISP filtering."
			},{
				xtype: "checkbox",
				name: "scrape-paused-torrents-enabled",
				fieldLabel: "Scrape paused torrents.",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable paused torrent scraping."
			},{
				xtype: "numberfield",
				name: "umask",
				fieldLabel: "Umask",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 18,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Sets transmission's file mode creation mask."
			}]
		},{
			xtype: "fieldset",
			title: "Remote administration settings",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "rpcenabled",
				fieldLabel: "Enable",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable remote administration."
			},{
				xtype: "numberfield",
				name: "rpcport",
				fieldLabel: "Port",
				vtype: "port",
				minValue: 1024,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 9091,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Port to open and listen for RPC requests on."
			},{
				xtype: "checkbox",
				name: "rpcauthenticationrequired",
				fieldLabel: "Authentication",
				checked: true,
				inputValue: 1,
				boxLabel: "Require clients to authenticate themselves.",
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "textfield",
				name: "rpcusername",
				fieldLabel: "Username",
				allowBlank: false,
				vtype: "username",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Used for client authentication."
			},{
				xtype: "passwordfield",
				name: "rpcpassword",
				fieldLabel: "Password",
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Used for client authentication."
			}]
		},{
			xtype: "fieldset",
			title: "Blocklists",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "blocklistenabled",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1,
				boxLabel: "Use blocklists."
			},{
				xtype: "checkbox",
				name: "blocklistsyncenabled",
				fieldLabel: "Auto sync",
				checked: false,
				inputValue: 1,
				boxLabel: "Update blocklists automatically.",
				listeners: {
					check: this._updateFormFields,
					scope: this
				}
			},{
				xtype: "combo",
				name: "blocklistsyncfrequency",
				hiddenName: "blocklistsyncfrequency",
				fieldLabel: "Sync frequency",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ "hourly","Hourly" ],
						[ "daily","Daily" ],
						[ "weekly","Weekly" ],
						[ "monthly","Monthly" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: "daily"
			},{
				xtype: "textfield",
				name: "blocklisturl",
				fieldLabel: "URL",
				allowBlank: true,
				width: 300,
				value: "http://update.transmissionbt.com/level1.gz",
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "The URL of the blocklist."
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		// Update authentication settings
		var field = this.findFormField("rpcauthenticationrequired");
		var checked = field.checked;
		var fields = [ "rpcusername", "rpcpassword" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
		// Update blocklist settings
		field = this.findFormField("blocklistsyncenabled");
		checked = field.checked;
		fields = [ "blocklistsyncfrequency", "blocklisturl" ];
		for (var i = 0; i < fields.length; i++) {
			field = this.findFormField(fields[i]);
			if (!Ext.isEmpty(field)) {
				field.allowBlank = !checked;
				field.setReadOnly(!checked);
			}
		}
		// Update 'sharedfolderref' field settings
		field = this.findFormField("enable");
		var checked = field.checked;
		field = this.findFormField("sharedfolderref");
		if (!Ext.isEmpty(field)) {
			field.allowBlank = !checked;
		}
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "transmissionbt", {
	cls: OMV.Module.Services.TransmissionBTSettingsPanel,
	title: "Settings",
	position: 10
});

/**
 * @class OMV.Module.Services.TransmissionBTPeerPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBTPeerPanel = function(config) {
	var initialConfig = {
		rpcService: "TransmissionBT",
		rpcGetMethod: "getPeer",
		rpcSetMethod: "setPeer"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBTPeerPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBTPeerPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "Peers",
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "fieldset",
				title: "Bindings",
				defaults: {
					labelSeparator: ""
				},
				items: [{
					xtype: "textfield",
					name: "bind-address-ipv4",
					fieldLabel: "IPv4",
					vtype: "IPv4Net",
					allowBlank: false,
					value: "0.0.0.0",
					plugins: [ OMV.form.plugins.FieldInfo ],
					infoText: "IPv4 address to listen on. Use 0.0.0.0 for all host IPs." 
				},{
					xtype: "textfield",
					name: "bind-address-ipv6",
					fieldLabel: "IPv6",
					allowBlank: false,
					value: "::",
					plugins: [ OMV.form.plugins.FieldInfo ],
					infoText: "IPv6 address to listen on. Use :: for all host IPs." 
				}]
			},{
				xtype: "fieldset",
				title: "Limits",
				defaults: {
					labelSeparator: ""
				},
				items: [{
					xtype: "numberfield",
					name: "peer-limit-global",
					fieldLabel: "Global",
					allowDecimals: false,
					allowNegative: false,
					allowBlank: false,
					value: 240
				},{
					xtype: "numberfield",
					name: "peer-limit-per-torrent",
					fieldLabel: "Per torrent",
					allowDecimals: false,
					allowNegative: false,
					allowBlank: false,
					value: 60
				},{
					xtype: "combo",
					name: "peer-socket-tos",
					hiddenName: "peer-socket-tos",
					fieldLabel: "Socket TOS",
					mode: "local",
					store: new Ext.data.SimpleStore({
						fields: [ "value","text" ],
						data: [
							[ "default","default" ],
							[ "lowcost","lowcost" ],
							[ "throughput","throughput" ],
							[ "lowdelay","lowdelay" ],
							[ "reliability","reliability" ]
						]
					}),
					displayField: "text",
					valueField: "value",
					allowBlank: false,
					editable: false,
					triggerAction: "all",
					value: "default"
				}]
			}]
		},{
			xtype: "fieldset",
			title: "Peer Ports",
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "numberfield",
				name: "peer-port",
				fieldLabel: "Peer port",
				vtype: "port",
				minValue: 1024,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 51413,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Port to listen for incoming peer connections."
			},{
				xtype: "checkbox",
				name: "peer-port-random-on-start",
				fieldLabel: "Random Port",
				checked: false,
				inputValue: 1,
				boxLabel: "Random Port on start."
			},{
				xtype: "numberfield",
				name: "peer-port-random-low",
				fieldLabel: "Random low",
				allowBlank: false,
				vtype: "port",
				minValue: 1024,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 1024
			},{
				xtype: "numberfield",
				name: "peer-port-random-high",
				fieldLabel: "Random high",
				vtype: "port",
				minValue: 1024,
				maxValue: 65535,
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 65535
			},{
				xtype: "checkbox",
				name: "port-forwarding-enabled",
				fieldLabel: "Port forwarding",
				checked: true,
				inputValue: 1,
				boxLabel: "Enable port forwarding via NAT-PMP or UPnP."
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "transmissionbt", {
	cls: OMV.Module.Services.TransmissionBTPeerPanel,
	title: "Peer",
	position: 20
});

/**
 * @class OMV.Module.Services.TransmissionBTFilesAndLocationsPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBTFilesAndLocationsPanel = function(config) {
	var initialConfig = {
		rpcService: "TransmissionBT",
		rpcGetMethod: "getLocationsAndFiles",
		rpcSetMethod: "setLocationsAndFiles"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBTFilesAndLocationsPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBTFilesAndLocationsPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "Locations",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "download-dir",
				fieldLabel: "Download directory",
				allowBlank: true,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Directory to keep downloads. If incomplete is enabled, only complete downloads will be stored here."
			},{
				xtype: "checkbox",
				name: "incomplete-dir-enabled",
				fieldLabel: "Incomplete",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable incomplete directory."
			},{
				xtype: "textfield",
				name: "incomplete-dir",
				fieldLabel: "Incomplete directory",
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Directory to keep files in until torrent is complete."
			},{
				xtype: "checkbox",
				name: "watch-dir-enabled",
				fieldLabel: "Watch",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable Watch directory."
			},{
				xtype: "textfield",
				name: "watch-dir",
				fieldLabel: "Watch directory",
				allowBlank: false,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Watch a directory for torrent files and add them to transmission"
			}]
		},{
			xtype: "fieldset",
			title: "Files",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "combo",
				name: "preallocation",
				hiddenName: "preallocation",
				fieldLabel: "Preallocation",
				mode: "local",
				store: new Ext.data.SimpleStore({
					fields: [ "value","text" ],
					data: [
						[ 0,"Off" ],
						[ 1,"Fast" ],
						[ 2,"Full" ]
					]
				}),
				displayField: "text",
				valueField: "value",
				allowBlank: false,
				editable: false,
				triggerAction: "all",
				value: 1,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Mode for preallocating files."
			},{
				xtype: "checkbox",
				name: "rename-partial-files",
				fieldLabel: "Postfix",
				checked: true,
				inputValue: 1,
				boxLabel: "Postfix partially downloaded files with .part."
			},{
				xtype: "checkbox",
				name: "start-added-torrents",
				fieldLabel: "Start Torrents",
				checked: true,
				inputValue: 1,
				boxLabel: "Start torrents as soon as they are added."
			},{
				xtype: "checkbox",
				name: "trash-original-torrent-files",
				fieldLabel: "Trash original",
				checked: false,
				inputValue: 1,
				boxLabel: "Delete torrents added from the watch directory."
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "transmissionbt", {
	cls: OMV.Module.Services.TransmissionBTFilesAndLocationsPanel,
	title: "Files and Locations",
	position: 30
});

/**
 * @class OMV.Module.Services.TransmissionBTBandwidthPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBTBandwidthPanel = function(config) {
	var initialConfig = {
		rpcService: "TransmissionBT",
		rpcGetMethod: "getBandwidth",
		rpcSetMethod: "setBandwidth"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBTBandwidthPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBTBandwidthPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "Speed",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "speed-limit-down-enabled",
				fieldLabel: "Limit Download",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable download limit."
			},{
				xtype: "numberfield",
				name: "speed-limit-down",
				fieldLabel: "Download",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 100,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Limit download speed. Value is kb/s."
			},{
				xtype: "checkbox",
				name: "speed-limit-up-enabled",
				fieldLabel: "Limit Upload",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable upload limit."
			},{
				xtype: "numberfield",
				name: "speed-limit-up",
				fieldLabel: "Upload",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 100,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Limit upload speed. Value is kb/s."
			},{
				xtype: "numberfield",
				name: "upload-slots-per-torrent",
				fieldLabel: "Upload slots",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 14
			}]
		},{
			xtype: "fieldset",
			title: "Turtle Mode",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "alt-speed-enabled",
				fieldLabel: "Enable",
				checked: false,
				inputValue: 1,
				boxLabel: "Enable Turtle Mode."
			},{
				xtype: "numberfield",
				name: "alt-speed-up",
				fieldLabel: "Upload",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 50,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Turtle Mode upload speed. Value is kb/s."
			},{
				xtype: "numberfield",
				name: "alt-speed-down",
				fieldLabel: "Download",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 50,
				plugins: [ OMV.form.plugins.FieldInfo ],
				infoText: "Turtle Mode download speed. Value is kb/s."
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "transmissionbt", {
	cls: OMV.Module.Services.TransmissionBTBandwidthPanel,
	title: "Bandwidth",
	position: 40
});

/**
 * @class OMV.Module.Services.TransmissionBTQueuingPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBTQueuingPanel = function(config) {
	var initialConfig = {
		rpcService: "TransmissionBT",
		rpcGetMethod: "getQueuing",
		rpcSetMethod: "setQueuing"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBTQueuingPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBTQueuingPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "General",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "queue-stalled-enabled",
				fieldLabel: "Queue Stalled",
				checked: true,
				inputValue: 1,
				boxLabel: "Torrents that have not shared data for queue-stalled-minutes are treated as 'stalled' and are not counted against the queue-download-size and seed-queue-size limits."
			},{
				xtype: "numberfield",
				name: "queue-stalled-minutes",
				fieldLabel: "Stalled Minutes",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 30
			}]
		},{
			xtype: "fieldset",
			title: "Download Queue",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "download-queue-enabled",
				fieldLabel: "Download",
				checked: true,
				inputValue: 1,
				boxLabel: "Transmission will only download download-queue-size non-stalled torrents at once."
			},{
				xtype: "numberfield",
				name: "download-queue-size",
				fieldLabel: "Size",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 5
			}]
		},{
			xtype: "fieldset",
			title: "Seed Queue",
			defaults: {
//				anchor: "100%",
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "seed-queue-enabled",
				fieldLabel: "Seed",
				checked: false,
				inputValue: 1,
				boxLabel: "Transmission will only seed seed-queue-size non-stalled torrents at once."
			},{
				xtype: "numberfield",
				name: "seed-queue-size",
				fieldLabel: "Size",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 10
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "transmissionbt", {
	cls: OMV.Module.Services.TransmissionBTQueuingPanel,
	title: "Queuing",
	position: 50
});

/**
 * @class OMV.Module.Services.TransmissionBTSchedulingPanel
 * @derived OMV.FormPanelExt
 */
OMV.Module.Services.TransmissionBTSchedulingPanel = function(config) {
	var initialConfig = {
		rpcService: "TransmissionBT",
		rpcGetMethod: "getScheduling",
		rpcSetMethod: "setScheduling"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBTSchedulingPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBTSchedulingPanel, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{xtype: "fieldset",
			title: "General",
			defaults: {
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "alt-speed-time-enabled",
				fieldLabel: "Scheduling",
				checked: false,
				inputValue: 1,
				boxLabel: "When enabled, this will toggle the Turtle Mode."
			},{
				xtype: "fieldset",
				title: "Time",
				defaults: {
					labelSeparator: ""
				},
				items: [{
					xtype: "compositefield",
					name: "begin-time",
					fieldLabel: "Begin",
					width: 200,
					items: [{
						xtype: "combo",
						name: "begin-hour",
						mode: "local",
						store: Array.range(0, 23),
						allowBlank: false,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: 9,
						reset: function() {}
					},{
						xtype: "displayfield",
						value: ":"
					},{
						xtype: "combo",
						name: "begin-minute",
						mode: "local",
						store: Array.range(0, 59),
						allowBlank: false,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: 0,
						reset: function() {}
					}]
				},{
					xtype: "compositefield",
					name: "end-time",
					fieldLabel: "End",
					width: 200,
					items: [{
						xtype: "combo",
						name: "end-hour",
						mode: "local",
						store: Array.range(0, 23),
						allowBlank: false,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: 17,
						reset: function() {}
					},{
						xtype: "displayfield",
						value: ":"
					},{
						xtype: "combo",
						name: "end-minute",
						mode: "local",
						store: Array.range(0, 59),
						allowBlank: false,
						editable: false,
						triggerAction: "all",
						width: 50,
						value: 0,
						reset: function() {}
					}]
				}]
			},{
				xtype: "fieldset",
				title: "Days",
				defaults: {
					labelSeparator: ""
				},
				items: [{xtype: "checkbox",
					name: "days-sunday",
					fieldLabel: "Sunday",
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-monday",
					fieldLabel: "Monday",
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-tuesday",
					fieldLabel: "Tuesday",
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-wednesday",
					fieldLabel: "Wednesday",
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-thursday",
					fieldLabel: "Thursday",
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-friday",
					fieldLabel: "Friday",
					checked: true,
					inputValue: 1
				},{
					xtype: "checkbox",
					name: "days-saturday",
					fieldLabel: "Saturday",
					checked: true,
					inputValue: 1
				}]
			}]
		},{
			xtype: "fieldset",
			title: "Idle",
			defaults: {
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "idle-seeding-limit-enabled",
				fieldLabel: "Seeding Limit",
				checked: false,
				inputValue: 1,
				boxLabel: "Stop seeding after being idle for N minutes."
			},{
				xtype: "numberfield",
				name: "idle-seeding-limit",
				fieldLabel: "Idle Minutes",
				allowDecimals: false,
				allowNegative: false,
				allowBlank: false,
				value: 30
			}]
		},{
			xtype: "fieldset",
			title: "Ratio",
			defaults: {
				labelSeparator: ""
			},
			items: [{xtype: "checkbox",
				name: "ratio-limit-enabled",
				fieldLabel: "Ratio",
				checked: false,
				inputValue: 1,
				boxLabel: "Transmission will only seed until ratio limit is reached."
			},{
				xtype: "numberfield",
				name: "ratio-limit",
				fieldLabel: "Ratio Limit",
				allowDecimals: true,
				allowNegative: false,
				allowBlank: false,
				value: 2.0
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "transmissionbt", {
	cls: OMV.Module.Services.TransmissionBTSchedulingPanel,
	title: "Scheduling",
	position: 60
});