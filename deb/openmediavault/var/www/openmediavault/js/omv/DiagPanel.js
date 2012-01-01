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
// require("js/omv/MessageBox.js")
// require("js/omv/data/Connection.js")

Ext.ns("OMV");

/**
 * @class OMV.DiagPanel
 * @derived Ext.Panel
 * Panel that is displaying various status informations.
 */
OMV.DiagPanel = function(config) {
	var initialConfig = {
		autoScroll: true
	};
	Ext.apply(initialConfig, config);
	OMV.DiagPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.DiagPanel, Ext.Panel, {
	initComponent : function() {
		Ext.apply(this, {
			tbar: new Ext.Toolbar({
				items: [{
					id: this.getId() + "-refresh",
					xtype: "button",
					text: "Refresh",
					icon: "images/reload.png",
					handler:this.cbRefreshBtnHdl,
					scope: this
				}]
			})
		});
		OMV.DiagPanel.superclass.initComponent.apply(this, arguments);
		this.on("render", this.doLoad, this, { delay: 10 });
	},

	/**
	 * @method doLoad
	 * Method which is called to load the content to be displayed.
	 * Override this method to customize the default behaviour.
	 */
	doLoad : function() {
	},

	/**
	 * @method cbRefreshBtnHdl
	 * Handler that is called when the 'Refresh' button in the top toolbar
	 * is pressed. Override this method to customize the default behaviour.
	 */
	cbRefreshBtnHdl : function() {
		this.doLoad();
	}
});
