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
// require("js/omv/tree/Folder.js")
// require("js/omv/window/Window.js")
// require("js/omv/window/MessageBox.js")

/**
 * @class OMV.window.FolderBrowser
 * @derived OMV.window.Window
 * @param uuid The UUID of the volume to process. Required.
 */
Ext.define("OMV.window.FolderBrowser", {
	extend: "OMV.window.Window",
	uses: [
		"OMV.tree.Folder",
		"OMV.window.MessageBox"
	],

	title: _("Select a directory"),
	width: 300,
	height: 400,
	layout: "fit",
	modal: true,
	border: true,
	buttonAlign: "center",

	constructor: function() {
		var me = this;
		me.callParent(arguments);
		me.addEvents(
			/**
			 * Fires after the dialog has been closed by pressing the
			 * 'OK' button.
			 * @param this The window object.
			 * @param node The selected tree node.
			 * @param path The selected directory path.
			 */
			"select"
		);
	},

	initComponent: function() {
		var me = this;
		Ext.apply(me, {
			buttons: [{
				id: me.getId() + "-ok",
				text: _("OK"),
				disabled: true,
				handler: me.onOkButton,
				scope: me
			},{
				text: _("Cancel"),
				handler: me.close,
				scope: me
			}],
			items: [ me.tp = Ext.create("OMV.tree.Folder", {
				uuid: me.uuid,
				border: false,
				listeners: {
					scope: me,
					select: function(tree, record, index, eOpts) {
						// Enable the 'OK' button.
						var button = me.queryById(me.getId() + "-ok");
						button.setDisabled(false);
					}
				}
			}) ]
		});
		me.callParent(arguments);
	},

	/**
	 * @method onOkButton
	 * Method that is called when the 'OK' button is pressed.
	 */
	onOkButton: function() {
		var me = this;
		var selModel = me.tp.getSelectionModel();
		var node = selModel.getSelection()[0];
		// Fire event before window is closed.
		me.fireEvent("select", me, node, me.tp.getPathFromNode(node));
		me.close();
	}
});
