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
// require("js/omv/Window.js")
// require("js/omv/MessageBox.js")

Ext.ns("OMV");

/**
 * @class OMV.ExecCmdDialog
 * @derived OMV.Window
 * Execute the given command asynchronously and display the command output
 * in the dialog. The command will be executed right after the dialog has
 * been rendered. By pressing the 'Stop' button a RPC will be send to stop
 * the command. The 'Close' button will be shown then to close the dialog.
 * @config rpcService The name of the RPC service
 * @config rpcMethod The name of the method to start the command asynchronously
 * @config rpcArgs The method arguments of the start RPC
 * @config rpcDelay The milliseconds to delay the RPC request. Default is 500.
 * @config rpcIgnoreErrors Ignore RPC errors. Defaults to FALSE.
 * @config hideStart Hide the 'Start' button. Defaults to FALSE.
 * @config hideStop Hide the 'Edit' button. Defaults to FALSE.
 * @config hideClose Hide the 'Close' button. Defaults to FALSE.
 * @config progress TRUE to display a progress bar. This is useful when the
 *   executed application does not have any output to be displayed in the
 *   content area. If set to TRUE the content area will not be displayed and
 *   the dialog will be set to autoHeight. Defaults to FALSE.
 * @config progressText The progress bar text. Defaults to 'Please wait ...".
 */
