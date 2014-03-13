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
 * @class OMV.window.Execute
 * @derived OMV.window.Window
 * Execute the given command asynchronously and display the command output
 * in the dialog. The command will be executed right after the dialog has
 * been rendered. By pressing the 'Stop' button a RPC will be send to stop
 * the command. The 'Close' button will be shown then to close the dialog.
 * @param rpcService The name of the RPC service
 * @param rpcMethod The name of the method to start the command asynchronously
 * @param rpcParams The method arguments of the start RPC
 * @param rpcDelay The milliseconds to delay the RPC request. Default is 500.
 * @param rpcIgnoreErrors Ignore RPC errors. Set to TRUE to ignore all errors
 *   or list special ones in an array. Defaults to FALSE.
 * @param hideStartButton Hide the 'Start' button. Defaults to FALSE.
 * @param hideStopButton Hide the 'Edit' button. Defaults to FALSE.
 * @param hideCloseButton Hide the 'Close' button. Defaults to FALSE.
 * @param adaptButtonState Automatically adjust the button states while the
 *   command is running. Defaults to TRUE.
 * @param progress TRUE to display a progress bar. This is useful when the
 *   executed application does not have any output to be displayed in the
 *   content area. If set to TRUE the content area will not be displayed.
 *   Defaults to FALSE.
 * @param progressText The progress bar text. Defaults to 'Please wait ...".
 * @param scrollBottom Set to TRUE to automatically scroll down the content.
 *   Defaults to TRUE.
 */
