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

/**
 * Convert special characters to HTML entities.
 */
String.prototype.htmlspecialchars = function() {
	var str = this;
	str = str.replace(/&/g, "&amp;");
	str = str.replace(/"/g, "&quot;");
	str = str.replace(/'/g, "&#039;");
	str = str.replace(/</g, "&lt;");
	str = str.replace(/>/g, "&gt;");
	return str;
}

Ext.applyIf(Array, {
	/**
	 * @function range
	 * Create an array containing a range of elements.
	 * @param low Low value.
	 * @param high High value.
	 * @param step If a step value is given, it will be used as the increment
	 * between elements in the sequence. It should be given as a positive number.
	 * If not specified, step will default to 1.
	 * @param asString Convert the elements to strings. Defaults to FALSE.
	 * @return Returns an array of elements from low  to high , inclusive.
	 * If low > high, the sequence will be from high to low.
	 */
	range : function(low, high, step, asString) {
		var array = [];
		var startv = low;
		var endv = high;
		var stepv = step || 1;
		asString = asString || false;

		if (startv < endv) {
			while (startv <= endv) {
				array.push(((asString) ? startv.toString() : startv));
				startv += stepv;
			}
		} else {
			while (startv >= endv) {
				array.push(((asString) ? startv.toString() : startv));
				startv -= stepv;
			}
		}

		return array;
	}
});

Ext.applyIf(Array.prototype, {
	/**
	 * @function each
	 * Apply a function to each element.
	 * @param fn The function to apply.
	 * @param scope (optional) The scope in which the function is executed.
	 * @return None
	 */
	each : function(fn, scope) {
		for (var i = 0; i < this.length; i++) {
			if (fn.call(scope || this, this[i], i) === false) {
				break;
			}
		}
	},

	/**
	 * @function walk
	 * Change each value according to a callback function.
	 * @param fn The function to apply.
	 * @return Returns the modified array.
	 */
	walk : function(fn) {
		var a = [], i = this.length;
		while (i--) {
			a.push(fn(this[i]));
		}
		return a.reverse();
	},

	/**
	 * @function insert
	 * Insert value at index, without overwriting existing keys.
	 * @param index The index where to insert the value.
	 * @param value The value to insert into the array.
	 * @return Returns the modified array.
	 */
	insert : function(index, value) {
		this.splice(index,0,value);
		return this;
	},

	/**
	 * @function replace
	 * Replace value at index.
	 * @param index The index of the value to replace.
	 * @param value The new value.
	 * @return Returns the modified array.
	 */
	replace : function(index, value) {
		this.splice(index,1,value);
		return this;
	}
});

Ext.applyIf(Date, {
	mapHour : [
		[ "*","*" ],
		[ "0","0" ],
		[ "1","1" ],
		[ "2","2" ],
		[ "3","3" ],
		[ "4","4" ],
		[ "5","5" ],
		[ "6","6" ],
		[ "7","7" ],
		[ "8","8" ],
		[ "9","9" ],
		[ "10","10" ],
		[ "11","11" ],
		[ "12","12" ],
		[ "13","13" ],
		[ "14","14" ],
		[ "15","15" ],
		[ "16","16" ],
		[ "17","17" ],
		[ "18","18" ],
		[ "19","19" ],
		[ "20","20" ],
		[ "21","21" ],
		[ "22","22" ],
		[ "23","23" ]
	],

	mapHour2Digits : [
		[ "*","*" ],
		[ "00","0" ],
		[ "01","1" ],
		[ "02","2" ],
		[ "03","3" ],
		[ "04","4" ],
		[ "05","5" ],
		[ "06","6" ],
		[ "07","7" ],
		[ "08","8" ],
		[ "09","9" ],
		[ "10","10" ],
		[ "11","11" ],
		[ "12","12" ],
		[ "13","13" ],
		[ "14","14" ],
		[ "15","15" ],
		[ "16","16" ],
		[ "17","17" ],
		[ "18","18" ],
		[ "19","19" ],
		[ "20","20" ],
		[ "21","21" ],
		[ "22","22" ],
		[ "23","23" ]
	],

	mapDayOfWeek : [
		[ "*","*" ],
		[ "1","Monday" ],
		[ "2","Tuesday" ],
		[ "3","Wednesday" ],
		[ "4","Thursday" ],
		[ "5","Friday" ],
		[ "6","Saturday" ],
		[ "7","Sunday" ]
	],

	mapDayOfMonth : [
		[ "*", "*" ],
		[ "1","1" ],
		[ "2","2" ],
		[ "3","3" ],
		[ "4","4" ],
		[ "5","5" ],
		[ "6","6" ],
		[ "7","7" ],
		[ "8","8" ],
		[ "9","9" ],
		[ "10","10" ],
		[ "11","11" ],
		[ "12","12" ],
		[ "13","13" ],
		[ "14","14" ],
		[ "15","15" ],
		[ "16","16" ],
		[ "17","17" ],
		[ "18","18" ],
		[ "19","19" ],
		[ "20","20" ],
		[ "21","21" ],
		[ "22","22" ],
		[ "23","23" ],
		[ "24","24" ],
		[ "25","25" ],
		[ "26","26" ],
		[ "27","27" ],
		[ "28","28" ],
		[ "29","29" ],
		[ "30","30" ],
		[ "31","31" ]
	],

	mapDayOfMonth2Digits : [
		[ "*", "*" ],
		[ "01","1" ],
		[ "02","2" ],
		[ "03","3" ],
		[ "04","4" ],
		[ "05","5" ],
		[ "06","6" ],
		[ "07","7" ],
		[ "08","8" ],
		[ "09","9" ],
		[ "10","10" ],
		[ "11","11" ],
		[ "12","12" ],
		[ "13","13" ],
		[ "14","14" ],
		[ "15","15" ],
		[ "16","16" ],
		[ "17","17" ],
		[ "18","18" ],
		[ "19","19" ],
		[ "20","20" ],
		[ "21","21" ],
		[ "22","22" ],
		[ "23","23" ],
		[ "24","24" ],
		[ "25","25" ],
		[ "26","26" ],
		[ "27","27" ],
		[ "28","28" ],
		[ "29","29" ],
		[ "30","30" ],
		[ "31","31" ]
	],

	mapMonth : [
		[ "*","*" ],
		[ "1","January" ],
		[ "2","February" ],
		[ "3","March" ],
		[ "4","April" ],
		[ "5","May" ],
		[ "6","June" ],
		[ "7","July" ],
		[ "8","August" ],
		[ "9","September" ],
		[ "10","October" ],
		[ "11","November" ],
		[ "12","December" ]
	],

	mapMonth2Digits : [
		[ "*","*" ],
		[ "01","January" ],
		[ "02","February" ],
		[ "03","March" ],
		[ "04","April" ],
		[ "05","May" ],
		[ "06","June" ],
		[ "07","July" ],
		[ "08","August" ],
		[ "09","September" ],
		[ "10","October" ],
		[ "11","November" ],
		[ "12","December" ]
	]
});

Ext.applyIf(Number.prototype, {
	/**
	 * Convert a number to bytes using binary multiples.
	 * @param fromPrefix The binary prefix name \em number is in, e.g. 'KiB'.
	 * @param toPrefix The binary prefix name to convert \em number to, e.g.
	 * 'TiB'.
	 * @return The converted number.
	 */
	binaryConvert : function(fromPrefix, toPrefix) {
		var prefixes = {
			"B": 0,
			"KiB": 10,
			"MiB": 20,
			"GiB": 30,
			"TiB": 40,
			"PiB": 50,
			"EiB": 60,
			"ZiB": 70,
			"YiB": 80
		};
		return this.toFixed(0) * Math.pow(2, prefixes[fromPrefix] -
		  prefixes[toPrefix]);
	}
});
