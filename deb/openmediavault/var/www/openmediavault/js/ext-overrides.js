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

////////////////////////////////////////////////////////////////////////////////
// Ext.form.field.VTypes
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.form.field.VTypes, {
	ReIPv4Seg: "(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])",
	ReIPv6Seg: "[0-9a-f]{1,4}"
});
Ext.apply(Ext.form.field.VTypes, {
	// IPv4 and IPv6 regex based on http://stackoverflow.com/questions/53497/regular-expression-that-matches-valid-ipv6-addresses
	ReIPv4Addr: Ext.String.format("({0}\.){3,3}{1}",
	  Ext.form.field.VTypes.ReIPv4Seg,
	  Ext.form.field.VTypes.ReIPv4Seg),
	ReIPv6Addr: Ext.String.format("("+
	  "({0}:){7,7}{1}|" +                    // 1:2:3:4:5:6:7:8
	  "({2}:){1,7}:|" +                      // 1::                                 1:2:3:4:5:6:7::
	  "({3}:){1,6}:{4}|" +                   // 1::8               1:2:3:4:5:6::8   1:2:3:4:5:6::8
	  "({5}:){1,5}(:{6}){1,2}|" +            // 1::7:8             1:2:3:4:5::7:8   1:2:3:4:5::8
	  "({7}:){1,4}(:{8}){1,3}|" +            // 1::6:7:8           1:2:3:4::6:7:8   1:2:3:4::8
	  "({9}:){1,3}(:{10}){1,4}|" +           // 1::5:6:7:8         1:2:3::5:6:7:8   1:2:3::8
	  "({11}:){1,2}(:{12}){1,5}|" +          // 1::4:5:6:7:8       1:2::4:5:6:7:8   1:2::8
	  "{13}:((:{14}){1,6})|" +               // 1::3:4:5:6:7:8     1::3:4:5:6:7:8   1::8
	  ":((:{15}){1,7}|:)|" +                 // ::2:3:4:5:6:7:8    ::2:3:4:5:6:7:8  ::8       ::       
	  "fe80:(:{16}){0,4}%[0-9a-zA-Z]{1,}|" + // fe80::7:8%eth0     fe80::7:8%1  (link-local IPv6 addresses with zone index)
	  "::(ffff(:0{1,4}){0,1}:){0,1}{17}|" +  // ::255.255.255.255  ::ffff:255.255.255.255  ::ffff:0:255.255.255.255 (IPv4-mapped IPv6 addresses and IPv4-translated addresses)
	  "({18}:){1,4}:{19}" +                  // 2001:db8:3:4::192.0.2.33  64:ff9b::192.0.2.33 (IPv4-Embedded IPv6 Address)
	  ")",
	  Ext.form.field.VTypes.ReIPv6Seg, Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg, Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg, Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg, Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg, Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg, Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg, Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv6Seg,
	  Ext.form.field.VTypes.ReIPv4Seg,
	  Ext.form.field.VTypes.ReIPv6Seg, Ext.form.field.VTypes.ReIPv4Seg)
});
Ext.apply(Ext.form.field.VTypes, {
	IP: function(v) {
		if(Ext.form.field.VTypes.IPv4(v) || Ext.form.field.VTypes.IPv6(v))
			return true;
		return false;
	},
	IPText: _("This field should be an IP address"),
	IPMask: /[0-9a-f\.:]/i,

	IPNetCIDR: function(v) {
		if(Ext.form.field.VTypes.IPv4NetCIDR(v) ||
		  Ext.form.field.VTypes.IPv6NetCIDR(v))
			return true;
		return false;
	},
	IPNetCIDRText: _("This field should be a IP network address in CIDR notation"),
	IPNetCIDRMask: /[0-9a-f:\.\/]/i,

	IPv4: function(v) {
		var re =  Ext.String.format("^{0}$", Ext.form.field.VTypes.ReIPv4Addr);
		return new RegExp(re, "i").test(v);
	},
	IPv4Text: _("This field should be an IPv4 address"),
	IPv4Mask: /[\d\.]/i,

	IPv4List:  function(v) {
		var valid = true;
		// Split string into several IPv4 addresses.
		Ext.each(v.split(/[,;]/), function(ip) {
			// Is it a valid IPv4 address?
			if(!Ext.form.field.VTypes.IPv4(ip)) {
				valid = false;
				return false;
			}
		});
		return valid;
	},
	IPv4ListText: "This field should be a list of IPv4 addresses",
	IPv4ListMask: /[\d\.,;]/i,

	IPv4Net: function(v) {
		return Ext.form.field.VTypes.IPv4(v);
	},
	IPv4NetText: _("This field should be a IPv4 network address"),
	IPv4NetMask: /[\d\.\/]/i,

	IPv4NetCIDR: function(v) {
		var re =  Ext.String.format("^({0}\/(3[0-2]|[0-2]?[0-9]))$",
		  Ext.form.field.VTypes.ReIPv4Addr);
		return new RegExp(re, "i").test(v);
	},
	IPv4NetCIDRText: _("This field should be a IPv4 network address in CIDR notation"),
	IPv4NetCIDRMask: /[\d\.\/]/i,

	IPv4Fw: function(v) {
		if(v === "0/0")
			return true;
		// 172.16.76.4 or !192.168.178.87/24
		var re = Ext.String.format("^(!)?({0})(\/(3[0-2]|[0-2]?\\d))?$",
		  Ext.form.field.VTypes.ReIPv4Addr);
		if(RegExp(re, "i").test(v))
			return true;
		// 192.168.178.20-192.168.178.254
		re = Ext.String.format("^(!)?(({0})([-]({1})){0,1})$",
		  Ext.form.field.VTypes.ReIPv4Addr, Ext.form.field.VTypes.ReIPv4Addr);
		return RegExp(re, "i").test(v);
	},
	IPv4FwText: _("This field should be either a IPv4 network address (with /mask), a IPv4 range or a plain IPv4 address (e.g. 172.16.76.4 or !192.168.178.87/24 or 192.168.178.20-192.168.178.254)"),
	IPv4FwMask: /[\d\.\/\-:!]/i,

	IPv6: function(v) {
		var re =  Ext.String.format("^{0}$", Ext.form.field.VTypes.ReIPv6Addr);
		return new RegExp(re, "i").test(v);
	},
	IPv6Text: _("This field should be an IPv6 address"),
	IPv6Mask: /[0-9a-f:]/i,

	IPv6NetCIDR: function(v) {
		var re = Ext.String.format("^({0}\/(12[0-8]|1[0-1][0-9]|[1-9][0-9]|[0-9]))$",
		  Ext.form.field.VTypes.ReIPv6Addr)
		return new RegExp(re, "i").test(v);
	},
	IPv6NetCIDRText: _("This field should be a IPv6 network address in CIDR notation"),
	IPv6NetCIDRMask: /[0-9a-f:\/]/i,

	IPv6Fw: function(v) {
		// 2001:db8::15 or !2001:db8::/96
		var re = Ext.String.format("^(!)?({0})(\/(12[0-8]|[0-9]?\\d))?$",
		  Ext.form.field.VTypes.ReIPv6Addr);
		if(RegExp(re, "i").test(v))
			return true;
		// 2001:db8::10-2001:db8::20
		re = Ext.String.format("^(!)?(({0})([-]({1})){0,1})$",
		  Ext.form.field.VTypes.ReIPv6Addr, Ext.form.field.VTypes.ReIPv6Addr);
		return RegExp(re, "i").test(v);
	},
	IPv6FwText: _("This field should be either a IPv6 network address (with /mask), a IPv6 range or a plain IPv6 address (e.g. !2001:db8::/96 or 2001:db8::10-2001:db8::20 or 2001:db8::15)"),
	IPv6FwMask: /[0-9a-f\-:\/!]/i,

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
		var re = "[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]";
		return RegExp("^!?((" + re + ")([-:](" + re + ")){0,1})$", "i").test(v);
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

	hostnameIP: function(v) {
		if(Ext.form.field.VTypes.hostname(v))
			return true;
		if(Ext.form.field.VTypes.IP(v))
			return true;
		return false;
	},
	hostnameIPText: _("This field should be a hostname or an IP address"),
	hostnameIPMask: /[a-z0-9:\-\.]/i,

	hostnameIPv4: function(v) {
		if(Ext.form.field.VTypes.hostname(v))
			return true;
		if(Ext.form.field.VTypes.IPv4(v))
			return true;
		return false;
	},
	hostnameIPv4Text: _("This field should be a hostname or an IPv4 address"),
	hostnameIPv4Mask: /[a-z0-9\-\.]/i,

	hostnameIPList: function(v) {
		var valid = true;
		var parts = v.split(new RegExp('[,;]')) || [];
		Ext.Array.each(parts, function(part) {
			if(!Ext.form.VTypes.hostnameIP(part)) {
				valid = false;
				return false;
			}
		}, this);
		return valid;
	},
	hostnameIPListText: _("This field should be a list of hostnames or IP addresses"),
	hostnameIPListMask: /[a-z0-9:\-\.,;]/i,

	domainname: function(v) {
		// See http://shauninman.com/archive/2006/05/08/validating_domain_names
		return /^[a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?([.][a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?)*$/.test(v);
	},
	domainnameText: _("Invalid domain name"),
	domainnameMask: /[a-z0-9\-\.]/i,

	domainnameIP: function(v) {
		if(Ext.form.field.VTypes.domainname(v))
			return true;
		if(Ext.form.field.VTypes.IP(v))
			return true;
		return false;
	},
	domainnameIPText: _("This field should be a domainname or an IP address"),
	domainnameIPMask: /[a-z0-9:\-\.]/i,

	domainnameIPv4: function(v) {
		if(Ext.form.field.VTypes.domainname(v))
			return true;
		if(Ext.form.field.VTypes.IPv4(v))
			return true;
		return false;
	},
	domainnameIPv4Text: _("This field should be a domainname or an IPv4 address"),
	domainnameIPv4Mask: /[a-z0-9\-\.]/i,

	domainnameIPList: function(v) {
		var valid = true;
		var parts = v.split(new RegExp('[,;]')) || [];
		Ext.Array.each(parts, function(part) {
			if(!Ext.form.VTypes.domainnameIP(part)) {
				valid = false;
				return false;
			}
		}, this);
		return valid;
	},
	domainnameIPListText: _("This field should be a list of domainnames or IP addresses"),
	domainnameIPListMask: /[a-z0-9:\-\.,;]/i,

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
		return !/["':]/.test(v);
	},
	commentText: _("The comment contains invalid characters"),
	commentMask: /[^"':]/,

	password: function(v) {
		return !/[^a-zA-Z0-9\.\-_]/.test(v);
	},
	passwordText: _("The password contains invalid characters"),
	passwordMask: /[a-zA-Z0-9\.\-_]/,

	// String that are used as file system label
	fslabel: function(v) {
		return /^[a-zA-Z0-9]+$/.test(v);
	},
	fslabelText: _("Invalid file system label"),
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
	devnameMask: /[a-zA-Z0-9\.\-_]/,

	noBlank: function(v) {
		return !/[ ]+/.test(v);
	},
	noBlankText: _("This field does not allow blanks"),
	noBlankMask: /[^ ]/i,

	email: function(v) {
		// http://de.wikipedia.org/wiki/Top-Level-Domain#Spezialf.C3.A4lle
		return /^(")?(?:[^\."])(?:(?:[\.])?(?:[\w\-!#$%&'*+/=?^_`{|}~]))*\1@(\w[\-\w]*\.){1,5}([A-Za-z]){2,}$/.test(v);
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.Function
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.Function, {
	/**
	 * Create a combined function call sequence of the original function
	 * and the passed function. The resulting function returns the results
	 * of the original function. The passed function is called with the
	 * parameters of the original function and it's result.
	 */
	createSequenceEx: function(originalFn, newFn, scope) {
		if (!newFn)
			return originalFn;
		else {
			return function() {
				// Call the original function.
				var result = originalFn.apply(this, arguments);
				// Append the result to the end of the arguments list.
//				arguments[arguments.length] = result;
//				arguments.length++;
				var args = Ext.Array.slice(arguments);
				Ext.Array.push(args, [ result ]);
				// Call the new function.
				newFn.apply(scope || this, args);
				return result;
			};
		}
	},

	/**
	 * Adds behavior to an existing method that is executed after the
	 * original behavior of the function. The passed function is called
	 * with the parameters of the original function and it's result.
	 */
	interceptAfterEx: function(object, methodName, fn, scope) {
		var method = object[methodName] || Ext.emptyFn;
		return (object[methodName] = function() {
			// Call the original function.
			var result = method.apply(this, arguments);
			// Append the result to the end of the arguments list.
//			arguments[arguments.length] = result;
//			arguments.length++;
			var args = Ext.Array.slice(arguments);
			Ext.Array.push(args, [ result ]);
			// Call the new function.
			return fn.apply(scope || this, args);
		});
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.LoadMask
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.LoadMask.prototype, {
	msg: _("Loading ...")
});

////////////////////////////////////////////////////////////////////////////////
// Ext.window.MessageBox
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.window.MessageBox, {
	buttonText: {
		ok: _("OK"),
		cancel: _("Cancel"),
		yes: _("Yes"),
		no: _("No")
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.data.Store
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.data.Store.prototype, {
	dirty: false,

	markDirty: function() {
		this.dirty = true;
	},

	isDirty: function() {
		return this.dirty;
	},

	isLoaded: function() {
		var me = this;
		// Note, this method may be incorrect in some cases, but in
		// most cases the following indications should be suitable to
		// find out whether a store has been loaded or not.
		if(me.isLoading())
			return false;
		if(Ext.isDefined(me.totalCount))
			return true;
		if(Ext.isDefined(me.lastOptions))
			return true;
		// Note, if the store has been loaded but no content has been
		// received the following test returns an incorrect result.
		if(me.getCount() > 0)
			return true;
		return false;
	},

	/**
	 * Adds data to the store. The model instance will be created
	 * automatically.
	 */
	addData: function(data) {
		var me = this;
		var record = me.createModel(data);
		return me.add(record);
	},

	/**
	 * Inserts data into the store at the given index. The model instance
	 * will be created automatically.
	 */
	insertData: function(index, data) {
		var me = this;
		var records = [];
		if (!Ext.isIterable(data))
			data = [ data ];
		Ext.Array.each(data, function(item) {
			var record = me.createModel(item);
			Ext.Array.push(records, record);
		});
		return me.insert(index, records);
	},

	/**
	 * Gets all values for each record in this store.
	 * @return An array of object hash containing all the record's values.
	 */
	getData: function() {
		var me = this;
		var result = [];
		me.each(function(record) {
			var data = record.getData();
			Ext.Array.push(result, [ data ]);
		});
		return result;
	}
});

// Fix modified issue when adding, inserting or removing records from a store.
Ext.data.Store.prototype.insert = Ext.Function.createSequence(
  Ext.data.Store.prototype.insert, function(index, records) {
	this.markDirty();
});
Ext.data.Store.prototype.add = Ext.Function.createSequence(
  Ext.data.Store.prototype.add, function(records) {
	this.markDirty();
});
Ext.Function.interceptAfter(Ext.data.Store.prototype, "remove",
  function(records) {
	this.markDirty();
});
Ext.Function.interceptAfter(Ext.data.Store.prototype, "afterEdit",
  function(record) {
	var me = this, recs = me.getModifiedRecords();
	if(recs.length > 0)
		me.markDirty();
});
Ext.Function.interceptAfter(Ext.data.Store.prototype, "rejectChanges",
  function() {
	this.dirty = false;
});
Ext.Function.interceptAfter(Ext.data.Store.prototype, "commitChanges",
  function() {
	this.dirty = false;
});

////////////////////////////////////////////////////////////////////////////////
// Ext.state.CookieProvider
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.state.CookieProvider.prototype, {
	/**
	 * Clear all states.
	 */
	clearAll: function() {
		var me = this;
		var cookie = document.cookie + ";";
		var re = /\s?(.*?)=(.*?);/g;
		var matches;
		var prefix = me.prefix;
		var len = prefix.length;
		while(null != (matches = re.exec(cookie))) {
			var name = matches[1];
			if(name && name.substring(0, len) == prefix) {
				this.clear(name.substr(len));
			}
		}
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.field.Field
////////////////////////////////////////////////////////////////////////////////

// Do not reset a form field that is read-only.
Ext.Function.createInterceptor(Ext.form.field.Field.prototype, "reset",
  function() {
	if(this.readOnly)
		return false;
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.field.Trigger
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.form.field.Trigger.prototype, {
	getTriggerButtonEl: function(id) {
		var me = this, el = null;
		if(Ext.isNumber(id)) {
			el = me.triggerEl.item(id);
		} else if(Ext.isString(id)) {
			// Search by the given CSS class.
			var selector = Ext.String.format("[class~={0}][role=presentation]", id);
			el = me.getEl().query(selector)[0];
		}
		return el;
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.field.ComboBox
////////////////////////////////////////////////////////////////////////////////

Ext.form.field.ComboBox.prototype.setValue = Ext.Function.createInterceptor(
  Ext.form.field.ComboBox.prototype.setValue, function() {
	var me = this;
	// Check if the store has already been loaded. If not, then call this
	// function again after the store has been loaded. Note, if the store
	// is already loading then do nothing, the base implementation will
	// handle this for us.
	// Note, in some situations it occurs that the setValue function is
	// called multiple times before the store has been loaded. To ensure
	// that unfired listeners do not overwrite the value which is
	// successfully set after the store has been loaded it is necessary
	// to remove all of them.
	if(!me.store.isLoading() && !me.store.isLoaded()) {
		var fn = Ext.Function.bind(me.setValue, me, arguments);
		if(!Ext.isDefined(me.setValueListeners))
			me.setValueListeners = [];
		me.setValueListeners.push(fn);
		me.store.on({
			single: true,
			load: fn
		});
		return false;
	}
	// Remove buffered listeners (which have not been fired until now),
	// otherwise they will overwrite the given value.
	if(Ext.isDefined(me.setValueListeners)) {
		Ext.Array.each(me.setValueListeners, function(fn) {
			me.store.un("load", fn)
		});
		delete me.setValueListeners;
	}
});

Ext.apply(Ext.form.field.ComboBox.prototype, {
	/**
	 * Get the record of the current selection.
	 * @return The matched record or FALSE.
	 */
	getRecord: function() {
		var me = this;
		var value = me.getValue();
		var record = me.findRecordByValue(value);
		return record;
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.Base
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.Base.prototype, {
	callPrototype: function(methodName, args) {
		var proto = Object.getPrototypeOf(this);
		return proto[methodName].apply(this, args || []);
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.Array
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.Array, {
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
	range: function(low, high, step, asString) {
		var array = [];
		var startv = low;
		var endv = high;
		var stepv = step || 1;
		asString = asString || false;

		if(startv < endv) {
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
	},

	/**
	 * @function walk
	 * Change each value according to a callback function.
	 * @param array The array to process.
	 * @param fn The function to apply.
	 * @return Returns the modified array.
	 */
	walk: function(array, fn) {
		var a = [], i = array.length;
		while(i--) {
			a.push(fn(array[i]));
		}
		return a.reverse();
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.ClassManager
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.ClassManager, {
	/**
	 * Converts a string expression to an array of matching class names. This
	 * method is equal to 'getNamesByExpression' but returns also classes
	 * that do not have an alias.
	 * @param expression The class name expression to search for.
	 */
	getClassNamesByExpression: function(expression) {
		var names = [], name, regex;
		if (expression.indexOf('*') !== -1)
			expression = expression.replace(/\*/g, '(.*?)');
		regex = new RegExp('^' + expression + '$');
		for (name in this.classes) {
			if (name.search(regex) !== -1) {
				names.push(name);
			}
		}
		return names;
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.window.Window
////////////////////////////////////////////////////////////////////////////////

Ext.override(Ext.window.Window, {
	/**
	 * Stateful windows position does not work correct. See:
	 * http://www.sencha.com/forum/showthread.php?249459-4.1.3-Stateful-window-position-is-STILL-incorrect
	 * http://www.sencha.com/forum/showthread.php?223430-4.1.1-Window-getState-should-respect-the-floatParent-propert-of-a-window
	 */
	getState: function() {
		var me = this;
		var state = me.callParent();
		if (!(!!me.maximized || me.ghostBox)) {
			Ext.apply(state, {
				pos: me.getPosition(!!me.floatParent)
			});
		}
		return state;
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.Object
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.Object, {
	hasProperty: function(object, name) {
		return object.hasOwnProperty(name);
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
	applyEx: function(o, c, defaults) {
		// no "this" reference for friendly out of scope calls
		if(defaults) {
			Ext.apply(o, defaults);
		}
		if(o && c && typeof c == 'object') {
			for(var p in c) {
				if((typeof c[p] == 'object') && (typeof o[p] !== 'undefined')) {
					Ext.applyEx(o[p], c[p]);
				} else {
					o[p] = c[p];
				}
			}
		}
		return o;
	},

	/**
	 * Finds out whether a variable is an UUID v4.
	 * @param value The variable being evaluated.
	 * @return TRUE if variable is a UUID, otherwise FALSE.
	 */
	isUUID: function(value) {
		if(Ext.isEmpty(value) || !Ext.isString(value))
			return false;
		return /^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}$/i.test(value);
	}
});
