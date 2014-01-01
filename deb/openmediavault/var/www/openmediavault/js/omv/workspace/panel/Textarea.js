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
// require("js/omv/workspace/panel/Panel.js")
// require("js/omv/Rpc.js")
// require("js/omv/window/MessageBox.js")

/**
 * @ingroup webgui
 * @class OMV.workspace.panel.Textarea
 * @derived OMV.workspace.panel.Panel
 * A panel that contains a textarea.
 * @param readOnly Mark the textarea as readonly. Defaults to TRUE.
 * @param rpcService The RPC service name.
 * @param rpcMethod The RPC method to request the data.
 * @param rpcParams The RPC parameters.
 */
Ext.define("OMV.workspace.panel.Textarea", {
	extend: "OMV.workspace.panel.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.window.MessageBox"
	],

	layout: "fit",
	hideTopToolbar: false,
	autoLoadData: true,
	readOnly: true,

	initComponent: function() {
		var me = this;
		me.items = [{
			id: me.getId() + "-content",
			xtype: "textarea",
			readOnly: me.readOnly,
			cls: Ext.baseCSSPrefix + "form-textarea-monospaced",
			fieldStyle: {
				border: "0px"
			}
		}]
		me.callParent(arguments);
	},

	doLoad: function() {
		var me = this;
		// Display waiting dialog.
		me.mask(_("Loading ..."));
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				me.unmask();
				if(!success) {
					OMV.MessageBox.error(null, response);
				} else {
					me.setValue(response);
				}
			},
			relayErrors: true,
			rpcData: {
				service: me.rpcService,
				method: me.rpcMethod,
				params: me.rpcParams || null
			}
		});
	},

	/**
	 * Set the textarea content to be displayed.
	 * @param value The value to set.
	 * @return None.
	 */
	setValue: function(value) {
		var me = this;
		var c = me.queryById(me.getId() + "-content");
		if(!Ext.isEmpty(c)) {
			c.setValue(value);
		}
	},

	/**
	 * Returns the textarea content.
	 * @return The textarea content as string.
	 */
	getValue: function() {
		var me = this;
		var c = me.queryById(me.getId() + "-content");
		if(!Ext.isEmpty(c)) {
			value = c.getValue();
		}
		return value;
	}
});
