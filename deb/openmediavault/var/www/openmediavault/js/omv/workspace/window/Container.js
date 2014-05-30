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
// require("js/omv/Rpc.js")
// require("js/omv/window/Window.js")
// require("js/omv/window/MessageBox.js")

/**
 * @ingroup webgui
 * Generic workspace window class that can be used to display a form, grid
 * or tab panel. The content is loaded/stored via RPC.
 * @class OMV.workspace.window.Container
 * @derived OMV.window.Window
 * @param rpcService The RPC service name. Required.
 * @param rpcGetMethod The RPC method to get the data. Required.
 * @param rpcGetParams The RPC get method parameters. Required.
 * @param rpcSetMethod The RPC method to commit the data. Required.
 * @param rpcSetPollStatus Set to TRUE if the RPC is a long running one. In
 *   this case the process status is polled frequently. Defaults to FALSE.
 * @param hideOkButton True to hide the 'OK' button. Defaults to false.
 * @param hideCancelButton True to hide the 'Cancel' button.
 *   Defaults to false.
 * @param hideCloseButton True to hide the 'Close' button.
 *   Defaults to true.
 * @param hideResetButton True to hide the 'Reset' button.
 *   Defaults to true.
 * @param okButtonText The button text. Defaults to 'Save'.
 * @param resetButtonText The button text. Defaults to 'Reset'.
 * @param cancelButtonText The button text. Defaults to 'Cancel'.
 * @param closeButtonText The button text. Defaults to 'Close'.
 * @param mode The mode how to retrieve the data displayed in the property
 *   dialog. This can be 'local' or 'remote' which means the data is
 *   requested via RPC. Defaults to 'remote'.
 * @param readOnly True if the property values are read-only. The 'OK' and
 *   'Reset' buttons will be disabled in this case. Defaults to FALSE.
 * @param closeIfNotDirty Close the dialog immediatelly after 'OK' has been
 *   pressed and the form is not dirty. Defaults to TRUE.
 * @param autoLoadData Automatically execute the doLoad method after
 *   the component has been initialized. This only applies if mode is set to
 *   'remote'. Defaults to TRUE.
 */
