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
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")

Ext.ns("OMV.form");

/**
 * @class OMV.form.GroupComboBox
 * @derived Ext.form.ComboBox
 * Display group names.
 * @param groupType Which users should be displayed, system, normal or all.
 *   Defaults to 'all'.
 */
OMV.form.GroupComboBox = function(config) {
	var rpcMethod = {
		"normal": "enumerateGroups",
		"system": "enumerateSystemGroups",
		"all": "enumerateAllGroups"
	};
	var initialConfig = {
		allowBlank: false,
		editable: true,
		typeAhead: true,
		forceSelection: true,
		selectOnFocus: true,
		minChars: 1,
		triggerAction: "all",
		store: new OMV.data.Store({
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "UserMgmt",
				"method": rpcMethod[config.groupType || "all"],
				"appendPagingParams": false
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "name",
				fields: [
					{ name: "name" }
				]
			}),
			sortInfo: {
				field: "name",
				direction: "ASC"
			},
			listeners: {
				scope: this,
				"load": function(store, records, options) {
					// Switch combobox to mode = 'local' to do not
					// execute an RPC on each query (e.g. typeahead).
					if (true === this.typeAhead) {
						this.mode = "local";
					}
				}
			}
		}),
		emptyText: _("Select a group name..."),
		valueField: "name",
		displayField: "name"
	};
	Ext.apply(initialConfig, config);
	OMV.form.GroupComboBox.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.form.GroupComboBox, Ext.form.ComboBox, {});
Ext.reg("groupcombo", OMV.form.GroupComboBox);