Ext.define("OMV.window.Execute", {
	extend: "OMV.window.Window",
	uses: [
		"OMV.Rpc",
		"OMV.window.MessageBox"
	],

	title: _("Execute command"),
	width: 600,
	height: 380,
	layout: "fit",
	modal: true,
	border: false,
	buttonAlign: "center",
	closable: false,

	rpcDelay: 500,
	rpcIgnoreErrors: false,
	rpcIgnoreErrorCodes: [],
	hideStartButton: false,
	hideStopButton: false,
	hideCloseButton: false,
	adaptButtonState: true,
	progress: false,
	progressText: _("Please wait ..."),
	scrollBottom: true,

	cmdIsRunning: false,
	getContentAllowed: false,

	constructor: function() {
		var me = this;
		me.callParent(arguments);
		me.addEvents(
			/**
			 * Fires when the command execution has been started.
			 * @param this The window object.
			 */
			"start",
			/**
			 * Fires when the command execution has been finished.
			 * @param this The window object.
			 * @param response The RPC response object.
			 */
			"finish",
			/**
			 * Fires when the command execution has been failed.
			 * @param this The window object.
			 * @param response The RPC response object.
			 */
			"exception"
		);
	},

	initComponent: function() {
		var me = this;
		if(false === me.progress) {
			me.contentCtrl = Ext.create("Ext.form.field.TextArea", {
				name: "content"
			});
		} else {
			me.height = 58;
			me.content = "";
			me.contentCtrl = Ext.create("Ext.ProgressBar", {
				text: me.progressText
			});
		}
		Ext.apply(me, {
			buttons: [{
				id: me.getId() + "-start",
				text: _("Start"),
				hidden: me.hideStartButton,
				handler: me.start,
				scope: me
			},{
				id: me.getId() + "-stop",
				text: _("Stop"),
				hidden: me.hideStopButton,
				disabled: true,
				handler: me.stop,
				scope: me
			},{
				id: me.getId() + "-close",
				text: _("Close"),
				hidden: me.hideCloseButton,
				handler: me.close,
				scope: me
			}],
			items: [
				me.contentCtrl
			]
		});
		me.callParent(arguments);
	},

	/**
	 * Start the process.
	 */
	start: function() {
		var me = this;
		// Execute RPC.
		OMV.Rpc.request({
			  scope: me,
			  callback: function(id, success, response) {
				  if(success) {
					  this.getContentAllowed = true;
					  // Command has been executed successfully. Remember the
					  // execution command identifier.
					  this.bgStatusFilename = response;
					  this.cmdContentPos = 0;
					  // Reset controls.
					  this.setValue("");
					  if(true === this.progress) {
						  this.contentCtrl.reset();
						  this.contentCtrl.wait({
							  text: this.progressText
						  });
					  }
					  // Update the button states.
					  this.setButtonDisabled("start", true);
					  this.setButtonDisabled("stop", false);
					  this.setButtonDisabled("close", true);
					  // Fire event.
					  this.fireEvent("start", this);
					  // Begin to request the command output.
					  this.doGetOutput();
				  } else {
					  // Enable 'Close' button to be able to close the dialog.
					  this.setButtonDisabled("start", true);
					  this.setButtonDisabled("close", false);
					  // Fire exception event to notify listeners.
					  this.fireEvent("exception", this, response);
				  }
			  },
			  relayErrors: true,
			  rpcData: {
				  service: me.rpcService,
				  method: me.rpcMethod,
				  params: me.rpcParams
			  }
		  });
	},

	/**
	 * Method that is called when the 'Stop' button is pressed.
	 */
	stop: function() {
		var me = this;
		me.getContentAllowed = false;
		// Execute RPC
		OMV.Rpc.request({
			  scope: me,
			  callback: function(id, success, response) {
				  if(true === this.progress) {
					  this.contentCtrl.reset();
				  }
				  if(success) {
					  // Update the button states.
					  this.setButtonDisabled("start", false);
					  this.setButtonDisabled("stop", true);
					  this.setButtonDisabled("close", false);
				  } else {
					  // Enable 'Close' button to be able to close the dialog.
					  this.setButtonDisabled("stop", true);
					  this.setButtonDisabled("close", false);
					  // Fire exception event to notify listeners.
					  this.fireEvent("exception", this, response);
				  }
			  },
			  relayErrors: true,
			  rpcData: {
				  service: "Exec",
				  method: "stop",
				  params: {
					  filename: me.bgStatusFilename
				  }
			  }
		  });
	},

	/**
	 * Get the command output via RPC.
	 * @private
	 */
	doGetOutput: function() {
		var me = this;
		// Is command still running? It might happen that the function is
		// called after the 'stop' RPC has been executed because the function
		// is called delayed. In this case simply do not execute the RPC.
		if(me.getContentAllowed === true) {
			// Execute RPC.
			OMV.Rpc.request({
				  scope: me,
				  callback: function(id, success, response) {
					  if(success) {
						  this.cmdContentPos = response.pos;
						  this.cmdIsRunning = response.running;
						  // Update the command content.
						  this.appendValue(response.output);
						  // If command is still running then do another RPC
						  // request.
						  if(this.cmdIsRunning === true) {
							  Ext.Function.defer(this.doGetOutput,
								this.rpcDelay, this);
						  } else {
							  if(true === this.progress) {
								  this.contentCtrl.reset();
							  }
							  this.fireEvent("finish", this, response);
						  }
						  // Update button states.
						  if(true === this.adaptButtonState) {
							  this.setButtonDisabled("start",
								this.cmdIsRunning);
							  this.setButtonDisabled("stop",
								!this.cmdIsRunning);
							  this.setButtonDisabled("close",
								this.cmdIsRunning);
						  }
					  } else {
						  var ignore = false;
						  if(this.rpcIgnoreErrors === true)
							  ignore = true;
						  else if(Ext.isArray(this.rpcIgnoreErrors)) {
							  // Check if there are defined some special error
							  // codes that should be ignored.
							  ignore = Ext.Array.contains(this.rpcIgnoreErrors,
								response.code);
						  }
						  // Ignore RPC errors?
						  if(ignore === true) {
							  // Execute another RPC.
							  Ext.Function.defer(this.doGetOutput,
								this.rpcDelay, this);
						  } else {
							  // Enable 'Close' button to be able to close the
							  // dialog.
							  this.setButtonDisabled("stop", true);
							  this.setButtonDisabled("close", false);
							  // Fire exception to allow listeners to react
							  // on errors.
							  this.fireEvent("exception", this, response);
						  }
					  }
				  },
				  relayErrors: true,
				  rpcData: {
					  service: "Exec",
					  method: "getOutput",
					  params: {
						  filename: me.bgStatusFilename,
						  pos: me.cmdContentPos
					  }
				  }
			  });
		}
	},

	/**
	 * Get the content displayed in the dialog.
	 * @return The content displayed in the dialog.
	 */
	getValue: function() {
		var me = this;
		var value = "";
		if(false === me.progress) {
			value = me.contentCtrl.getValue();
		} else {
			value = me.content;
		}
		return value;
	},

	/**
	 * Set the content displayed in the dialog.
	 * @param value The value to be displayed in the dialog.
	 * @return None
	 */
	setValue: function(value, scrollBottom) {
		var me = this;
		if(false === me.progress) {
			me.contentCtrl.setValue(value);
			if(true === me.scrollBottom) {
				var el = me.contentCtrl.inputEl;
				if(el && el.dom)
					el.dom.scrollTop = el.dom.scrollHeight
			}
		} else {
			me.content = value;
		}
	},

	/**
	 * Appends the given value to the displayed content.
	 * @param value The value to be appended to the displayed content.
	 * @return None
	 */
	appendValue: function(value) {
		var me = this;
		if(false === me.progress) {
			var content = me.getValue();
			me.setValue(content + value);
		} else {
			me.content += value;
		}
	},

	/**
	 * Convenience function for setting the given button disabled/enabled.
	 * @param name The name of the button which can be 'start', 'stop'
	 *   or 'close'.
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
	 * @param name The name of the button which can be 'start', 'stop'
	 *   or 'close'.
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
