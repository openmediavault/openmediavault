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

/**
 * @ingroup webgui
 * @class OMV.workspace.panel.Panel
 * @derived Ext.panel.Panel
 * @param hideTopToolbar TRUE to hide the whole toolbar. Defaults
 *   to TRUE.
 * @param hideRefreshButton Hide the 'Refresh' button in the top toolbar.
 *   Defaults to FALSE.
 * @param refreshButtonText The 'Refresh' button text. Defaults to 'Refresh'.
 * @param autoLoadData Automatically execute the doLoad method after
 *   render. Defaults to FALSE.
 */
Ext.define("OMV.workspace.panel.Panel", {
	extend: "Ext.panel.Panel",

	hideTopToolbar: true,
	hideRefreshButton: false,
	refreshButtonText: _("Refresh"),
	refreshButtonIconCls: "x-fa fa-refresh",
	autoLoadData: false,

	border: false,
	scrollable: true,
	header: false,

	initComponent: function() {
		var me = this;
		// Initialize toolbar.
		me.dockedItems = [];
		if(!me.hideTopToolbar) {
			me.dockedItems.push(me.topToolbar = Ext.widget({
				xtype: "toolbar",
				dock: "top",
				items: me.getTopToolbarItems(me)
			}));
		}
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
	 * Returns the items displayed in the top toolbar.
	 * @param c This component object.
	 * @return An array of buttons displayed in the top toolbar.
	 */
	getTopToolbarItems: function(c) {
		var me = this;
		return [{
			id: me.getId() + "-refresh",
			xtype: "button",
			text: me.refreshButtonText,
			iconCls: me.refreshButtonIconCls,
			hidden: me.hideRefreshButton,
			handler: Ext.Function.bind(me.onRefreshButton, me, [ me ]),
			scope: me
		}];
	},

	/**
	 * Method which is called to load the content to be displayed.
	 * Override this method to customize the default behaviour.
	 */
	doLoad: function() {
		// Nothing to do here by default.
	},

	/**
	 * Handler that is called when the 'Refresh' button in the top toolbar
	 * is pressed. Override this method to customize the default behaviour.
	 */
	onRefreshButton: function() {
		this.doLoad();
	}
});
