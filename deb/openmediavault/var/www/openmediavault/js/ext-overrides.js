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

////////////////////////////////////////////////////////////////////////////////
// Ext.form.VTypes
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.form.VTypes, {
	IPv4: function(v) {
		return /^([1-9][0-9]{0,1}|1[013-9][0-9]|12[0-689]|2[01][0-9]|22[0-3])([.]([1-9]{0,1}[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])){2}[.]([1-9][0-9]{0,1}|1[0-9]{2}|2[0-4][0-9]|25[0-4])$/.test(v);
	},
	IPv4Text: _("This field should be an IPv4 address"),
	IPv4Mask: /[\d\.]/i,

	IPv4Net: function(v) {
		return /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/.test(v);
	},
	IPv4NetText: _("This field should be a IPv4 network address"),
	IPv4NetMask: /[\d\.\/]/i,

	IPv4NetCIDR: function(v) {
		return /^([0-9][0-9]{0,1}|1[013-9][0-9]|12[0-689]|2[01][0-9]|22[0-3])([.]([0-9]{0,1}[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])){2}[.]([0-9][0-9]{0,1}|1[0-9]{2}|2[0-4][0-9]|25[0-4])\/(3[0-2]|[0-2]?[0-9])$/.test(v);
	},
	IPv4NetCIDRText: _("This field should be a IPv4 network address in CIDR notation"),
	IPv4NetCIDRMask: /[\d\.\/]/i,

	IPv4Fw: function(v) {
		ipv4RegEx = "\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}";
		if (v === "0/0")
			return true;
		// 172.16.76.4 or !192.168.178.87/24
		if (RegExp("^(!)?("+ipv4RegEx+")(\/(3[0-2]|[0-2]?\\d))?$", "i").test(v))
			return true;
		// 192.168.178.20-192.168.178.254
		return RegExp("^(!)?(("+ipv4RegEx+")([-]("+ipv4RegEx+")){0,1})$", "i").test(v);
	},
	IPv4FwText: _("This field should be either a IPv4 network address (with /mask), a IPv4 range or a plain IPv4 address (e.g. 172.16.76.4 or !192.168.178.87/24 or 192.168.178.20-192.168.178.254)"),
	IPv4FwMask: /[\d\.\/\-:!]/i,

	netmask: function(v) {
		return /^(128|192|224|24[08]|25[245].0.0.0)|(255.(0|128|192|224|24[08]|25[245]).0.0)|(255.255.(0|128|192|224|24[08]|25[245]).0)|(255.255.255.(0|128|192|224|24[08]|252))$/.test(v);
	},
	netmaskText: _("This field should be a netmask within the range 128.0.0.0 - 255.255.255.252"),
	netmaskMask: /[.0-9]/,

	port: function(v) {
		return /^([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/.test(v);
	},
	portText: _("This field should be a port in the range of 1 - 65535"),
	portMask: /[0-9]/i,

	portFw: function(v) {
		var portRegEx = "[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]";
		return RegExp("^!?(("+portRegEx+")([-:]("+portRegEx+")){0,1})$", "i").test(v);
	},
	portFwText: _("This field should be a port or port range (e.g. 21 or !443 or 1024-65535)"),
	portFwMask: /[0-9\-:!]/i,

	num: function(v) {
		return /^[0-9]+$/.test(v);
	},
	numText: _("This field should only contain numbers"),
	numMask: /[0-9]/i,

	numList: function(v) {
		return /^(\d+[,;])*\d+$/.test(v);
	},
	numListText: _("This field should only contain numbers seperated by <,> or <;>"),
	numListMask: /[\d,;]/i,

	textList: function(v) {
		return /^(\w+[,;])*\w+$/.test(v);
	},
	textListText: _("This field should only contain strings seperated by <,> or <;>"),
	textListMask: /[\w,;]/i,

	portList: function(v) {
		return /^((0|[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])[,;])*(0|[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/.test(v);
	},
	portListText: _("This field should only contain ports (0 - 65535) seperated by <,> or <;>"),
	portListMask: /[\d,;]/i,

	hostname: function(v) {
		return /^[a-zA-Z]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9]){0,1}$/.test(v);
	},
	hostnameText: _("Invalid hostname"),
	hostnameMask: /[a-z0-9\-]/i,

	hostnameIPv4: function(v) {
		if(Ext.form.VTypes.hostname(v))
			return true;
		if(Ext.form.VTypes.IPv4(v))
			return true;
		return false;
	},
	hostnameIPv4Text: _("This field should be a hostname or an IPv4 address"),
	hostnameIPv4Mask: /[a-z0-9\-\.]/i,

	domainname: function(v) {
		return /^[a-zA-Z]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?([.][a-zA-Z]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?)*$/.test(v);
	},
	domainnameText: _("Invalid domain name"),
	domainnameMask: /[a-z0-9\-\.]/i,

	groupname: function(v) {
		return /^[a-zA-Z0-9\-\.]+$/.test(v);
	},
	groupnameText: _("Invalid group name"),
	groupnameMask: /[a-zA-Z0-9\-\.]/,

	username: function(v) {
		// Taken from Debian adduser
		return /^[_.A-Za-z0-9][-\@_.A-Za-z0-9]*\$?$/.test(v);
	},
	usernameText: _("Invalid user name"),
	usernameMask: /[-\@_.A-Za-z0-9]/,

	comment: function(v) {
		return !/[:]/.test(v);
	},
	commentText: _("Invalid comment"),
	commentMask: /[^:]/,

	password: function(v) {
		return !/[^a-zA-Z0-9\.\-_]/.test(v);
	},
	passwordText: _("The password contains invalid characters"),
	passwordMask: /[a-zA-Z0-9\.\-_]/,

	// String that are used as filesystem label
	fslabel: function(v) {
		return /^[a-zA-Z0-9]+$/.test(v);
	},
	fslabelText: _("Invalid filesystem label"),
	fslabelMask: /[a-zA-Z0-9]/,

	sharename: function(v) {
		// We are using the SMB/CIFS file/directory naming convention for this:
		// All characters are legal in the basename and extension except the
		// space character (0x20) and:
		// "./\[]:+|<>=;,*?
		// A share name or server or workstation name SHOULD not begin with a
		// period (“.”) nor should it include two adjacent periods (“..”).
		// References:
		// http://tools.ietf.org/html/draft-leach-cifs-v1-spec-01
		// http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx
		return /^[^.]([^"/\\\[\]:+|<>=;,*?. ]+){0,1}([.][^"/\\\[\]:+|<>=;,*?. ]+){0,}$/.test(v);
	},
	sharenameText: "This field contains invalid characters, e.g. space character or \"/\[]:+|<>=;,*?",
	sharenameMask: /[^"/\\\[\]:+|<>=;,*? ]/,

	// Strings that are used as part of a device name
	devname: function(v) {
		return /^[a-zA-Z0-9\.\-_]+$/.test(v);
	},
	devnameText: _("Invalid name"),
	devnameMask: /[a-zA-Z0-9\.\-_]/
});

////////////////////////////////////////////////////////////////////////////////
// Ext.MessageBox
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.LoadMask.prototype, {
	msg: _("Loading ...")
});

////////////////////////////////////////////////////////////////////////////////
// Ext.MessageBox
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.MessageBox, {
	buttonText: {
		ok: _("OK"),
		cancel: _("Cancel"),
		yes: _("Yes"),
		no: _("No")
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.BasicForm
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.form.BasicForm.prototype, {
	/**
	 * Retrieves the fields in the form as a set of key/value pairs. If
	 * multiple fields exist with the same name they are returned as an array.
	 * Disabled fields and fields where the submitValue option is set to false
	 * are not processed and included in the result array.
     * @return The values in the form.
	 */
	getValuesEx : function() {
		var name, key, values;
		// Use the default ExtJS implementation to get the form field values.
		// This is done to get the same behaviour as a form submit. The
		// resulting array contains the requested form fields except disabled
		// and 'submitValue = false' fields.
		values = this.getValues();
		for (key in values) {
			var field = this.findField(key);
			if (!Ext.isEmpty(field)) {
				switch (field.xtype) {
				case "datefield":
					break;
				default:
					values[key] = field.getValue();
					break;
				}
			}
		}
		// Note, unchecked checkboxes are not included, so it is necessary to
		// loop over all form fields to get them also.
		var func = function(f) {
			if (f.isFormField) {
				if (!f.disabled && f.submitValue && (f.xtype == "checkbox")) {
					key = f.getName();
					value = values[key];
					if (!Ext.isDefined(value)) {
						values[key] = f.getValue();
					}
				} else if (f.isComposite) {
					return f.items.each(func);
				} else if (f instanceof Ext.form.CheckboxGroup && f.rendered) {
					return f.eachItem(func);
				}
			}
		};
		this.items.each(func);
		return values;
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.Field
////////////////////////////////////////////////////////////////////////////////

// Add 'autoComplete' config option.
Ext.apply(Ext.form.Field.prototype, {
	autoComplete: false,

	getAutoCreate : function() {
		var cfg = Ext.form.Field.superclass.getAutoCreate.call(this);
		if(true === this.autoComplete) {
			cfg.autocomplete = "on";
		}
		return cfg;
	}
});

// Add 'setReadOnly' to Ext.form.Field
Ext.override(Ext.form.Field, {
	setReadOnly : function(readOnly) {
		var el = this.getEl();
		var xtype = this.getXType();
		this.readOnly = readOnly;
		if (readOnly) {
			if (el) {
				el.dom.setAttribute('readOnly', true);
			}
			this.addClass(this.disabledClass);
		} else {
			if (el) {
				el.dom.removeAttribute('readOnly');
			}
			this.removeClass(this.disabledClass);
		}
		if ("checkbox" == xtype) {
			this.readOnly = readOnly;
		}
		if (("radiogroup" == xtype) || ("checkboxgroup" == xtype)) {
			var items = this.items.items;
			for (var i = 0; i < items.length; i++) {
				items[i].readOnly = readOnly;
			}
		}
	}
});

// Add tooltips to Ext.form.Field
Ext.sequence(Ext.form.Field.prototype, "afterRender", function() {
	if (this.qtip) {
		Ext.QuickTips.register({
			target:  this,
			title: "",
			text: this.qtip,
			enabled: true,
			showDelay: 0
		});
	}
});

// Show or hide the form field label when show() or hide() is called
Ext.override(Ext.form.Field, {
	onHide : function() {
		var element = this.getEl().up('.x-form-item');
		if (!Ext.isEmpty(element)) {
			element.setDisplayed(false);
		}
	},

	onShow : function() {
		var element = this.getEl().up('.x-form-item');
		if (!Ext.isEmpty(element)) {
			element.setDisplayed(true);
		}
	}
});

// Do not reset a form field that is read-only
Ext.intercept(Ext.form.Field.prototype, "reset", function() {
	if (this.readOnly) {
		return false;
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.TriggerField
////////////////////////////////////////////////////////////////////////////////

// Display trigger field if field is read-only like it is done in previous
// versions (e.g. 3.0.3).
Ext.sequence(Ext.form.TriggerField.prototype, "updateEditState", function() {
	if (this.rendered) {
		this.trigger.setDisplayed(!this.hideTrigger);
		this.onResize(this.width || this.wrap.getWidth());
	}
});

// Grey out read-only trigger fields.
Ext.sequence(Ext.form.TriggerField.prototype, "setReadOnly",
  function(readOnly) {
	Ext.form.TriggerField.superclass.setReadOnly.call(this, readOnly);
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.ComboBox
////////////////////////////////////////////////////////////////////////////////

// Restrict the selected value to one of the values in the list per default.
Ext.apply(Ext.form.ComboBox.prototype, {
	forceSelection: true,
	loadingText: _("Loading ...")
});

// Sometimes the first load of a form loads a value into a ComboBox before
// that ComboBox's store has been populated.
// http://www.extjs.com/forum/showthread.php?p=364333
Ext.override(Ext.form.ComboBox, {
	setValue : function(v) {
		if(!Ext.isObject(this.store))
			return;
		if(this.mode == 'remote' && !Ext.isDefined(this.store.totalLength)){
			// Display the loading text in the meanwhile ...
			this.setRawValue(this.loadingText);
			// Call this funtion again after store has been loaded
			this.store.on('load', this.setValue.createDelegate(this, arguments),
			  null, {single: true});
			if(this.store.lastOptions === null){
				this.store.load();
			}
			// Set value to get trackResetOnLoad working
			this.value = v;
			return;
		}
		var text = v;
		if(this.valueField){
			var r = this.findRecord(this.valueField, v);
			if(r){
				text = r.data[this.displayField];
			}else if(Ext.isDefined(this.valueNotFoundText)){
				text = this.valueNotFoundText;
			}
		}
		this.lastSelectionText = text;
		if(this.hiddenField){
			this.hiddenField.value = v;
		}
		Ext.form.ComboBox.superclass.setValue.call(this, text);
		this.value = v;
		return this;
	}
});

// Encode HTML entities in combobox drop-down list
Ext.intercept(Ext.form.ComboBox.prototype, "initList", function() {
	if (!this.list){
		if (!this.tpl) {
			var cls = 'x-combo-list';
			this.tpl = new Ext.XTemplate(
			  '<tpl for="."><div class="'+cls+'-item">',
			  '{[Ext.util.Format.htmlEncode(values.'+this.displayField+')]}',
			  '</div></tpl>');
		}
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.Radio
////////////////////////////////////////////////////////////////////////////////

// See http://www.sencha.com/forum/showthread.php?77999-Bug-in-Ext.form.Radio-setValue&highlight=radio%20check%20event
Ext.override(Ext.form.Radio, {
	setValue : function(v) {
		if (typeof v == 'boolean') {
			Ext.form.Radio.superclass.setValue.call(this, v);
		} else if (this.rendered) {
			var els = this.getCheckEl().select('input[name=' + this.el.dom.name + ']');
			els.each(function(el){
				if (el.dom.value == v) {
					Ext.getCmp(el.dom.id).setValue(true);
				} else {
					Ext.getCmp(el.dom.id).setValue(false);
				}
			}, this);
		}
		return this;
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.Checkbox
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.form.Checkbox.prototype, {
	msgTarget: "under"
});
Ext.override(Ext.form.Checkbox, {
	markInvalid : function(msg) {
		Ext.form.Checkbox.superclass.markInvalid.apply(this, arguments);
	},

	clearInvalid : function(){
		Ext.form.Checkbox.superclass.clearInvalid.apply(this, arguments);
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.grid.Column
////////////////////////////////////////////////////////////////////////////////

// Encode HTML entities in default column renderer (strings only)
Ext.override(Ext.grid.Column, {
	renderer : function(value) {
		if (Ext.isString(value)) {
			if (value.length < 1) {
				return '&#160;';
			} else {
				return Ext.util.Format.htmlEncode(value);
			}
		}
		return value;
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.data.Store
////////////////////////////////////////////////////////////////////////////////

Ext.override(Ext.data.Store, {
	dirty: false,
	markDirty : function() {
        this.dirty = true;
    },
	isDirty : function() {
		return this.dirty;
	}
});

// Fix modified issue when adding, inserting or removing records from a store.
Ext.sequence(Ext.data.Store.prototype, "insert", function(index, records) {
	this.markDirty();
});
Ext.sequence(Ext.data.Store.prototype, "add", function(records) {
	this.markDirty();
});
Ext.sequence(Ext.data.Store.prototype, "remove", function(records) {
	this.markDirty();
});
Ext.sequence(Ext.data.Store.prototype, "afterEdit", function(record) {
	if (this.modified.length > 0) {
		this.markDirty();
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.TabPanel
////////////////////////////////////////////////////////////////////////////////

Ext.override(Ext.TabPanel, {
	removeTabIconClass : function(tab, cls) {
		var el = this.getTabEl(tab);
		if (el) {
			if (Ext.fly(el).hasClass('x-tab-with-icon')) {
				Ext.fly(el).addClass('x-tab-with-icon').
				  child('span.x-tab-strip-text').removeClass(cls);
				Ext.fly(el).removeClass('x-tab-with-icon');
			}
		}
	},

	setTabIconClass : function(tab, cls) {
		var el = this.getTabEl(tab);
		if (el) {
			Ext.fly(el).addClass('x-tab-with-icon').
			  child('span.x-tab-strip-text').addClass(cls);
		}
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.state.CookieProvider
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.state.CookieProvider.prototype, {
	/**
	 * Clear all states.
	 */
	clearCookies : function() {
		var cookie = document.cookie + ";";
		var re = /\s?(.*?)=(.*?);/g;
		var matches;
		while (null != (matches = re.exec(cookie))) {
			var name = matches[1];
			if (name && name.substring(0,3) == "ys-") {
				this.clear(name.substr(3));
			}
		}
	}
});

////////////////////////////////////////////////////////////////////////////////
// Additional helper functions
////////////////////////////////////////////////////////////////////////////////

Ext.applyIf(Ext, {
	/**
	 * Copies recursively all the properties of config to obj.
	 * @param o The receiver of the properties
	 * @param c The source of the properties
	 * @param defaults A different object that will also be applied for default values
	 * @return Returns the receiver object.
	 */
	applyEx : function(o, c, defaults) {
		// no "this" reference for friendly out of scope calls
		if (defaults) {
			Ext.apply(o, defaults);
		}
		if (o && c && typeof c == 'object') {
			for (var p in c) {
				if ((typeof c[p] == 'object') && (typeof o[p] !== 'undefined')) {
					Ext.applyEx(o[p], c[p]);
				} else {
					o[p] = c[p];
				}
			}
		}
		return o;
	},

	clone : function(o) {
		var c = (o instanceof Array) ? [] : {};
		for (i in o) {
			if (o[i] && typeof o[i] == "object") {
				c[i] = Ext.clone(o[i]);
			} else c[i] = o[i]
		}
		return c;
	},

	/**
	 * Finds out whether a variable is an UUID v4.
	 * @param v The variable being evaluated.
	 * @return TRUE if variable is a UUID, otherwise FALSE.
	 */
	isUUID: function(v) {
		if (!Ext.isString(v))
			return false;
		return /^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}$/i.test(v);
	}
});
