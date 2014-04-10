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
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

/**
 * @ingroup webgui
 * @class OMV.form.field.UnixFilePermComboBox
 * @derived Ext.form.field.ComboBox
 * Display file permissions.
 * @param valueType The type of the file permission value. This can be
 *   'octal' or 'string'. Defaults to 'octal'.
 */
Ext.define("OMV.form.field.UnixFilePermComboBox", {
	extend: "Ext.form.field.ComboBox",
	alias: "widget.unixfilepermcombo",
	requires: [
		"Ext.data.ArrayStore"
	],

	valueType: "octal",

	allowBlank: false,
	editable: false,
	forceSelection: true,
	selectOnFocus: true,
	triggerAction: "all",
	emptyText: _("Select a file permission ..."),
	displayField: "text",
	valueField: "value",
	queryMode: "local",

	initComponent: function() {
		var me = this;
		var data = {
			"octal": [
				[ 0, _("None") ],
				[ 1, _("Execute only") ],
				[ 2, _("Write only") ],
				[ 3, _("Write/Execute") ],
				[ 4, _("Read only") ],
				[ 5, _("Read/Execute") ],
				[ 6, _("Read/Write") ],
				[ 7, _("Read/Write/Execute") ]
			],
			"string": [
				[ "", _("None") ],
				[ "x", _("Execute only") ],
				[ "w", _("Write only") ],
				[ "wx", _("Write/Execute") ],
				[ "r", _("Read only") ],
				[ "rx", _("Read/Execute") ],
				[ "rw", _("Read/Write") ],
				[ "rwx", _("Read/Write/Execute") ]
			]
		};
		Ext.apply(me, {
			store: Ext.create("Ext.data.ArrayStore", {
				fields: [ "value", "text" ],
				data: data[me.valueType]
			})
		});
		me.callParent(arguments);
	}
});
