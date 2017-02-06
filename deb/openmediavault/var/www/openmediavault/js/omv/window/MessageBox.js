/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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
	 * Display a info box.
	 * @param title The title bar text.
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
	 * @param title The title bar text.
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
	 * @param title The title bar text.
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
	 * @param title The title bar text.
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
		if(!Ext.isObject(error)) {
			this.failure(title, error);
			return;
		}
		var me = this, defaultHeight = 120;
		var dlg = Ext.create("Ext.Window", {
			border: false,
			title: title || _("Error"),
			modal: true,
			width: 400,
			height: defaultHeight,
			minHeight: 100,
			plain: true,
			resizable: true,
			constrain: true,
			minimizable: false,
			maximizable: false,
			stateful: false,
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
					if(Ext.isFunction(fn))
						fn.call(scope || me, me);
				}
			},{
				text: _("Show details"),
				hidden: Ext.isEmpty(error.code) &&
				  Ext.isEmpty(error.trace),
				handler: function(c, e) {
					var text = "";
					if(!Ext.isEmpty(error.code))
						text += _("Error #") + error.code + ":\n";
					if(!Ext.isEmpty(error.trace))
						text += error.trace;
					details.update(OMV.util.Format.whitespace(
					  Ext.util.Format.htmlDecode(text), "pre"));
					if(details.isVisible()) {
						details.hide();
						c.setText(_("Show details"));
					} else {
						details.show();
						c.setText(_("Hide details"));
					}
					// Recalculate dialog height.
					dlg.setHeight(null);
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
					cls: Ext.baseCSSPrefix + "message-box-icon " +
					  Ext.baseCSSPrefix + "dlg-icon " + me.ERROR
				},{
					xtype: "container",
					flex: 1,
					layout: {
						type: "vbox",
                        align: "stretch"
					},
					items: [{
						xtype: "component",
						cls: me.baseCls + "-text",
						html: (Ext.isEmpty(error.message) ?
						  _("An error has occured") :
						  Ext.util.Format.htmlDecode(error.message))
					}]
				}]
			}, details = Ext.create("Ext.panel.Panel", {
				flex: 1,
				hidden: true,
				scrollable: true,
				height: 150
			})]
		});
		return dlg.show();
	},

	/**
	 * Displays a message box with an infinitely auto-updating progress bar.
	 * @param title The title bar text
	 * @param msg The message box body text
	 */
	wait: function(title, msg) {
		return this.callParent([ msg, title || _("Please wait ...") ]);
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
		var dlg = Ext.create("Ext.Window", {
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
