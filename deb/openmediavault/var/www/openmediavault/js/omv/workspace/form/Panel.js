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
// require("js/omv/Rpc.js")
// require("js/omv/window/MessageBox.js")
// require("js/omv/form/Panel.js")
// require("js/omv/form/CompositeField.js")
// require("js/omv/form/plugin/LinkedFields.js")
// require("js/omv/form/field/plugin/FieldInfo.js")

/**
 * @ingroup webgui
 * @class OMV.workspace.form.Panel
 * @derived OMV.form.Panel
 * A enhanced form panel that automatically loads the data after the panel
 * has been rendered. It will also display an array of buttons at the bottom
 * of the panel. By default these are the 'OK' and 'Reset' buttons.
 * @param rpcService The RPC service name. Required.
 * @param rpcGetMethod The RPC method to request the data. Required.
 * @param rpcGetParams The RPC parameters. Required.
 * @param rpcSetMethod The RPC method to commit the data. Required.
 * @param hideTopToolbar TRUE to hide the whole toolbar. Defaults to FALSE.
 * @param hideOkButton Hide the 'OK' button.
 * @param hideResetButton Hide the 'Reset' button.
 * @param okButtonText The 'OK' button text. Defaults to 'Apply'.
 * @param okButtonIconCls The button icon CSS class.
 * @param resetButtonText The 'Reset' button text. Defaults to 'Reset'.
 * @param resetButtonIconCls The button icon CSS class.
 * @param onlySubmitIfDirty Only submit the form values if the form is
 *   marked as dirty. Defaults to TRUE.
 * @param autoLoadData Automatically execute the doLoad method after
 *   the component has been initialized. Defaults to TRUE.
 */