Ext.define("OMV.workspace.window.Container", {
	extend: "OMV.window.Window",
	requires: [
		"OMV.Rpc"
	],
	uses: [
		"OMV.window.MessageBox"
	],

	width: 400,
	layout: "fit",
	modal: true,
	mode: "remote",
	rpcSetPollStatus: false,
	hideOkButton: false,
	hideResetButton: false,
	hideCancelButton: false,
	hideCloseButton: true,
	okButtonText: _("Save"),
	resetButtonText: _("Reset"),
	cancelButtonText: _("Cancel"),
	closeButtonText: _("Close"),
	buttonAlign: "center",
	readOnly: false,
	closeIfNotDirty: true,
	autoLoadData: true,
	submitMsg: _("Saving ..."),

	constructor: function(config) {
		var me = this;
		me.callParent(arguments);
		me.addEvents(
			/**
			 * Fires before the loading RPC is executed.
			 * @param this The window object.
			 * @param options The RPC options.
			 */
			"beforeload",
			/**
			 * Fires after the content has been loaded successful.
			 * @param this The window object.
			 * @response The RPC response.
			 */
			"load",
			/**
			 * Fires before the saving RPC is executed.
			 * @param this The window object.
			 * @param options The RPC options.
			 */
			"beforesubmit",
			/**
			 * Fires after the content has been submitted successful.
			 * @param this The window object.
			 * @param values The values displayed in the window.
			 * @response The RPC response.
			 */
			"submit",
			/**
			 * Fires when the RPC has been failed.
			 * @param this The window object.
			 * @response The RPC response.
			 */
			"exception"
		);
	},

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			buttons: me.getButtonItems(me),
			items: me.getWindowItems(me)
		});
		me.callParent(arguments);
		if(Ext.isDefined(me.rpcGetMethod) && me.autoLoadData &&
		  (me.mode === "remote")) {
			// Force loading after the component markup is rendered.
			me.on({
				single: true,
				render: me.doLoad
			});
		}
	},

	/**
	 * Initialize the window items.
	 * @param c This component object.
	 */
	getWindowItems: Ext.emptyFn,

	/**
	 * Returns the buttons displayed in the property window.
	 * @param c This component object.
	 */
	getButtonItems: function(c) {
		var me = this;
		return [{
			id: me.getId() + "-ok",
			xtype: "button",
			text: me.okButtonText,
			hidden: me.hideOkButton,
			disabled: me.readOnly,
			handler: Ext.Function.bind(me.onOkButton, me, [ me ]),
			scope: me
		},{
			id: me.getId() + "-reset",
			xtype: "button",
			text: me.resetButtonText,
			hidden: me.hideResetButton,
			disabled: me.readOnly,
			handler: Ext.Function.bind(me.onResetButton, me, [ me ]),
			scope: me
		},{
			id: me.getId() + "-cancel",
			xtype: "button",
			text: me.cancelButtonText,
			hidden: me.hideCancelButton,
			handler: Ext.Function.bind(me.onCancelButton, me, [ me ]),
			scope: me
		},{
			id: me.getId() + "-close",
			xtype: "button",
			text: me.closeButtonText,
			hidden: me.hideCloseButton,
			handler: Ext.Function.bind(me.onCloseButton, me, [ me ]),
			scope: me
		}];
	},

	/**
	 * Validate the window content.
	 * @return Returns TRUE if client-side validation on the window content
	 *   is successful.
	 */
	isValid: function() {
		return true;
	},

	/**
	 * Clears all invalid field messages in this window.
	 */
	clearInvalid: Ext.emptyFn,

	/**
	 * Mark fields in this window invalid in bulk.
	 * @param errors Either an array in the form
	 *   [{id:'fieldId', msg:'The message'},...] or an object hash of
	 *   {id: msg, id2: msg2}
	 * @return The window itself.
	 */
	markInvalid: Ext.emptyFn,

	/**
	 * Checks if any fields in this window have changed from their original
	 * values. If the values have been loaded into the window then these are
	 * the original ones.
	 * @return Returns TRUE if any fields in this window have changed from
	 *   their original values.
	 */
	isDirty: function() {
		return false;
	},

	/**
	 * Set values for fields in this window in bulk.
	 * @param values The values to set in the window of an object hash.
	 * @return The window itself.
	 */
	setValues: Ext.emptyFn,

	/**
	 * Returns the fields in this window as an object with key/value pairs.
	 */
	getValues: function() {
		return {};
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
		if(me.fireEvent("beforeload", me, rpcOptions) === false)
			return;
		// Display waiting dialog.
		me.mask(_("Loading ..."));
		// Execute RPC.
		OMV.Rpc.request(rpcOptions);
	},

	onLoad: function(id, success, response) {
		var me = this;
		me.unmask();
		if(!success) {
			OMV.MessageBox.error(null, response);
			me.fireEvent("exception", me, response);
		} else {
			response = me.processLoadResponse(response);
			me.setValues(response);
			me.clearInvalid();
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
		if(me.mode === "remote") {
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
			if(me.fireEvent("beforesubmit", me, rpcOptions) === false)
				return;
			// Display waiting dialog.
			me.mask(me.submitMsg);
			// Execute RPC.
			OMV.Rpc.request(rpcOptions);
		} else {
			var params = me.getRpcSetParams();
			me.fireEvent("submit", me, params);
			me.close();
		}
	},

	onSubmit: function(id, success, response) {
		var me = this;
		// Is this a long running RPC? If yes, then periodically check
		// if it is still running, otherwise we are finished here and
		// we can notify listeners and close the window.
		if(me.rpcSetPollStatus) {
			if(!success) {
				me.unmask();
				OMV.MessageBox.error(null, response);
				me.fireEvent("exception", me, response);
				return;
			}
			// Execute RPC.
			OMV.Rpc.request({
				scope: me,
				callback: me.onIsRunning,
				relayErrors: true,
				rpcData: {
					service: "Exec",
					method: "isRunning",
					params: {
						filename: response
					}
				}
			});
		} else {
			me.unmask();
			if(success) {
				var values = me.getRpcSetParams();
				me.fireEvent("submit", me, values, response);
				me.close();
			} else {
				OMV.MessageBox.error(null, response);
				me.fireEvent("exception", me, response);
			}
		}
	},

	onIsRunning: function(id, success, response) {
		var me = this;
		if(!success) {
			me.unmask();
			OMV.MessageBox.error(null, response);
			me.fireEvent("exception", me, response);
		} else {
			if(response.running === true) {
				Ext.Function.defer(function() {
					// Execute RPC.
					OMV.Rpc.request({
						scope: me,
						callback: me.onIsRunning,
						relayErrors: true,
						rpcData: {
							service: "Exec",
							method: "isRunning",
							params: {
								filename: response.filename
							}
						}
					});
				}, 1000, me);
			} else {
				me.unmask();
				var values = me.getRpcSetParams();
				me.fireEvent("submit", me, values, response);
				me.close();
			}
		}
	},

	/**
	 * Method that is called when the 'OK' button is pressed.
	 * @param this The window itself.
	 */
	onOkButton: function() {
		var me = this;
		// Quit immediatelly if the property values have not been modified.
		if(me.closeIfNotDirty && !me.isDirty()) {
			me.close();
			return;
		}
		// Validate values.
		if(!me.isValid()) {
			// Do not close the property dialog.
			me.markInvalid();
		} else {
			me.doSubmit();
		}
	},

	/**
	 * Method that is called when the 'Cancel' button is pressed.
	 * @param this The window itself.
	 */
	onCancelButton: function() {
		this.close();
	},

	/**
	 * Method that is called when the 'Close' button is pressed.
	 * @param this The window itself.
	 */
	onCloseButton: function() {
		this.close();
	},

	/**
	 * Method that is called when the 'Reset' button is pressed.
	 * @param this The window itself.
	 */
	onResetButton: Ext.emptyFn,

	/**
	 * Convenience function for setting the given button disabled/enabled.
	 * @param name The name of the button which can be 'ok', 'cancel',
	 *   'close' or 'reset'.
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
	 * @param name The name of the button which can be 'ok', 'cancel',
	 *   'close' or 'reset'.
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
