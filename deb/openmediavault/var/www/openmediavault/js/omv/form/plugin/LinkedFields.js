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

/**
 * @ingroup webgui
 * @class OMV.form.plugin.LinkedFields
 * @derived Ext.AbstractPlugin
 * Using this plugin it is possible to link various form fields together,
 * thus you can apply properties to a field if others have been modified,
 * e.g. checking a checkbox may show/hide a text field.
 * @param correlations The correlations between the fields. This
 *   configuration contains the following properties:
 *   name - The name of the field.
 *   conditions - The conditions that must be fulfilled to apply the
 *     given properties.
 *   properties - The properties that will be applied if the conditions
 *     are fulfilled. If the conditions do not apply the opposite of the
 *    given properties are set.
 * @param applyPropertiesOnInit Set to TRUE to apply the properties to
 *   the configured fields when the form is initialized. Defaults to TRUE.
 */
Ext.define("OMV.form.plugin.LinkedFields", {
	extend: "Ext.AbstractPlugin",
	alias: "plugin.linkedfields",

	fp: null,
	correlations: [],
	applyPropertiesOnInit: true,

	constructor: function(config) {
		var me = this;
		var correlations = [];
		Ext.Array.each(config.correlations, function(correlation) {
			if(!Ext.isArray(correlation.name))
				correlation.name = [ correlation.name ];
			Ext.Array.each(correlation.name, function(name) {
				var clone = Ext.clone(correlation);
				clone.name = name;
				if(!Ext.isFunction(clone.properties)) {
					if(!Ext.isArray(clone.properties))
						clone.properties = [ clone.properties ];
				}
				correlations.push(clone);
			});
		});
		config.correlations = correlations;
		me.callParent([ config ]);
	},

	init: function(c) {
		var me = this;
		me.fp = c;
		// Register listeners for each field to monitor and react on
		// changes to their values.
		Ext.Array.each(me.correlations, function(correlation) {
			Ext.Array.each(correlation.conditions, function(condition) {
				var field = me.findField(condition.name);
				if(!field)
					return;
				field.on("change", me.onChange, me);
			});
			// Apply the properties.
			if(me.applyPropertiesOnInit) {
				me.processCorrelation(correlation);
			}
		});
	},

	onChange: function(c) {
		var me = this;
		var name = c.getName();
		Ext.Array.each(me.correlations, function(correlation) {
			// First check if the fields are bonded together.
			var bonded = false;
			Ext.Array.each(correlation.conditions, function(condition) {
				// Process the event if the name of the changed field is
				// listed in the properties of the current processed
				// condition. Alternatively process the event if no name
				// but a function is defined.
				if((condition.name === name) || (!Ext.isDefined(
				  condition.name) && Ext.isFunction(condition.func))) {
					bonded = true;
					return false;
				}
			});
			// Skip this correlation if the field that has triggered the
			// event is not bonded with the currently processed field.
			if(!bonded)
				return;
			me.processCorrelation(correlation);
		});
	},

	/**
	 * Helper method to process the correlation conditions and apply the
	 * configured properties.
	 * @param correlation The correlation to be processed.
	 */
	processCorrelation: function(correlation) {
		var me = this;
		var valid = true;
		// Calculate the condition result.
		Ext.Array.each(correlation.conditions, function(condition) {
			// Compare the field value with the condition value or execute
			// the given compare function.
			if(Ext.isDefined(condition.func) && Ext.isFunction(
			  condition.func)) {
				var values = me.getForm().getValues();
				// Execute the compare function. The values of the form
				// are passed to the function, the scope is set to the
				// form itself. The compare function must return TRUE
				// or FALSE.
				valid = Ext.callback(condition.func, me.getForm(), [ values ]);
			} else {
				var field = me.findField(condition.name);
				if(!field)
					return;
				var value = field.getValue();
				if(!Ext.isArray(condition.value)) {
					var operator = condition.op || "===";
					switch(operator) {
					case "===":
					case "eq":
						valid = (value === condition.value);
						break;
					case "!==":
					case "ne":
						valid = (value !== condition.value);
						break;
					case "==":
						valid = (value == condition.value);
						break;
					case "!=":
						valid = (value != condition.value);
						break;
					case "<":
					case "lt":
						valid = (value < condition.value);
						break;
					case "<=":
					case "le":
						valid = (value <= condition.value);
						break;
					case ">":
					case "gt":
						valid = (value > condition.value);
						break;
					case ">=":
					case "ge":
						valid = (value >= condition.value);
						break;
					case "z": // String is null, that is, has zero length.
						valid = Ext.isEmpty(value)
						break;
					case "n": // String is not null.
						valid = !Ext.isEmpty(value)
						break;
					}
				} else {
					valid = Ext.Array.contains(condition.value, value);
				}
			}
			// Exit immediatelly if the values do not match.
			if(!valid)
				return false;
		});
		// Get the field control.
		var field = me.findField(correlation.name);
		if(!field)
			return;
		// Apply the configured properties.
		if(Ext.isFunction(correlation.properties)) {
			// Execute the properties function. The condition result and the
			// form field are passed to the function.
			Ext.callback(correlation.properties, me.getForm(),
			  [ valid, field ]);
		} else {
			Ext.Array.each(correlation.properties, function(property) {
				switch(property) {
				case "submitValue":
					if(Ext.isDefined(field.submitValue)) {
						field.submitValue = valid;
					}
					break;
				case "!submitValue":
				case "notSubmitValue":
					if(Ext.isDefined(field.submitValue)) {
						field.submitValue = !valid;
					}
					break;
				case "allowBlank":
					if(Ext.isDefined(field.allowBlank)) {
						field.allowBlank = valid;
					}
					break;
				case "!allowBlank":
				case "notAllowBlank":
					if(Ext.isDefined(field.allowBlank)) {
						field.allowBlank = !valid;
					}
					break;
				case "readOnly":
					if(Ext.isFunction(field.setReadOnly)) {
						field.setReadOnly(valid);
					}
					break;
				case "!readOnly":
				case "notReadOnly":
					if(Ext.isFunction(field.setReadOnly)) {
						field.setReadOnly(!valid);
					}
					break;
				case "disabled":
				case "!enabled":
				case "notEnabled":
					if(Ext.isFunction(field.setDisabled)) {
						field.setDisabled(valid);
					}
					break;
				case "enabled":
				case "!disabled":
				case "notDisabled":
					if(Ext.isFunction(field.setDisabled)) {
						field.setDisabled(!valid);
					}
					break;
				case "show":
				case "!hide":
				case "notHide":
					if(valid)
						field.show();
					else
						field.hide();
					break;
				case "hide":
				case "!show":
				case "notShow":
					if(valid)
						field.hide();
					else
						field.show();
					break;
				case "visible":
					if(Ext.isFunction(field.setVisible)) {
						field.setVisible(valid);
					}
					break;
				case "!visible":
				case "notVisible":
					if(Ext.isFunction(field.setVisible)) {
						field.setVisible(!valid);
					}
					break;
				}
			});
		}
		if(Ext.isFunction(field.clearInvalid)) {
			field.clearInvalid();
		}
	},

	/**
	 * Helper method to get the form panel associated to this plugin.
	 */
	getForm: function() {
		return this.fp;
	},

	/**
	 * Helper method to get a field by name or id.
	 * @param id The value to search for (specify either a id or name).
	 * @return The first matching field, or null if none was found.
	 */
	findField: function(id) {
		var me = this;
		var selector = Ext.String.format("[name={0}], #{0}", id);
		var field = me.getForm().query(selector)[0];
		// Check if the field is a compomnent. Note, do not use 'isFormField',
		// otherwise it is not possible to query for Ext.button.Button
		// components for example.
		if(!Ext.isObject(field) || !field.isComponent)
			return null;
		return field;
	}
});
