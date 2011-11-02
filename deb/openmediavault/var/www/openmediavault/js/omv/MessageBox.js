/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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

Ext.ns("OMV");

/**
 * A Ext.MessageBox wrapper class.
 */
OMV.MessageBox = function() {
	return {
		/**
		 * Displays a new message box, or reinitializes an existing message
		 * box, based on the config options passed in.
		 */
		show : function(options) {
			return Ext.Msg.show(options);
		},

		/**
		 * Hides the message box if it is displayed.
		 */
		hide : function() {
			return Ext.Msg.hide();
		},

		/**
		* Display a info box.
		* @param title The title bar text.
		* @param msg The message box body text.
		* @param fn (optional) The callback function invoked after the.
		* message box is closed.
		* @param scope (optional) The scope in which the callback is executed.
		*/
		info : function(title, msg, fn, scope) {
			return this.show({
				title: title || "Information",
				msg: msg,
				modal: true,
				icon: Ext.Msg.INFO,
				buttons: Ext.Msg.OK,
				fn: fn,
				scope: scope
			});
		},

		/**
		* Display a warning box.
		* @param title The title bar text.
		* @param msg The message box body text.
		* @param fn (optional) The callback function invoked after the.
		* message box is closed.
		* @param scope (optional) The scope in which the callback is executed.
		*/
		warning : function(title, msg, fn, scope) {
			return this.show({
				title: title || "Warning",
				msg: msg,
				modal: true,
				icon: Ext.Msg.WARNING,
				buttons: Ext.Msg.OK,
				fn: fn,
				scope: scope
			});
		},

		/**
		* Display a success box.
		* @param title The title bar text.
		* @param msg The message box body text.
		* @param fn (optional) The callback function invoked after the.
		* message box is closed.
		* @param scope (optional) The scope in which the callback is executed.
		*/
		success : function(title, msg, fn, scope) {
			return this.show({
				title: title || "Success",
				msg: msg || "The changes have been applied successfully.",
				modal: true,
				icon: Ext.Msg.INFO,
				buttons: Ext.Msg.OK,
				fn: fn,
				scope: scope
			});
		},

		/**
		* Display a failure box.
		* @param title The title bar text.
		* @param msg The message box body text.
		* @param fn (optional) The callback function invoked after the.
		* message box is closed.
		* @param scope (optional) The scope in which the callback is executed.
		*/
		failure : function(title, msg, fn, scope) {
			return this.show({
				title: title || "Error",
				msg: msg,
				modal: true,
				icon: Ext.Msg.ERROR,
				buttons: Ext.Msg.OK,
				fn: fn,
				scope: scope
			});
		},

		/**
		* Display an error box.
		* @param title The title bar text.
		* @param error The error object or the message box body text.
		*/
		error : function(title, error) {
			if (!Ext.isObject(error)) {
				this.failure(title, error);
				return;
			}
			var dlg = new Ext.Window({
				autoCreate : true,
				border: false,
				hideBorders: true,
				title: title || "Error",
				modal: true,
				width: 400,
				height: 150,
				minHeight: 100,
				shim: true,
				plain: true,
				footer: true,
				resizable: true,
				constrain: true,
				constrainHeader: true,
				minimizable: false,
				maximizable: false,
				stateful: false,
				closable: true,
				layout: "vbox",
				layoutConfig: {
					pack: "start",
					align: "stretch"
				},
				buttonAlign: "center",
				buttons: [{
					text: Ext.Msg.buttonText.ok,
					handler: function(c, e) {
						dlg.close();
					}
				},{
					text: "Show details",
					hidden: Ext.isEmpty(error.code) &&
					  Ext.isEmpty(error.trace),
					handler: function(c, e) {
						if (Ext.isEmpty(dlg.detailCt)) {
							var text = "";
							if (!Ext.isEmpty(error.code))
								text += "Error #" + error.code + ":\n";
							if (!Ext.isEmpty(error.trace))
								text += error.trace;
							dlg.detailCt = new Ext.Panel({
								autoScroll: true,
								hidden: true,
								flex: 2,
								html: "<pre>" + Ext.util.Format.htmlEncode(
								  text) + "</pre>"
							});
							dlg.add(dlg.detailCt);
						}
						if (dlg.detailCt.isVisible()) {
							dlg.detailCt.hide();
							dlg.setHeight(150);
							c.setText("Show details");
						} else {
							dlg.detailCt.show();
							var height = dlg.getHeight();
							dlg.setHeight(height + 150);
							c.setText("Hide details");
						}
						dlg.doLayout();
					}
				}],
				items: [{
					xtype: "panel",
					flex: 1,
					bodyCssClass: "x-window-dlg",
					bodyStyle: "padding: 5px; background: none transparent;",
					html: "<div class='ext-mb-icon ext-mb-error'></div>" +
					  "<div class='ext-mb-content'>" +
					  "<span class='ext-mb-text'>" + (Ext.isEmpty(
						error.message) ? "An error has occured" :
						Ext.util.Format.htmlEncode(error.message)) +
					  "</span></div>"
				}],
				detailCt: null
			});
			return dlg.show();
		},

		/**
		 * Displays a message box with an infinitely auto-updating progress bar.
		 * @param title The title bar text
		 * @param msg The message box body text
		 */
		wait : function(title, msg) {
			return Ext.Msg.wait(msg, title || "Please wait ...");
		},

		/**
		 * Displays a message box with a progress bar. This message box has
		 * no buttons and is not closeable by the user.
		 */
		progress : function(title, msg, progressText) {
			return Ext.Msg.progress(title || "Please wait ...", msg, progressText);
		},

		/**
		 * Updates a progress-style message box's text and progress bar.
		 */
		updateProgress : function(value, progressText, msg) {
			return Ext.Msg.updateProgress(value, progressText, msg);
		}
	};
}();
OMV.Msg = OMV.MessageBox;
