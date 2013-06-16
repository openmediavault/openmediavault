/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
	autoLoadData: false,

	border: false,
	autoScroll: true,

	initComponent: function() {
		var me = this;
		if(!me.hideTopToolbar) {
			Ext.apply(me, {
				dockedItems: [{
					xtype: "toolbar",
					dock: "top",
					items: me.getTopToolbarItems()
				}]
			});
		}
		me.callParent(arguments);
		if(me.autoLoadData) {
			me.doLoad();
		}
	},

	/**
	 * Returns the items displayed in the top toolbar.
	 */
	getTopToolbarItems: function() {
		var me = this;
		return [{
			id: me.getId() + "-refresh",
			xtype: "button",
			text: me.refreshButtonText,
			icon: "images/refresh.png",
			iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
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