Ext.define("OMV.workspace.form.Panel", {
	extend: "OMV.form.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.form.CompositeField",
		"OMV.form.plugin.LinkedFields",
		"OMV.form.field.plugin.FieldInfo"
	],
	uses: [
		"OMV.window.MessageBox"
	],

	buttonAlign: "left",
	bodyPadding: "5 5 0",

	onlySubmitIfDirty: true,
	hideTopToolbar: false,
	hideOkButton: false,
	hideResetButton: false,
	okButtonText: _("Save"),
	okButtonIconCls: "x-fa fa-check",
	resetButtonText: _("Reset"),
	resetButtonIconCls: "x-fa fa-repeat",
	autoLoadData: true,
	loadMsg: _("Loading ..."),
	submitMsg: _("Saving ..."),

	constructor: function() {
		var me = this;
		me.callParent(arguments);
		/**
		 * @event beforeload
		 * Fires before the form content is loaded.
		 * @param this The form object.
		 * @param options The RPC options.
		 */
		/**
		 * @event load
		 * Fires after the form content has been loaded successful.
		 * @param this The form object.
		 * @response The RPC response.
		 */
		/**
		 * @event beforesubmit
		 * Fires before the form is submitted.
		 * @param this The form object.
		 * @param options The RPC options.
		 */
		/**
		 * @event submit
		 * Fires after the form has been submitted successful.
		 * @param this The form object.
		 * @param values The form values.
		 * @response The RPC response.
		 */
		/**
		 * @event exception
		 * Fires when the form submittion has been failed.
		 */
	},

	initComponent: function() {
		var me = this;
		// Initialize toolbar.
		me.dockedItems = [];
		if(!me.hideTopToolbar) {
			me.dockedItems.push(me.topToolbar = Ext.widget({
				xtype: "toolbar",
				dock: "top",
				defaults: {
					minWidth: me.minButtonWidth
				},
				items: me.getButtonItems(me)
			}));
		}
		// Initialize form.
		Ext.apply(me, {
			items: me.getFormItems()
		});
		me.callParent(arguments);
		if(me.autoLoadData) {
			// Force loading after the component markup is rendered.
			me.on({
				single: true,
				render: me.doLoad
			});
		}
	},

	/**
	 * Returns the items displayed in the toolbar.
	 * @param c This component object.
	 * @return An array of buttons displayed in the toolbar.
	 */
	getButtonItems: function(c) {
		var me = this;
		return [{
			id: me.getId() + "-ok",
			xtype: "button",
			text: me.okButtonText,
			iconCls: me.okButtonIconCls,
			hidden: me.hideOkButton,
			handler: Ext.Function.bind(me.onOkButton, me, [ me ]),
			scope: me
		},{
			id: me.getId() + "-reset",
			xtype: "button",
			text: me.resetButtonText,
			iconCls: me.resetButtonIconCls,
			hidden: me.hideResetButton,
			handler: Ext.Function.bind(me.onResetButton, me, [ me ]),
			scope: me
		}];
	},

	/**
	 * Method that is called when the 'OK' button is pressed.
	 */
	onOkButton: function() {
		var me = this;
		me.doSubmit();
	},

	/**
	 * Method that is called when the 'Reset' button is pressed.
	 */
	onResetButton: function() {
		var me = this;
		me.reset();
	},

	/**
	 * Get the parameters for the \em rpcGetMethod RPC.
	 * @return The RPC parameters.
	 */
	getRpcGetParams: function() {
		return this.rpcGetParams || null
	},

	doLoad: function() {
		var me = this;
		var rpcOptions = {
			scope: me,
			callback: me.onLoad,
			relayErrors: true,
			rpcData: {
				service: me.rpcService,
				method: me.rpcGetMethod || "get",
				params: me.getRpcGetParams()
			}
		};
		if (me.fireEvent("beforeload", me, rpcOptions) === false)
			return;
		// Display waiting dialog.
		me.setLoading(me.loadMsg);
		// Execute RPC.
		OMV.Rpc.request(rpcOptions);
	},

	/**
	 * Handler that is called by the RPC initiated by 'doLoad'.
	 */
	onLoad: function(id, success, response) {
		var me = this;
		me.setLoading(false);
		if (!success) {
			me.fireEvent("exception", me, response);
			OMV.MessageBox.error(null, response);
		} else {
			var basicForm = me.getForm();
			response = me.processLoadResponse(response);
			me.setValues(response);
			basicForm.clearInvalid();
			me.fireEvent("load", me, response);
		}
	},

	/**
	 * Method that is called after the content loading RPC has been
	 * successfully returned.
	 * @param response The object returned by the RPC request.
	 */
	processLoadResponse: function(response) {
		return response;
	},

	doReload: function() {
		var me = this;
		me.doLoad();
	},

	/**
	 * Get the parameters for the \em rpcSetMethod RPC.
	 * @return The RPC parameters.
	 */
	getRpcSetParams: function() {
		var values = this.getValues();
		return values;
	},

	doSubmit: function() {
		var me = this;
		if (me.onlySubmitIfDirty && !me.isDirty())
			return;
		// Validate values
		if (!me.isValid()) {
			me.markInvalid();
			return;
		}
		var rpcOptions = {
			scope: me,
			callback: me.onSubmit,
			relayErrors: true,
			rpcData: {
				service: me.rpcService,
				method: me.rpcSetMethod || "set",
				params: me.getRpcSetParams()
			}
		};
		if (me.fireEvent("beforesubmit", me, rpcOptions) === false)
			return;
		// Display waiting dialog.
		me.setLoading(me.submitMsg);
		// Execute RPC.
		OMV.Rpc.request(rpcOptions);
	},

	/**
	 * Handler that is called by the RPC initiated by 'doSubmit'.
	 * Reload the panel if the RPC was successful.
	 */
	onSubmit: function(id, success, response) {
		var me = this;
		me.setLoading(false);
		if (!success) {
			OMV.MessageBox.error(null, response);
			me.fireEvent("exception", me, response);
		} else {
			var values = me.getRpcSetParams();
			me.doReload();
			me.fireEvent("submit", me, values, response);
		}
	},

	/**
	 * Returns the items displayed in the property window form.
	 * This function must be overwritten by every derived class.
	 */
	getFormItems: function() {
		return [];
	},

	/**
	 * Set/Unset all form fields read-only.
	 */
	setReadOnly: function(readOnly) {
		var me = this;
		me.dockedItems.each(function(dockedItem) {
			if(Ext.isFunction(dockedItem.setReadOnly)) {
				dockedItem.setDisabled(readOnly);
			}
		}, me);
		var basicForm = me.getForm();
		basicForm.getFields().each(function(field) {
			if(Ext.isFunction(field.setReadOnly)) {
				field.setReadOnly(readOnly);
			}
		}, me);
	},

	/**
	 * Convenience function for setting the given button disabled/enabled.
	 * @param name The name of the button.
	 * @param disabled TRUE to disable the button, FALSE to enable.
	 * @return The button component, otherwise FALSE.
	 */
	setButtonDisabled: function(name, disabled) {
		var me = this;
		var button = me.queryById(me.getId() + "-" + name);
		if(!Ext.isObject(button) || !button.isButton)
			return false;
		return button.setDisabled(disabled);
	},

	/**
	 * Convenience function to show or hide the given button.
	 * @param name The name of the button.
	 * @param visible TRUE to show the button, FALSE to hide.
	 * @return The button component, otherwise FALSE.
	 */
	setButtonVisible: function(name, visible) {
		var me = this;
		var button = me.queryById(me.getId() + "-" + name);
		if(!Ext.isObject(button) || !button.isButton)
			return false;
		return button.setVisible(visible);
	}
});
