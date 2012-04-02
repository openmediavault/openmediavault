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
	text: _("Certificates"),
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
				header: _("Name"),
				sortable: true,
				dataIndex: "name",
				id: "name"
			},{
				header: _("Valid to"),
				sortable: true,
				dataIndex: "validto",
				id: "validto",
				renderer: OMV.util.Format.localeTimeRenderer()
			},{
				header: _("Comment"),
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
			proxy: new OMV.data.DataProxy({
				"service": "CertificateMgmt",
				"method": "getList"
			}),
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
		// Replace the default 'Add' button
		tbar.remove(0);
		tbar.insert(0, new Ext.SplitButton({
			text: _("Add"),
			icon: "images/add.png",
			handler: function() {
				this.showMenu();
			},
			menu: new Ext.menu.Menu({
				items: [
					{ text: _("Create"), value: "create" },
					{ text: _("Import"), value: "import" }
				],
				listeners: {
					itemclick: function(item, e) {
						this.cbAddBtnHdl(item.value);
					},
					scope: this
				}
			})
		}));
		// Add 'Detail' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-detail",
			xtype: "button",
			text: _("Detail"),
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

	cbAddBtnHdl : function(action) {
		var cls;
		switch (action) {
		case "import":
			cls = OMV.Module.System.CertificatePropertyDialog;
			break;
		case "create":
			cls = OMV.Module.System.CreateCertificateDialog;
			break;
		}
		var wnd = new cls({
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
			rpcGetParams: { "uuid": record.get("uuid") }
		});
		wnd.show();
	},

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "CertificateMgmt",
		  "delete", { "uuid": record.get("uuid") });
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "certificates", {
	cls: OMV.Module.System.CertificateGridPanel
});

/**
 * @class OMV.Module.System.CertificatePropertyDialog
 * @derived OMV.CfgObjectDialog
 */
