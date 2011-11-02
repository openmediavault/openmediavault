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
// require("js/omv/Window.js")
// require("js/omv/MessageBox.js")
// require("js/omv/data/Connection.js")

Ext.ns("OMV");

/**
 * @class OMV.CfgObjectTabDialog
 * @derived OMV.Window
 * @param uuid The UUID of the object to add/edit. Set to -1 for a new object.
 * @param rpcService The RPC service name.
 * @param rpcGetMethod The RPC method to get request the data.
 * @param rpcSetMethod The RPC method to commit the data.
 * @param success The function that should be called in case of a successful
 * data commit.
 * @param scope The scope to be used when callig the success function.
 * @param hideOk True to hide the 'OK' button. Defaults to false.
 * @param hideCancel True to hide the 'Cancel' button. Defaults to false.
 * @param hideClose True to hide the 'Close' button. Defaults to true.
 * @param hideReset True to hide the 'Reset' button. Defaults to true.
 * @param mode The mode how to retrieve the data displayed in the property
 * dialog. This can be 'local' or 'remote' which means the data is requested
 * via RPC. Defaults to 'remote'.
 */
OMV.CfgObjectTabDialog = function(config) {
	var initialConfig = {
		width: 400,
		height: 200,
		layout: "fit",
		modal: true,
		border: false,
		mode: "remote",
		hideOk: false, // Hide the 'Ok' button
		hideReset: false, // Hide the 'Reset' button
		hideCancel: false, // Hide the 'Cancel' button
		hideClose: true, // Hide the 'Close' button
		buttonAlign: "center"
	};
	Ext.apply(initialConfig, config);
	OMV.CfgObjectTabDialog.superclass.constructor.call(this, initialConfig);
	this.addEvents(
		/**
		 * Fires after the form content has been loaded successful.
		 */
		"load",
		/**
		 * Fires after the submission has been finished successful.
		 */
		"submit"
	);
};
Ext.extend(OMV.CfgObjectTabDialog, OMV.Window, {
	initComponent : function() {
		this.tab = this.initTab();
		Ext.apply(this, {
			buttons: [{
				id: this.getId() + "-ok",
				text: "OK",
				hidden: this.hideOk,
				handler: this.cbOkBtnHdl,
				scope: this
			},{
				id: this.getId() + "-reset",
				text: "Reset",
				hidden: this.hideReset,
				handler: this.cbResetBtnHdl,
				scope: this
			},{
				id: this.getId() + "-cancel",
				text: "Cancel",
				hidden: this.hideCancel,
				handler: this.cbCancelBtnHdl,
				scope: this
			},{
				id: this.getId() + "-close",
				text: "Close",
				hidden: this.hideClose,
				handler: this.cbCloseBtnHdl,
				scope: this
			}],
			items: this.tab
		});
		OMV.CfgObjectTabDialog.superclass.initComponent.apply(this, arguments);
		if ((this.mode === "remote") && (this.uuid !== OMV.UUID_UNDEFINED)) {
			this.on("render", this.doLoad, this, { delay: 10 });
		}
	},

	/**
	 * Initialize the property window tab panel.
	 */
	initTab : function() {
		var config = Ext.apply({
			activeTab: 0,
			layoutOnTabChange: true,
			enableTabScroll: true
		}, this.getTabConfig());
		config.items = this.getTabItems();
		return new Ext.TabPanel(config);
	},

	/**
	 * Returns additional tab configuration options.
	 */
	getTabConfig : function() {
		return {};
	},

	/**
	 * Returns the items displayed in the property window tab.
	 * This function must be overwritten by every derived class.
	 */
	getTabItems : function() {
		return [];
	},

	/**
	 * Validate the tab values.
	 * @return Returns true if client-side validation on the form is successful.
	 */
	isValid : function() {
		var valid = true;
		this.tab.items.each(function(item) {
			// Clear invalid flag
			this.clearInvalid(item);
			// Check if there is a validation function and execute it
			// if existing
			if (Ext.isFunction(item.isValid)) {
				if (!item.isValid()) {
					valid = false;
					// Mark tab panel component as invalid
					this.markInvalid(item);
				}
			}
		}, this);
		return valid;
	},

	/**
	 * Checks if any field in one of the tabs have changed from their
	 * original values.
	 * @return Returns true if any field of the tabs have changed from
	 * their original values.
	 */
	isDirty : function() {
		var dirty = false;
		this.tab.items.each(function(item) {
			if (Ext.isFunction(item.isDirty)) {
				dirty = item.isDirty();
			}
			if (dirty) { // Abort immediatelly
				return false;
			}
		}, this);
		return dirty;
	},

	/**
	 * Set values for fields of all tabs in bulk.
	 * @param values The values to set in the tabs of an object hash.
	 */
	setValues : function(values) {
		this.tab.items.each(function(item) {
			if (Ext.isFunction(item.setValues)) {
				item.setValues(values);
			}
		}, this);
	},

	/**
	 * Returns the fields of all tabs as an object with key/value pairs.
	 */
	getValues : function() {
		var values = {};
		this.tab.items.each(function(item) {
			if (Ext.isFunction(item.getValues)) {
				var v = item.getValues();
				Ext.applyEx(values, v);
			}
		}, this);
		return values;
	},

	doLoad : function() {
		// Display waiting dialog
		OMV.MessageBox.wait(null, "Loading ...");
		// Execute RPC
		OMV.Ajax.request(this.cbLoadHdl, this, this.rpcService,
		  this.rpcGetMethod, [ this.uuid ]);
	},

	cbLoadHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			this.setValues(response);
			this.fireEvent("load", this);
		} else {
			OMV.MessageBox.error(null, error);
		}
	},

	doSubmit : function() {
		var values = this.getValues();
		Ext.apply(values, {
			uuid: this.uuid
		});
		if (this.mode === "remote") {
			// Display waiting dialog
			OMV.MessageBox.wait(null, "Saving ...");
			// Execute RPC
			OMV.Ajax.request(this.cbSubmitHdl, this, this.rpcService,
			  this.rpcSetMethod, [ values ]);
		} else {
			this.fireEvent("submit", this, values);
			this.close();
		}
	},

	cbSubmitHdl : function(id, response, error) {
		OMV.MessageBox.updateProgress(1);
		OMV.MessageBox.hide();
		if (error === null) {
			var values = this.getValues();
			Ext.apply(values, {
				uuid: this.uuid
			});
			this.fireEvent("submit", this, values);
			this.close();
		} else {
			OMV.MessageBox.error(null, error);
		}
	},

	/**
	 * @method cbOkBtnHdl
	 * Method that is called when the 'OK' button is pressed.
	 */
	cbOkBtnHdl : function() {
		// Quit immediatelly if the property values have not been modified and
		// the object is processed object is not new.
		if ((this.uuid !== OMV.UUID_UNDEFINED) && (!this.isDirty())) {
			this.close();
			return;
		}
		// Validate values
		if (!this.isValid()) {
			// Do not close the property dialog. The invalid fields are marked
			// automatically.
		} else {
			this.doSubmit();
		}
	},

	/**
	 * @method cbCancelBtnHdl
	 * Method that is called when the 'Cancel' button is pressed.
	 */
	cbCancelBtnHdl : function() {
		this.close();
	},

	/**
	 * @method cbCloseBtnHdl
	 * Method that is called when the 'Close' button is pressed.
	 */
	cbCloseBtnHdl : function() {
		this.close();
	},

	/**
	 * @method cbResetBtnHdl
	 * Method that is called when the 'Reset' button is pressed.
	 */
	cbResetBtnHdl : function() {
		this.tab.items.each(function(item) {
			if (Ext.isFunction(item.reset)) {
				item.reset();
			}
		}, this);
	},

	/**
	 * @method clearInvalid
	 * Clear any invalid styles/messages for the given tab panel component.
	 * @param item The tab panel where to clear the invalid flag.
	 * @param bubble If set to TRUE all tab panels above the start panel
	 * will be processed too.
	 */
	clearInvalid : function(item, bubble) {
		bubble = Ext.isDefined(bubble) ? bubble : true;
		item.bubble(function(c) {
			if (c.isXType("tabpanel")) {
				c.removeTabIconClass(item, "x-tab-strip-invalid-icon");
				item = c;
				// Exit if bubble is not enabled
				if (!bubble) {
					return false;
				}
			}
		});
	},

	/**
	 * @method markInvalid
	 * Mark the given tab panel component as invalid.
	 * @param item The tab panel to mark invalid,
	 * @param bubble If set to TRUE all tab panels above the start panel
	 * will be processed too.
	 */
	markInvalid : function(item, bubble) {
		bubble = Ext.isDefined(bubble) ? bubble : true;
		item.bubble(function(c) {
			if (c.isXType("tabpanel")) {
				c.setTabIconClass(item, "x-tab-strip-invalid-icon");
				item = c;
				// Exit if bubble is not enabled
				if (!bubble) {
					return false;
				}
			}
		});
	}
});
