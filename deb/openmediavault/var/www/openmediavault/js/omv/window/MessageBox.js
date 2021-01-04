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
// require("js/omv/util/Format.js")

/**
 * @ingroup webgui
 * @class OMV.window.MessageBox
 * @derived Ext.window.MessageBox
 * A Ext.window.MessageBox wrapper class.
 */
Ext.define("OMV.window.MessageBox", {
	extend: "Ext.window.MessageBox",

	YESCANCEL: 10,

	/**
	 * Display a confirmation box that can be answered with 'Yes' or 'No'.
	 * The 'No' button is focused by default.
	 * @param title The title bar text. Defaults to 'Confirmation'.
	 * @param msg The message box body text.
	 * @param fn (optional) The callback function invoked after the.
	 * message box is closed.
	 * @param scope (optional) The scope in which the callback is executed.
	 */
	confirm: function(title, msg, fn, scope) {
		return this.show({
			title: title || _("Confirmation"),
			msg: msg,
			modal: true,
			icon: this.QUESTION,
			buttons: this.YESNO,
			defaultFocus: "no",
			fn: fn,
			scope: scope
		});
	},

	/**
	 * Display a warning box that can be answered with 'Yes' or 'No'.
	 * The 'No' button is focused by default.
	 * @param title The title bar text. Defaults to 'Warning'.
	 * @param msg The message box body text.
	 * @param fn (optional) The callback function invoked after the.
	 * message box is closed.
	 * @param scope (optional) The scope in which the callback is executed.
	 */
	confirmWarning: function(title, msg, fn, scope) {
		return this.show({
			title: title || _("Warning"),
			msg: msg,
			modal: true,
			icon: this.WARNING,
			buttons: this.YESNO,
			defaultFocus: "no",
			fn: fn,
			scope: scope
		});
	},

	/**
	 * Display an info box.
	 * @param title The title bar text. Defaults to 'Information'.
	 * @param msg The message box body text.
	 * @param fn (optional) The callback function invoked after the.
	 * message box is closed.
	 * @param scope (optional) The scope in which the callback is executed.
	 */
	info: function(title, msg, fn, scope) {
		return this.show({
			title: title || _("Information"),
			msg: msg,
			modal: true,
			icon: this.INFO,
			buttons: this.OK,
			fn: fn,
			scope: scope
		});
	},

	/**
	 * Display a warning box.
	 * @param title The title bar text. Defaults to 'Warning'.
	 * @param msg The message box body text.
	 * @param fn (optional) The callback function invoked after the.
	 * message box is closed.
	 * @param scope (optional) The scope in which the callback is executed.
	 */
	warning: function(title, msg, fn, scope) {
		return this.show({
			title: title || _("Warning"),
			msg: msg,
			modal: true,
			icon: this.WARNING,
			buttons: this.OK,
			fn: fn,
			scope: scope
		});
	},

	/**
	 * Display a success box.
	 * @param title The title bar text. Defaults to 'Success'.
	 * @param msg The message box body text.
	 * @param fn (optional) The callback function invoked after the.
	 * message box is closed.
	 * @param scope (optional) The scope in which the callback is executed.
	 */
	success: function(title, msg, fn, scope) {
		return this.show({
			title: title || _("Success"),
			msg: msg || _("The changes have been applied successfully."),
			modal: true,
			icon: this.INFO,
			buttons: this.OK,
			fn: fn,
			scope: scope
		});
	},

	/**
	 * Display a failure box.
	 * @param title The title bar text. Defaults to 'Error'.
	 * @param msg The message box body text.
	 * @param fn (optional) The callback function invoked after the
	 *   message box is closed.
	 * @param scope (optional) The scope in which the callback is executed.
	 */
	failure: function(title, msg, fn, scope) {
		return this.show({
			title: title || _("Error"),
			msg: msg,
			modal: true,
			icon: this.ERROR,
			buttons: this.OK,
			fn: fn,
			scope: scope
		});
	},

	/**
	 * Display an error box.
	 * @param title The title bar text.
	 * @param error The error object or the message box body text. An error
	 *   object must contain the fields \em code, \em error and \em message.
	 * @param fn (optional) The callback function invoked after the
	 *   message box is closed.
	 * @param scope (optional) The scope in which the callback is executed.
	 */
	error: function(title, error, fn, scope) {
		var me = this;
		if (!Ext.isObject(error)) {
			this.failure(title, error);
			return;
		}
		// Build the details text.
		var detailsText = "";
		if (!Ext.isEmpty(error.code))
			detailsText += _("Error #") + error.code + ":\n";
		if (!Ext.isEmpty(error.trace))
			detailsText += error.trace;
		// Display the error dialog.
		var dlg = Ext.create("Ext.Window", {
			title: title || _("Error"),
			modal: true,
			width: 400,
			minHeight: 100,
			maxHeight: 600,
			resizable: true,
			constrain: true,
			minimizable: false,
			maximizable: false,
			scrollable: true,
			closable: true,
			layout: {
				type: "vbox",
				align: "stretch"
			},
			buttonAlign: "center",
			defaultFocus: 0,
			buttons: [{
				text: this.buttonText.ok,
				handler: function(c, e) {
					// Close the dialog.
					dlg.close();
					// Execute given callback function.
					if (Ext.isFunction(fn))
						fn.call(scope || me, me);
				}
			},{
				text: _("Show details"),
				hidden: Ext.isEmpty(detailsText),
				handler: function(c, e) {
					var visible = details.isVisible();
					c.setText(!visible ? _("Hide details") : _("Show details"));
					details.setVisible(!visible);
				}
			}],
			items: [{
				xtype: "container",
				layout: "hbox",
				padding: 10,
				style: {
					overflow: "hidden"
				},
				items: [{
					xtype: "component",
					cls: [
						Ext.baseCSSPrefix + "message-box-icon",
						Ext.baseCSSPrefix + "dlg-icon",
						me.ERROR
					]
				},{
					xtype: "container",
					flex: 1,
					layout: {
						type: "vbox",
						align: "stretch"
					},
					items: [{
						xtype: "component",
						cls: [
							Ext.baseCSSPrefix + "selectable",
							me.baseCls + "-text"
						],
						html: (Ext.isEmpty(error.message) ?
							_("An error has occurred") :
							Ext.util.Format.htmlEncode(error.message))
					}]
				}]
			}, details = Ext.create("Ext.panel.Panel", {
				cls: Ext.baseCSSPrefix + "selectable",
				border: true,
				padding: 10,
				flex: 1,
				hidden: true,
				scrollable: true,
				minHeight: 175,
				html: OMV.util.Format.whitespace(detailsText, "pre")
			})]
		});
		/*
		// Display the details in a collapsible panel.
		var dlg = Ext.create("Ext.Window", {
			title: title || _("Error"),
			modal: true,
			width: 400,
			minHeight: 100,
			resizable: true,
			constrain: true,
			minimizable: false,
			maximizable: false,
			closable: true,
			layout: {
				type: "vbox",
				align: "stretch"
			},
			buttonAlign: "center",
			defaultFocus: 0,
			buttons: [{
				text: this.buttonText.ok,
				handler: function(c, e) {
					// Close the dialog.
					dlg.close();
					// Execute given callback function.
					if (Ext.isFunction(fn))
						fn.call(scope || me, me);
				}
			}],
			items: [{
				flex: 1,
				layout: "hbox",
				padding: 10,
				style: {
					overflow: "hidden"
				},
				items: [{
					xtype: "component",
					cls: [
						Ext.baseCSSPrefix + "message-box-icon",
						Ext.baseCSSPrefix + "dlg-icon",
						me.ERROR
					]
				},{
					xtype: "container",
					flex: 1,
					scrollable: true,
					layout: {
						type: "vbox",
						align: "stretch"
					},
					items: [{
						xtype: "component",
						cls: [
							Ext.baseCSSPrefix + "selectable",
							me.baseCls + "-text"
						],
						html: (Ext.isEmpty(error.message) ?
							_("An error has occurred") :
							Ext.util.Format.htmlEncode(error.message))
					}]
				}]
			},{
				xtype: "panel",
				title: _("Details"),
				cls: Ext.baseCSSPrefix + "selectable",
				flex: 1,
				animCollapse: false,
				titleCollapse: true,
				collapsible: true,
				collapsed: true,
				scrollable: true,
				hidden: Ext.isEmpty(detailsText),
				html: detailsText
			}]
		});
		*/
		return dlg.show();
	},

	/**
	 * Displays a message box with an infinitely auto-updating progress bar.
	 * @param title The title bar text
	 * @param msg The message box body text
	 * @param config A Ext.ProgressBar#wait config object.
	 */
	wait: function(title, msg, config) {
		return this.callParent([ msg, title || _("Please wait ..."), config ]);
	},

	/**
	 * Displays a message box with a progress bar. This message box has
	 * no buttons and is not closeable by the user.
	 */
	progress: function(title, msg, progressText) {
		return this.callParent([ title || _("Please wait ..."), msg,
		  progressText ]);
	},

	/**
	 * Display a modal message box at the top of the page which is inspired
	 * by the Amiga Guru Meditation error notice. Thanks for having so much
	 * fun with the best gaming machine ever :-)
	 * See http://en.wikipedia.org/wiki/Guru_Meditation and
	 * http://www.amigahistory.co.uk/guruguide.html
	 * @param config The following config options are supported:
	 *   \li msg The message text.
	 *   \li fn A callback function which is called when a mouse button
	 *     or key has been pressed.
	 *   \li scope The scope (this reference) in which the function will
	 *     be executed.
	 *   \li closable Set to FALSE to ignore mouse clicks and indefinitely
	 *     display the message box.
	 * @return The window object.
	 */
	guru: function(config) {
		var me = this;
		// Note, use the ExtJS window class to get some features out-of-the
		// box (e.g. auto z-index the dialog to top when the rest of the
		// viewport is masked). The plain HTML implementation is prevered
		// when the z-index issue is solved. The problem here is how to
		// calculate it, this is normally done by the Ext.ZIndexManager
		// class.
		var dlg = Ext.create("Ext.window.Window", {
			border: true,
			modal: true,
			width: "100%",
			height: "auto",
			header: false,
			resizable: false,
			constrain: true,
			stateful: false,
			closable: false,
			x: 0,
			y: 0,
			baseCls: Ext.baseCSSPrefix + "message-box-guru",
			html: Ext.String.format("Software Failure.&nbsp;&nbsp;&nbsp;" +
			  "Press left mouse button to continue.<br/>{0}", config.msg)
		});
		if (!Ext.isDefined(config.closable) || (true === config.closable)) {
			// Monitor key press and mouse clicks.
			var fn = function(e, t, eOpts) {
				// Unregister event handlers.
				Ext.getBody().un({
					keypress: fn,
					click: fn
				});
				// Remove message box.
				dlg.close();
				// Execute given callback function.
				if (Ext.isFunction(config.fn))
					config.fn.call(config.scope || me, me);
			};
			Ext.getBody().on({
				keypress: fn,
				click: fn
			});
		}
		return dlg.show();
	}
}, function() {
	/**
	 * @class OMV.MessageBox
	 * @derived OMV.window.MessageBox
	 * @singleton
	 * Singleton instance of {@link OMV.window.MessageBox}.
	 */
	OMV.MessageBox = OMV.Msg = new this();
});