OMV.Module.System.CertificatePropertyDialog = function(config) {
	var initialConfig = {
		rpcService: "CertificateMgmt",
		rpcGetMethod: "get",
		rpcSetMethod: "set",
		title: (config.uuid == OMV.UUID_UNDEFINED) ?
		  _("Import SSL certificate") : _("Edit SSL certificate"),
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
			fieldLabel: _("Private key"),
			cls: "x-form-textarea-monospaced",
			allowBlank: false,
			height: 150,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("Paste an private RSA key in X.509 PEM format here.")
		},{
			xtype: "textarea",
			name: "certificate",
			fieldLabel: _("Certificate"),
			cls: "x-form-textarea-monospaced",
			allowBlank: false,
			height: 150,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("Paste a RSA certificate in X.509 PEM format here.")
		},{
			xtype: "textfield",
			name: "comment",
			fieldLabel: _("Comment"),
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
		title: _("Certificate details"),
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

/**
 * @class OMV.Module.System.CreateCertificateDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.System.CreateCertificateDialog = function(config) {
	var initialConfig = {
		rpcService: "CertificateMgmt",
		rpcSetMethod: "create",
		title: _("Create self-signed SSL certificate"),
		autoHeight: true,
		hideReset: true,
		width: 550,
		closeIfNotDirty: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.CreateCertificateDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.System.CreateCertificateDialog, OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "combo",
			name: "size",
			hiddenName: "size",
			fieldLabel: _("Key size"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ 512,"512b" ],
					[ 1024,"1024b" ],
					[ 2048,"2048b" ],
					[ 4096,"4096b" ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: 1024,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The RSA key length.")
		},{
			xtype: "combo",
			name: "days",
			hiddenName: "days",
			fieldLabel: _("Period of validity"),
			mode: "local",
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ 1,_("1 day") ],
					[ 2,_("2 days") ],
					[ 3,_("3 days") ],
					[ 4,_("4 days") ],
					[ 5,_("5 days") ],
					[ 6,_("6 days") ],
					[ 7,_("1 week") ],
					[ 14,_("2 weeks") ],
					[ 21,_("3 weeks") ],
					[ 30,_("1 month") ],
					[ 90,_("3 months") ],
					[ 180,_("6 months") ],
					[ 270,_("9 months") ],
					[ 365,_("1 year") ],
					[ 740,_("2 years") ],
					[ 1095,_("3 years") ],
					[ 1460,_("4 years") ],
					[ 1825,_("5 years") ],
					[ 3650,_("10 years") ],
					[ 5475,_("15 years") ],
					[ 7300,_("20 years") ],
					[ 9125,_("25 years") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: false,
			editable: false,
			triggerAction: "all",
			value: 365,
			plugins: [ OMV.form.plugins.FieldInfo ],
			infoText: _("The number of days the certificate is valid for.")
		},{
			xtype: "textfield",
			name: "cn",
			fieldLabel: _("Common Name"),
			allowBlank: false
		},{
			xtype: "textfield",
			name: "o",
			fieldLabel: _("Organization Name"),
			allowBlank: true
		},{
			xtype: "textfield",
			name: "ou",
			fieldLabel: _("Organizational Unit"),
			allowBlank: true
		},{
			xtype: "textfield",
			name: "l",
			fieldLabel: _("City"),
			allowBlank: true
		},{
			xtype: "textfield",
			name: "st",
			fieldLabel: _("State/Province"),
			allowBlank: true
		},{
			xtype: "combo",
			name: "c",
			hiddenName: "c",
			fieldLabel: _("Country"),
			mode: "local",
			// See http://www.digicert.com/ssl-certificate-country-codes.htm
			store: new Ext.data.SimpleStore({
				fields: [ "value","text" ],
				data: [
					[ "AF",_("Afghanistan") ],
					[ "AX",_("Aland Islands") ],
					[ "AL",_("Albania") ],
					[ "DZ",_("Algeria") ],
					[ "AS",_("American Samoa") ],
					[ "AD",_("Andorra") ],
					[ "AO",_("Angola") ],
					[ "AI",_("Anguilla") ],
					[ "AQ",_("Antarctica") ],
					[ "AG",_("Antigua and Barbuda") ],
					[ "AR",_("Argentina") ],
					[ "AM",_("Armenia") ],
					[ "AW",_("Aruba") ],
					[ "AU",_("Australia") ],
					[ "AT",_("Austria") ],
					[ "AZ",_("Azerbaijan") ],
					[ "BS",_("Bahamas") ],
					[ "BH",_("Bahrain") ],
					[ "BD",_("Bangladesh") ],
					[ "BB",_("Barbados") ],
					[ "BY",_("Belarus") ],
					[ "BE",_("Belgium") ],
					[ "BZ",_("Belize") ],
					[ "BJ",_("Benin") ],
					[ "BM",_("Bermuda") ],
					[ "BT",_("Bhutan") ],
					[ "BO",_("Bolivia") ],
					[ "BA",_("Bosnia and Herzegovina") ],
					[ "BW",_("Botswana") ],
					[ "BV",_("Bouvet Island") ],
					[ "BR",_("Brazil") ],
					[ "BQ",_("British Antarctic Territory") ],
					[ "IO",_("British Indian Ocean Territory") ],
					[ "VG",_("British Virgin Islands") ],
					[ "BN",_("Brunei") ],
					[ "BG",_("Bulgaria") ],
					[ "BF",_("Burkina Faso") ],
					[ "BI",_("Burundi") ],
					[ "KH",_("Cambodia") ],
					[ "CM",_("Cameroon") ],
					[ "CA",_("Canada") ],
					[ "CT",_("Canton and Enderbury Islands") ],
					[ "CV",_("Cape Verde") ],
					[ "KY",_("Cayman Islands") ],
					[ "CF",_("Central African Republic") ],
					[ "TD",_("Chad") ],
					[ "CL",_("Chile") ],
					[ "CN",_("China") ],
					[ "CX",_("Christmas Island") ],
					[ "CC",_("Cocos (Keeling) Islands") ],
					[ "CO",_("Colombia") ],
					[ "KM",_("Comoros") ],
					[ "CG",_("Congo (Brazzaville)") ],
					[ "CD",_("Congo (Kinshasa)") ],
					[ "CK",_("Cook Islands") ],
					[ "CR",_("Costa Rica") ],
					[ "HR",_("Croatia") ],
					[ "CU",_("Cuba") ],
					[ "CY",_("Cyprus") ],
					[ "CZ",_("Czech Republic") ],
					[ "DK",_("Denmark") ],
					[ "DJ",_("Djibouti") ],
					[ "DM",_("Dominica") ],
					[ "DO",_("Dominican Republic") ],
					[ "NQ",_("Dronning Maud Land") ],
					[ "TL",_("East Timor") ],
					[ "EC",_("Ecuador") ],
					[ "EG",_("Egypt") ],
					[ "SV",_("El Salvador") ],
					[ "GQ",_("Equatorial Guinea") ],
					[ "ER",_("Eritrea") ],
					[ "EE",_("Estonia") ],
					[ "ET",_("Ethiopia") ],
					[ "FK",_("Falkland Islands") ],
					[ "FO",_("Faroe Islands") ],
					[ "FJ",_("Fiji") ],
					[ "FI",_("Finland") ],
					[ "FR",_("France") ],
					[ "GF",_("French Guiana") ],
					[ "PF",_("French Polynesia") ],
					[ "TF",_("French Southern Territories") ],
					[ "FQ",_("French Southern and Antarctic Territories") ],
					[ "GA",_("Gabon") ],
					[ "GM",_("Gambia") ],
					[ "GE",_("Georgia") ],
					[ "DE",_("Germany") ],
					[ "GH",_("Ghana") ],
					[ "GI",_("Gibraltar") ],
					[ "GR",_("Greece") ],
					[ "GL",_("Greenland") ],
					[ "GD",_("Grenada") ],
					[ "GP",_("Guadeloupe") ],
					[ "GU",_("Guam") ],
					[ "GT",_("Guatemala") ],
					[ "GN",_("Guinea") ],
					[ "GW",_("Guinea-Bissau") ],
					[ "GY",_("Guyana") ],
					[ "HT",_("Haiti") ],
					[ "HM",_("Heard Island and McDonald Islands") ],
					[ "HN",_("Honduras") ],
					[ "HK",_("Hong Kong S.A.R., China") ],
					[ "HU",_("Hungary") ],
					[ "IS",_("Iceland") ],
					[ "IN",_("India") ],
					[ "ID",_("Indonesia") ],
					[ "IR",_("Iran") ],
					[ "IQ",_("Iraq") ],
					[ "IE",_("Ireland") ],
					[ "IL",_("Israel") ],
					[ "IT",_("Italy") ],
					[ "CI",_("Ivory Coast") ],
					[ "JM",_("Jamaica") ],
					[ "JP",_("Japan") ],
					[ "JT",_("Johnston Island") ],
					[ "JO",_("Jordan") ],
					[ "KZ",_("Kazakhstan") ],
					[ "KE",_("Kenya") ],
					[ "KI",_("Kiribati") ],
					[ "KW",_("Kuwait") ],
					[ "KG",_("Kyrgyzstan") ],
					[ "LA",_("Laos") ],
					[ "LV",_("Latvia") ],
					[ "LB",_("Lebanon") ],
					[ "LS",_("Lesotho") ],
					[ "LR",_("Liberia") ],
					[ "LY",_("Libya") ],
					[ "LI",_("Liechtenstein") ],
					[ "LT",_("Lithuania") ],
					[ "LU",_("Luxembourg") ],
					[ "MO",_("Macao S.A.R., China") ],
					[ "MK",_("Macedonia") ],
					[ "MG",_("Madagascar") ],
					[ "MW",_("Malawi") ],
					[ "MY",_("Malaysia") ],
					[ "MV",_("Maldives") ],
					[ "ML",_("Mali") ],
					[ "MT",_("Malta") ],
					[ "MH",_("Marshall Islands") ],
					[ "MQ",_("Martinique") ],
					[ "MR",_("Mauritania") ],
					[ "MU",_("Mauritius") ],
					[ "YT",_("Mayotte") ],
					[ "FX",_("Metropolitan France") ],
					[ "MX",_("Mexico") ],
					[ "FM",_("Micronesia") ],
					[ "MI",_("Midway Islands") ],
					[ "MD",_("Moldova") ],
					[ "MC",_("Monaco") ],
					[ "MN",_("Mongolia") ],
					[ "MS",_("Montserrat") ],
					[ "MA",_("Morocco") ],
					[ "MZ",_("Mozambique") ],
					[ "MM",_("Myanmar") ],
					[ "NA",_("Namibia") ],
					[ "NR",_("Nauru") ],
					[ "NP",_("Nepal") ],
					[ "NL",_("Netherlands") ],
					[ "AN",_("Netherlands Antilles") ],
					[ "NT",_("Neutral Zone") ],
					[ "NC",_("New Caledonia") ],
					[ "NZ",_("New Zealand") ],
					[ "NI",_("Nicaragua") ],
					[ "NE",_("Niger") ],
					[ "NG",_("Nigeria") ],
					[ "NU",_("Niue") ],
					[ "NF",_("Norfolk Island") ],
					[ "KP",_("North Korea") ],
					[ "VD",_("North Vietnam") ],
					[ "MP",_("Northern Mariana Islands") ],
					[ "NO",_("Norway") ],
					[ "OM",_("Oman") ],
					[ "QO",_("Outlying Oceania") ],
					[ "PC",_("Pacific Islands Trust Territory") ],
					[ "PK",_("Pakistan") ],
					[ "PW",_("Palau") ],
					[ "PS",_("Palestinian Territory") ],
					[ "PA",_("Panama") ],
					[ "PZ",_("Panama Canal Zone") ],
					[ "PG",_("Papua New Guinea") ],
					[ "PY",_("Paraguay") ],
					[ "YD",_("People's Democratic Republic of Yemen") ],
					[ "PE",_("Peru") ],
					[ "PH",_("Philippines") ],
					[ "PN",_("Pitcairn") ],
					[ "PL",_("Poland") ],
					[ "PT",_("Portugal") ],
					[ "PR",_("Puerto Rico") ],
					[ "QA",_("Qatar") ],
					[ "RE",_("Reunion") ],
					[ "RO",_("Romania") ],
					[ "RU",_("Russia") ],
					[ "RW",_("Rwanda") ],
					[ "SH",_("Saint Helena") ],
					[ "KN",_("Saint Kitts and Nevis") ],
					[ "LC",_("Saint Lucia") ],
					[ "PM",_("Saint Pierre and Miquelon") ],
					[ "VC",_("Saint Vincent and the Grenadines") ],
					[ "WS",_("Samoa") ],
					[ "SM",_("San Marino") ],
					[ "ST",_("Sao Tome and Principe") ],
					[ "SA",_("Saudi Arabia") ],
					[ "SN",_("Senegal") ],
					[ "CS",_("Serbia And Montenegro") ],
					[ "SC",_("Seychelles") ],
					[ "SL",_("Sierra Leone") ],
					[ "SG",_("Singapore") ],
					[ "SK",_("Slovakia") ],
					[ "SI",_("Slovenia") ],
					[ "SB",_("Solomon Islands") ],
					[ "SO",_("Somalia") ],
					[ "ZA",_("South Africa") ],
					[ "GS",_("South Georgia and the South Sandwich Islands") ],
					[ "KR",_("South Korea") ],
					[ "ES",_("Spain") ],
					[ "LK",_("Sri Lanka") ],
					[ "SD",_("Sudan") ],
					[ "SR",_("Suriname") ],
					[ "SJ",_("Svalbard and Jan Mayen") ],
					[ "SZ",_("Swaziland") ],
					[ "SE",_("Sweden") ],
					[ "CH",_("Switzerland") ],
					[ "SY",_("Syria") ],
					[ "TW",_("Taiwan") ],
					[ "TJ",_("Tajikistan") ],
					[ "TZ",_("Tanzania") ],
					[ "TH",_("Thailand") ],
					[ "TG",_("Togo") ],
					[ "TK",_("Tokelau") ],
					[ "TO",_("Tonga") ],
					[ "TT",_("Trinidad and Tobago") ],
					[ "TN",_("Tunisia") ],
					[ "TR",_("Turkey") ],
					[ "TM",_("Turkmenistan") ],
					[ "TC",_("Turks and Caicos Islands") ],
					[ "TV",_("Tuvalu") ],
					[ "PU",_("U.S. Miscellaneous Pacific Islands") ],
					[ "VI",_("U.S. Virgin Islands") ],
					[ "UG",_("Uganda") ],
					[ "UA",_("Ukraine") ],
					[ "SU",_("Union of Soviet Socialist Republics") ],
					[ "AE",_("United Arab Emirates") ],
					[ "GB",_("United Kingdom") ],
					[ "US",_("United States") ],
					[ "UM",_("United States Minor Outlying Islands") ],
					[ "UY",_("Uruguay") ],
					[ "UZ",_("Uzbekistan") ],
					[ "VU",_("Vanuatu") ],
					[ "VA",_("Vatican") ],
					[ "VE",_("Venezuela") ],
					[ "VN",_("Vietnam") ],
					[ "WK",_("Wake Island") ],
					[ "WF",_("Wallis and Futuna") ],
					[ "EH",_("Western Sahara") ],
					[ "YE",_("Yemen") ],
					[ "ZM",_("Zambia") ],
					[ "ZW",_("Zimbabwe") ]
				]
			}),
			displayField: "text",
			valueField: "value",
			allowBlank: true,
			typeAhead: true,
			forceSelection: true,
			triggerAction: "all",
			emptyText: _("Select a country ..."),
			value: ""
		},{
			xtype: "textfield",
			name: "email",
			fieldLabel: _("Email"),
			allowBlank: true,
			vtype: "email"
		}];
	}
});
