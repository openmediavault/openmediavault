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
// require("js/omv/form/FormPanel.js")

Ext.ns("OMV");

/**
 * @class OMV.FormPanelExt
 * @derived OMV.form.FormPanel
 * A enhanced form panel that automatically loads the data after the panel has
 * been rendered. It will also display an array of buttons at the bottom of
 * the panel. By default these are the 'OK' and 'Reset' buttons.
 * @config hideOK Hide the 'OK' button.
 * @config hideReset Hide the 'Reset' button.
 */
OMV.FormPanelExt = function(config) {
	var initialConfig = {
		hideOK: false,
		hideReset: false,
		autoScroll: true,
		buttonAlign: "left",
		trackResetOnLoad: true
	};
	Ext.apply(initialConfig, config);
	OMV.FormPanelExt.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.FormPanelExt, OMV.form.FormPanel, {
	initComponent : function() {
		Ext.apply(this, {
			buttons: this.getButtons(),
			items: this.getFormItems()
		});
		OMV.FormPanelExt.superclass.initComponent.apply(this, arguments);
		// Register event handler
		this.on("render", this.doLoad, this, { delay: 10 });
	},

	/**
	 * Returns the buttons displayed in the property window form.
	 */
	getButtons : function() {
		return [{
			id: this.getId() + "-ok",
			text: _("OK"),
			hidden: this.hideOK,
			handler: this.cbOkBtnHdl,
			scope: this
		},{
			id: this.getId() + "-reset",
			text: _("Reset"),
			hidden: this.hideReset,
			handler: this.cbResetBtnHdl,
			scope: this
		}];
	},

	/**
	 * @method cbOkBtnHdl
	 * Method that is called when the 'OK' button is pressed.
	 */
	cbOkBtnHdl : function() {
		this.doSubmit();
	},

	/**
	 * @method cbResetBtnHdl
	 * Method that is called when the 'Reset' button is pressed.
	 */
	cbResetBtnHdl : function() {
		this.reset();
	},

	/**
	 * Returns the items displayed in the property window form.
	 * This function must be overwritten by every derived class.
	 */
	getFormItems : function() {
		return [];
	},

	/**
	 * Find a Ext.form.Field in this form.
	 * @param id The value to search for (specify either a id, dataIndex,
	 * name or hiddenName).
	 * @returns The searched Ext.form.Field
	 */
	findFormField : function(id) {
		var basicForm = this.getForm();
		return basicForm.findField(id);
	},

	/**
	 * Handler that is called by the RPC initiated by 'doSubmit'.
	 * Reload the panel if the RPC was successful.
	 */
	cbSubmitHdl : function(id, response, error) {
		OMV.FormPanelExt.superclass.cbSubmitHdl.apply(this, arguments);
		if (error === null) {
			this.doReload();
		}
	},

	/**
	 * Set/Unset all form fields read-only.
	 */
	setReadOnly : function(readOnly) {
		this.buttons.each(function(button) {
			  button.setDisabled(readOnly);
		  }, this);
		var basicForm = this.getForm();
		basicForm.items.each(function(item) {
			  if (Ext.isFunction(item.setReadOnly)) {
				  item.setReadOnly(readOnly);
			  }
		  }, this);
	}
});