OMV.ExecCmdDialog = function(config) {
	var initialConfig = {
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
		cmdIsRunning: false,
		getContentAllowed: false,
		hideStart: false,
		hideStop: false,
		hideClose: false,
		progress: false,
		progressText: _("Please wait ...")
	};
	Ext.apply(initialConfig, config);
	OMV.ExecCmdDialog.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires when the command execution has been started.
		 */
		"start",
		/**
		 * Fires when the command execution has been finished.
		 */
		"finish",
		/**
		 * Fires when the command execution has been failed.
		 */
		"exception"
	);
};
Ext.extend(OMV.ExecCmdDialog, OMV.Window, {
	initComponent : function() {
		if (false === this.progress) {
			this.contentCtrl = new Ext.form.TextArea({
				name: "content",
				autoCreate: {
					tag: "textarea",
					autocomplete: "off"
				}
			});
		} else {
			this.content = "";
			this.contentCtrl = new Ext.ProgressBar({
				text: this.progressText
			});
		}
		Ext.apply(this, {
			autoHeight: this.progress,
			buttons: [{
				id: this.getId() + "-start",
				text: _("Start"),
				hidden: this.hideStart,
				handler: this.start,
				scope: this
			},{
				id: this.getId() + "-stop",
				text: _("Stop"),
				hidden: this.hideStop,
				disabled: true,
				handler: this.stop,
				scope: this
			},{
				id: this.getId() + "-close",
				text: _("Close"),
				hidden: this.hideClose,
				handler: function() {
					this.close();
				},
				scope: this
			}],
			items: this.contentCtrl
		});
		OMV.ExecCmdDialog.superclass.initComponent.apply(this, arguments);
	},

	/**
	 * @method start
	 * Start the process.
	 */
	start : function() {
		OMV.Ajax.request(function(id, response, error) {
			  if (error === null) {
				  this.getContentAllowed = true;
				  // Command has been executed successfully. Remember the
				  // execution command identifier.
				  this.cmdId = response;
				  this.cmdContentPos = 0;
				  // Reset controls.
				  this.setValue("");
				  if (true === this.progress) {
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
				  this.fireEvent("exception", this, error);
			  }
		  }, this, this.rpcService, this.rpcMethod, this.rpcArgs);
	},

	/**
	 * @method stop
	 * Method that is called when the 'Stop' button is pressed.
	 */
	stop : function() {
		this.getContentAllowed = false;
		OMV.Ajax.request(function(id, response, error) {
			  if (true === this.progress) {
				  this.contentCtrl.reset();
			  }
			  if (error === null) {
				  // Update the button states
				  this.setButtonDisabled("start", false);
				  this.setButtonDisabled("stop", true);
				  this.setButtonDisabled("close", false);
			  } else {
				  this.fireEvent("exception", this, error);
			  }
		  }, this, "Exec", "stop", { "id": this.cmdId });
	},

	/**
	 * @method doGetOutput
	 * Get the command output via RPC.
	 * @private
	 */
	doGetOutput : function() {
		// Is command still running? It might happen that the function is
		// called after the 'stop' RPC has been executed because the function
		// is called delayed. In this case simply do not execute the RPC.
		if (this.getContentAllowed === true) {
			OMV.Ajax.request(function(id, response, error) {
				  if (error === null) {
					  this.cmdContentPos = response.pos;
					  this.cmdIsRunning = response.running;
					  // Update the command content.
					  this.appendValue(response.output);
					  // If command is still running then do another RPC
					  // request.
					  if (this.cmdIsRunning === true) {
						  this.doGetOutput.defer(this.rpcDelay, this);
					  } else {
						  if (true === this.progress) {
							  this.contentCtrl.reset();
						  }
						  this.fireEvent("finish", this, response);
					  }
					  // Update button states
					  this.setButtonDisabled("start", this.cmdIsRunning);
					  this.setButtonDisabled("stop", !this.cmdIsRunning);
					  this.setButtonDisabled("close", this.cmdIsRunning);
				  } else {
					  // Ignore RPC errors?
					  if (this.rpcIgnoreErrors === true) {
						  // Execute another RPC.
						  this.doGetOutput.defer(this.rpcDelay, this);
					  } else {
						  // Fire exception to allow listeners to react
						  // on errors.
						  this.fireEvent("exception", this, error);
					  }
				  }
			  }, this, "Exec", "getOutput", { "id": this.cmdId,
			  "pos": this.cmdContentPos });
		}
	},

	/**
	 * @method getValue
	 * Get the content displayed in the dialog.
	 * @return The content displayed in the dialog.
	 */
	getValue : function() {
		var value = "";
		if (false === this.progress) {
			value = this.contentCtrl.getValue();
		} else {
			value = this.content;
		}
		return value;
	},

	/**
	 * @method setValue
	 * Set the content displayed in the dialog.
	 * @param value The value to be displayed in the dialog.
	 * @scrollBottom TRUE to scroll to the end of the displayed content.
	 *   Defaults to TRUE.
	 * @return None
	 */
	setValue : function(value, scrollBottom) {
		if (false === this.progress) {
			scrollBottom = scrollBottom || true;
			this.contentCtrl.setValue(value);
			if (scrollBottom === true) {
				// http://www.extjs.com/forum/showthread.php?t=49096
				var el = this.contentCtrl.getEl();
				if (el && el.dom) {
					el.dom.scrollTop = el.dom.scrollHeight
				}
			}
		} else {
			this.content = value;
		}
	},

	/**
	 * @method appendValue
	 * Appends the given value to the displayed content.
	 * @param value The value to be appended to the displayed content.
	 * @scrollBottom TRUE to scroll to the end of the displayed content.
	 *   Defaults to TRUE.
	 * @return None
	 */
	appendValue : function(value, scrollBottom) {
		if (false === this.progress) {
			var content = this.getValue();
			scrollBottom = Ext.isDefined(scrollBottom) ? scrollBottom : true;
			this.setValue(content + value, scrollBottom);
		} else {
			this.content += value;
		}
	},

	/**
	 * @method setButtonDisabled
	 * Convenience function for setting the given button disabled/enabled.
	 * @param name The name of the button which can be 'start', 'stop'
	 *   or 'close'.
	 * @param disabled TRUE to disable the button, FALSE to enable.
	 * @return The button component, otherwise FALSE.
	 */
	setButtonDisabled : function(name, disabled) {
		var btnCtrl = Ext.getCmp(this.getId() + "-" + name);
		if (!Ext.isDefined(btnCtrl))
			return false;
		return btnCtrl.setDisabled(disabled);
	},

	/**
	 * @method setButtonVisible
	 * Convenience function to show or hide the given button.
	 * @param name The name of the button which can be 'start', 'stop'
	 *   or 'close'.
	 * @param visible TRUE to show the button, FALSE to hide.
	 * @return The button component, otherwise FALSE.
	 */
	setButtonVisible : function(name, visible) {
		var btnCtrl = Ext.getCmp(this.getId() + "-" + name);
		if (!Ext.isDefined(btnCtrl))
			return false;
		return btnCtrl.setVisible(visible);
	}
});
