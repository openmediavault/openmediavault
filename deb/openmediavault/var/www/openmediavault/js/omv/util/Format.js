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

Ext.ns("OMV.util");

/**
 * Reusable data formating functions.
 * @ingroup webgui
 */
OMV.util.Format = function() {
	var f = function() {}
	f.prototype = Ext.util.Format;
	var o = function() {}
	Ext.extend(o, f, function() {
		return {
			/**
			 * @method boolean
			 * Format a boolean value in a string representation.
			 * @param value The boolean value to format.
			 * @return The formatted boolean string.
			 */
			boolean: function(value) {
				var trueValue = [ true, 1, "true", "ok", "1", "y",
					"yes", "on" ];
				if (Ext.isString(value))
					value = value.toLowerCase();
				return Ext.Array.contains(trueValue, value) ?
					_("Yes") : _("No");
			},

			/**
			 * @method arrayRenderer
			 * Returns an array rendering function
			 * @param data The array containing the key/value pairs used to
			 *   map the value to be rendered
			 * @return The rendering function.
			 */
			arrayRenderer: function(data) {
				return function(value) {
					for(var i = 0; i < data.length; i++) {
						var d = data[i];
						if(d[0] === value) {
							return d[1];
						}
					}
					return value;
				};
			},

			/**
			 * @method localeTime
			 * Convert the given UNIX epoch timestamp to a string, using
			 * locale conventions.
			 * @param value The date as UNIX epoch timestamp.
			 * @return The formatted date string.
			 */
			localeTime: function(value) {
				var dt = Ext.Date.parse(value, "U");
				return dt.toLocaleString();
			},

			/**
			 * @method localeTimeRenderer
			 * Returns a rendering function that displayes the given
			 * UNIX epoch timestamp in human readable form using the
			 * local time format.
			 * @return The rendering function.
			 */
			localeTimeRenderer: function() {
				return function(value) {
					return OMV.util.Format.localeTime(value);
				};
			},

			/**
			 * @method whitespace
			 * Format the given value using the defined white-space mode.
			 * @param value The value to format.
			 * @param mode The mode to use. This can be: normal, nowrap, pre,
			 *   pre-line, pre-wrap or inherit. Defaults to 'normal'.
			 * @return The HTML code to display the given text using the
			 *   given white-space mode.
			 */
			whitespace: function(value, mode) {
				mode = mode || "normal";
				return Ext.String.format('<span style="white-space:{0} ' +
				  '!important;">{1}</span>', mode,
				  Ext.String.htmlEncode(value));
			},

			/**
			 * @method binaryUnit
			 * Convert a value into the highest possible binary unit.
			 * @param value The value to format.
			 * @param invalidText The value that is returned if \em value
			 *   is not a number. Defaults to 'n/a'.
			 */
			binaryUnit: function(value, invalidText) {
				invalidText = Ext.isDefined(invalidText) ?
				  invalidText : _("n/a");
				var v = parseInt(value);
				if (!Ext.isNumber(v) || (0 > v))
					return invalidText;
				return v.binaryFormat();
			},

			/**
			 * @method progressBarRenderer
			 * Returns a rendering function that displays a progress bar.
			 * @param percentage A floating point value between 0 and 1.
			 * @param text The text shown in the progress bar.
			 * @param warningThreshold A floating point value between 0 and 1.
			 *   When the \em percentage value is above this value, the
			 *   progress bar will be rendered in a warning color.
			 * @return The rendering function.
			 */
			progressBarRenderer: function(percentage, text, warningThreshold) {
				return function(value, metaData, record, rowIndex, colIndex,
				  store, view) {
					var id = Ext.id();
					metaData.align = "center";
					var fn = function() {
						// Make sure the element already exists. If not,
						// then trigger this function delayed again.
						if (null == Ext.get(id)) {
							Ext.Function.defer(fn, 50);
							return;
						}
						var cls = "";
						if (Ext.isDefined(warningThreshold)) {
							if (percentage >= warningThreshold)
								cls = Ext.baseCSSPrefix + "progress-warning";
						}
						Ext.widget("progressbar", {
							renderTo: id,
							cls: cls,
							value: percentage,
							text: text,
							listeners: {
								afterrender: function(c) {
									// This is a workaround because the
									// text is only set if the progress bar
									// component is rendered.
									if (null != text)
										c.updateText(text);
								}
							}
						});
					};
					fn.apply(this);
					// Return the HTML code where the progress bar will
					// be rendered to.
					return Ext.String.format("<div id=\"{0}\"></div>", id);
				}
			}
		};
	}());
	return new o();
}();
