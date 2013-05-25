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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

/**
 * @class OMV.module.admin.system.network.Dns
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.system.network.Dns", {
	extend: "OMV.workspace.form.Panel",

	rpcService: "Network",
	rpcGetMethod: "getDNSNameServers",
	rpcSetMethod: "setDNSNameServers",

	getFormItems: function() {
		return [{
			xtype: "fieldset",
			title: _("DNS server"),
			fieldDefaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "textfield",
				name: "primarydns",
				fieldLabel: _("Primary"),
				vtype: "IPv4",
				allowBlank: true
			},{
				xtype: "textfield",
				name: "secondarydns",
				fieldLabel: _("Secondary"),
				vtype: "IPv4List",
				allowBlank: true
			}]
		}];
	},

	/**
	 * Set values for fields in this form in bulk.
	 * @param values The values to set in the form of an object hash.
	 * @return The basic form object.
	 */
	setValues: function(values) {
		this.callParent({
			"primarydns": values.shift(),
			"secondarydns": values.join(",")
		});
	},

	/**
	 * Returns the fields in this form as an object with key/value pairs.
	 */
	getValues: function() {
		var me = this;
		var values = me.callParent(arguments);
		var dnsnameservers = [];
		if(!Ext.isEmpty(values.primarydns))
			dnsnameservers.push(values.primarydns);
		if(!Ext.isEmpty(values.secondarydns))
			dnsnameservers = dnsnameservers.concat(
			  values.secondarydns.split(/[,;]/));
		return {
			"dnsnameservers": dnsnameservers
		};
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "dns",
	path: "/system/network",
	text: _("DNS Server"),
	position: 30,
	className: "OMV.module.admin.system.network.Dns"
});
