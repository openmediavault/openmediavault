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
// require("js/omv/workspace/window/Container.js")
// require("js/omv/tab/Panel.js")

/**
 * @ingroup webgui
 * A workspace window displaying a tab panel.
 * @class OMV.workspace.window.Tab
 * @derived OMV.workspace.window.Container
 */
Ext.define("OMV.workspace.window.Tab", {
	extend: "OMV.workspace.window.Container",
	requires: [
		"OMV.tab.Panel"
	],

	tabClassName: "OMV.tab.Panel",
	tabConfig: {},
	tabItems: [],

	getWindowItems: function() {
		var me = this;
		me.tp = me.initTab();
		return me.tp;
	},

	/**
	 * Initialize the tab panel displayed in this window.
	 * @return The tab panel object.
	 */
	initTab: function() {
		var me = this;
		return Ext.create(me.tabClassName, Ext.apply({
			border: false,
			activeTab: 0,
			layoutOnTabChange: true,
			enableTabScroll: true,
			defaults: {
				readOnly: me.readOnly
			},
			items: me.getTabItems()
		}, me.getTabConfig()));
	},

	/**
	 * Returns additional tab configuration options.
	 * @return The tab panel configuration object.
	 */
	getTabConfig: function() {
		return this.tabConfig;
	},

	/**
	 * Returns the items displayed in the property window form.
	 * This function must be overwritten by every derived class.
	 * @return An array of items displayed in the tab panel.
	 */
	getTabItems: function() {
		return this.tabItems;
	},

	/**
	 * Returns the tab panel.
	 * @return The tab panel object.
	 */
	getTab: function() {
		return this.tp;
	},

	/**
	 * Validate the tab values.
	 * @return Returns TRUE if client-side validation on the tab
	 *   is successful.
	 */
	isValid: function() {
		var me = this;
		var valid = true;
		var tab = me.getTab();
		tab.items.each(function(item) {
			// Clear invalid flag.
			tab.clearInvalidTab(item);
			// Check if there is a validation function and execute it
			// if existing.
			if(Ext.isFunction(item.isValid)) {
				if(!item.isValid()) {
					valid = false;
					// Mark tab panel component as invalid.
					tab.markInvalidTab(item);
				}
			}
		});
		return valid;
	},

	/**
	 * Clears all invalid field messages in this tab and its sub panels.
	 */
	clearInvalid: function() {
		var me = this;
		var tab = me.getTab();
		tab.items.each(function(item) {
			tab.clearInvalidTab(item);
			if(Ext.isFunction(item.clearInvalid))
				item.clearInvalid();
		});
	},

	/**
	 * Mark fields in this tab and its sub panels invalid in bulk.
	 * @param errors Either an array in the tab
	 *   [{id:'fieldId', msg:'The message'},...] or an object hash of
	 *   {id: msg, id2: msg2}
	 * @return The basic form panel.
	 */
	markInvalid: function(errors) {
		var me = this;
		me.getTab().items.each(function(item) {
			if(Ext.isFunction(item.markInvalid))
				item.markInvalid(errors);
		});
	},

	/**
	 * Checks if any fields in this tab have changed from their original
	 * values. If the values have been loaded into the tab then these are
	 * the original ones.
	 * @return Returns TRUE if any fields in this tab have changed from
	 *   their original values.
	 */
	isDirty: function() {
		var me = this;
		var dirty = false;
		me.getTab().items.each(function(item) {
			if(Ext.isFunction(item.isDirty))
				dirty = item.isDirty();
			if(dirty) // Abort immediatelly
				return false;
		});
		return dirty;
	},

	/**
	 * Set values for fields in this tab in bulk.
	 * @param values The values to set in the tab of an object hash.
	 * @return None.
	 */
	setValues: function(values) {
		var me = this;
		me.getTab().items.each(function(item) {
			if(Ext.isFunction(item.setValues))
				item.setValues(values);
		});
	},

	/**
	 * Returns the fields in this tab as an object with key/value pairs.
	 * @return An array of key/value pairs.
	 */
	getValues: function() {
		var me = this;
		var values = {};
		me.getTab().items.each(function(item) {
			if(Ext.isFunction(item.getValues)) {
				var v = item.getValues();
				Ext.applyEx(values, v);
			}
		});
		return values;
	},

	/**
	 * Method that is called when the 'Reset' button is pressed. The reset
	 * will be forwarded to the active tab.
	 * @param this The window itself.
	 */
	onResetButton: function() {
		var me = this;
		var activeTab = me.getTab().getActiveTab();
		if(Ext.isFunction(activeTab.reset))
			activeTab.reset();
	}
});
