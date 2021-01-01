/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
 * A panel that contains a text area.
 * @param rpcService The RPC service name. Required.
 * @param rpcMethod The RPC method to request the data. Required.
 * @param rpcParams The RPC parameters. Optional.
 * @param hideDownloadButton Hide the 'Download' button in the top toolbar.
 *   Defaults to TRUE.
 * @param textAreaConfig A config object that will be applied to the
 *   textarea field.
 */
Ext.define("OMV.workspace.panel.Textarea", {
	extend: "OMV.workspace.panel.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.window.MessageBox"
	],

	loadMsg: _("Loading ..."),
	textAreaConfig: {
		editable: false
	},

	layout: "fit",
	hideTopToolbar: false,
	hideDownloadButton: true,
	autoLoadData: true,
	scrollable: false,

	initComponent: function() {
		var me = this;
		me.ta = Ext.create(Ext.apply({
			xtype: "textarea",
			cls: Ext.baseCSSPrefix + "form-textarea-monospaced",
			inputWrapCls: Ext.form.field.TextArea.prototype.inputWrapCls +
			  " " + Ext.baseCSSPrefix + "form-text-wrap-noborder"
		}, me.getTextAreaConfig(me)));
		me.items = [ me.ta ];
		me.callParent(arguments);
	},

	/**
	 * Returns additional textarea configuration options.
	 * @param c This component object.
	 * @return The textarea configuration object.
	 */
	getTextAreaConfig: function(c) {
		return this.textAreaConfig;
	},

	/**
	 * Returns the textarea field.
	 * @return The textarea field object.
	 */
	getTextArea: function() {
		return this.ta;
	},

	getTopToolbarItems: function(c) {
		var me = this;
		var items = me.callParent(arguments);
		Ext.Array.push(items, {
			id: me.getId() + "-download",
			xtype: "button",
			text: _("Download"),
			iconCls: "x-fa fa-download",
			hidden: me.hideDownloadButton,
			handler: Ext.Function.bind(me.onDownloadButton, me, [ me ]),
			scope: me
		});
		return items;
	},

	doLoad: function() {
		var me = this;
		// Display waiting dialog.
		me.setLoading(me.loadMsg);
		// Execute RPC.
		OMV.Rpc.request({
			scope: me,
			callback: function(id, success, response) {
				me.setLoading(false);
				if (!success) {
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
	 * @return void
	 */
	setValue: function(value) {
		var me = this;
		return me.getTextArea().setValue(value);
	},

	/**
	 * Returns the textarea content.
	 * @return The textarea content as string.
	 */
	getValue: function() {
		var me = this;
		return me.getTextArea().getValue();
	},

	/**
	 * Handler that is called when the 'Download' button in the top toolbar
	 * is pressed.
	 */
	onDownloadButton: function() {
		var me = this;
		OMV.Download.request(me.rpcService, me.rpcMethod,
		  me.rpcParams || null);
	}
});
