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

	IPList: function(v) {
		var valid = true;
		// Split string into several IP addresses.
		Ext.each(v.split(/[,;]/), function(ip) {
			// Is it a valid IPv4 or IPv6 address?
			if (!Ext.form.field.VTypes.IP(ip)) {
				valid = false;
				return false;
			}
		});
		return valid;
	},
	IPListText: "This field should be a list of IP addresses separated by <,> or <;>",
	IPListMask: /[0-9a-f\.:,;]/i,

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

	IPv4List: function(v) {
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
	numListText: _("This field should only contain numbers separated by <,> or <;>"),
	numListMask: /[\d,;]/i,

	textList: function(v) {
		return /^(\w+[,;])*\w+$/.test(v);
	},
	textListText: _("This field should only contain strings separated by <,> or <;>"),
	textListMask: /[\w,;]/i,

	textCommaList: function(v) {
		return /^(\w+[,])*\w+$/.test(v);
	},
	textCommaListText: _("This field should only contain strings separated by <,>"),
	textCommaListMask: /[\w,]/i,

	portList: function(v) {
		return /^((0|[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])[,;])*(0|[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/.test(v);
	},
	portListText: _("This field should only contain ports (0 - 65535) separated by <,> or <;>"),
	portListMask: /[\d,;]/i,

	hostname: function(v) {
		return /^[a-zA-Z]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9]){0,1}$/.test(v);
	},
	hostnameText: _("Invalid host name"),
	hostnameMask: /[a-z0-9\-]/i,

	hostnameIP: function(v) {
		if(Ext.form.field.VTypes.hostname(v))
			return true;
		if(Ext.form.field.VTypes.IP(v))
			return true;
		return false;
	},
	hostnameIPText: _("This field should be a host name or an IP address"),
	hostnameIPMask: /[a-z0-9:\-\.]/i,

	hostnameIPv4: function(v) {
		if(Ext.form.field.VTypes.hostname(v))
			return true;
		if(Ext.form.field.VTypes.IPv4(v))
			return true;
		return false;
	},
	hostnameIPv4Text: _("This field should be a host name or an IPv4 address"),
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
	hostnameIPListText: _("This field should be a list of host names or IP addresses separated by <,> or <;>"),
	hostnameIPListMask: /[a-z0-9:\-\.,;]/i,

	hostnameIPNetCIDR: function(v) {
		if(Ext.form.field.VTypes.hostname(v))
			return true;
		if(Ext.form.field.VTypes.IPNetCIDR(v))
			return true;
		return false;
	},
	hostnameIPNetCIDRText: _("This field should be a host name or an IP address in CIDR notation"),
	hostnameIPNetCIDRMask: /[a-z0-9:\-\.\/]/i,

	hostnameIPNetCIDRList: function(v) {
		var valid = true;
		var parts = v.split(new RegExp('[,;]')) || [];
		Ext.Array.each(parts, function(part) {
			if(!Ext.form.VTypes.hostnameIPNetCIDR(part)) {
				valid = false;
				return false;
			}
		}, this);
		return valid;
	},
	hostnameIPNetCIDRListText: _("This field should be a list of host names or IP addresses in CIDR notation separated by <,> or <;>"),
	hostnameIPNetCIDRListMask: /[a-z0-9:\-\.\/,;]/i,

	domainname: function(v) {
		// See http://shauninman.com/archive/2006/05/08/validating_domain_names
		return /^[a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?([.][a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])?)*$/.test(v);
	},
	domainnameText: _("Invalid domain name"),
	domainnameMask: /[a-z0-9\-\.]/i,

	domainnameList: function(v) {
		var valid = true;
		var parts = v.split(new RegExp('[,;]')) || [];
		Ext.Array.each(parts, function(part) {
			if (!Ext.form.VTypes.domainname(part)) {
				valid = false;
				return false;
			}
		}, this);
		return valid;
	},
	domainnameListText: _("This field should be a list of domain names or IP addresses separated by <,> or <;>"),
	domainnameListMask: /[a-z0-9:\-\.,;]/i,

	domainnameIP: function(v) {
		if(Ext.form.field.VTypes.domainname(v))
			return true;
		if(Ext.form.field.VTypes.IP(v))
			return true;
		return false;
	},
	domainnameIPText: _("This field should be a domain name or an IP address"),
	domainnameIPMask: /[a-z0-9:\-\.]/i,

	domainnameIPv4: function(v) {
		if(Ext.form.field.VTypes.domainname(v))
			return true;
		if(Ext.form.field.VTypes.IPv4(v))
			return true;
		return false;
	},
	domainnameIPv4Text: _("This field should be a domain name or an IPv4 address"),
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
	domainnameIPListText: _("This field should be a list of domain names or IP addresses"),
	domainnameIPListMask: /[a-z0-9:\-\.,;]/i,

	smtpserver: function(v) {
		if (!Ext.isString(v))
			return false;
		v = v.replace(/[\[\]]/g, "");
		if (Ext.form.field.VTypes.domainname(v))
			return true;
		if (Ext.form.field.VTypes.IP(v))
			return true;
		return false;
	},
	smtpserverText: _("This field should be a domain name or an IP address"),
	smtpserverMask: /[a-z0-9:\[\]\-\.]/i,

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
	},

	sshPubKey: function(v) {
		if (Ext.form.field.VTypes.sshPubKeyOpenSSH(v))
			return true;
		if (Ext.form.field.VTypes.sshPubKeyRFC4716(v))
			return true;
		return false;
	},
	sshPubKeyText: _("Invalid SSH public key (OpenSSH or RFC 4716 format)"),
	sshPubKeyMask: /[a-zA-Z0-9@+/=\- ]/,

	sshPubKeyRFC4716: function(v) {
		// See https://tools.ietf.org/html/rfc4716#section-3.4
		return RegExp("^---- BEGIN SSH2 PUBLIC KEY ----(\n|\r|\f)((.+)?" +
		  "((\n|\r|\f).+)*)(\n|\r|\f)---- END SSH2 PUBLIC KEY ----" +
		  "[\n\r\f]*$").test(v);
	},
	sshPubKeyRFC4716Text: _("Invalid SSH public key (RFC 4716 format)"),
	sshPubKeyRFC4716Mask: /[a-zA-Z0-9+/=\- ]/,

	sshPubKeyOpenSSH: function(v) {
		return /^ssh-rsa AAAA[0-9A-Za-z+/]+[=]{0,3}\s*(.+)?$/.test(v);
	},
	sshPubKeyOpenSSHText: _("Invalid SSH public key (OpenSSH format)"),
	sshPubKeyOpenSSHMask: /[a-zA-Z0-9@+/=\- ]/,

	nfsOptionList: function(v) {
		return /^(([a-zA-Z_]+)(=([\w@:/]+))?[,])*([a-zA-Z_]+)(=([\w@:/]+))?$/.test(v);
	},
	nfsOptionListText: _("This field should only contain options separated by <,>"),
	nfsOptionListMask: /[\w,@:/=]/i
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
// Ext.panel.Panel
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.panel.Panel.prototype, {
	// Disable the shadow by default.
	shadow: false
});

////////////////////////////////////////////////////////////////////////////////
// Ext.view.AbstractView
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.view.AbstractView.prototype, {
	loadingText: _("Loading ...")
});

////////////////////////////////////////////////////////////////////////////////
// Ext.window.MessageBox
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.window.MessageBox.prototype, {
	buttonText: {
		ok: _("OK"),
		cancel: _("Cancel"),
		yes: _("Yes"),
		no: _("No")
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.grid.RowEditor
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.grid.RowEditor.prototype, {
	saveBtnText: _("Save"),
	cancelBtnText: _("Cancel"),
	errorsText: _("Errors"),
	dirtyText: _("You need to commit or cancel your changes")
});

// https://www.sencha.com/forum/showthread.php?107914-Row-Editor-Checkbox-alignment-problem
// https://stackoverflow.com/a/23888701
Ext.define("Ext.overrides.grid.RowEditor", {
	override: "Ext.grid.RowEditor",
	compatibility: "6.2.0.981",
	requires: [
		"Ext.grid.RowEditor"
	],
	addFieldsForColumn: function(column, initial) {
		var me = this, i, len, field, style;
		if (Ext.isArray(column)) {
			for (i = 0, len = column.length; i < len; i++) {
				me.addFieldsForColumn(column[i], initial);
			}
			return;
		}
		if (column.getEditor) {
			field = column.getEditor(null, me.getDefaultFieldCfg());
			if (column.align === 'right') {
				style = field.fieldStyle;
				if (style) {
					if (Ext.isObject(style)) {
						style = Ext.apply({}, style);
					} else {
						style = Ext.dom.Element.parseStyles(style);
					}
					if (!style.textAlign && !style['text-align']) {
						style.textAlign = 'right';
					}
				} else {
					style = 'text-align:right';
				}
				field.fieldStyle = style;
			}
			// ===> STARTFIX
			if (column.align === 'center') {
				field.fieldStyle = 'text-align:center';
			}
			// <=== ENDFIX
			if (column.xtype === 'actioncolumn') {
				field.fieldCls += ' ' + Ext.baseCSSPrefix + 'form-action-col-field';
			}
			if (me.isVisible() && me.context) {
				if (field.is('displayfield')) {
					me.renderColumnData(field, me.context.record, column);
				} else {
					field.suspendEvents();
					field.setValue(me.context.record.get(column.dataIndex));
					field.resumeEvents();
				}
			}
			if (column.hidden) {
				me.onColumnHide(column);
			} else if (column.rendered && !initial) {
				// Setting after initial render
				me.onColumnShow(column);
			}
		}
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.toolbar.Paging
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.toolbar.Paging.prototype, {
	displayMsg: _("Displaying {0} - {1} of {2}"),
	emptyMsg: _("No data to display"),
	beforePageText: _("Page"),
	afterPageText: _("of {0}"),
	firstText: _("First Page"),
	prevText: _("Previous Page"),
	nextText: _("Next Page"),
	lastText: _("Last Page"),
	refreshText: _("Refresh")
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
		if (me.isLoading())
			return false;
		if (Ext.isDefined(me.totalCount))
			return true;
		if (Ext.isDefined(me.lastOptions))
			return true;
		// Note, if the store has been loaded but no content has been
		// received the following test returns an incorrect result.
		if (me.getCount() > 0)
			return true;
		return false;
	},

	/**
	 * Adds data to the store. The model instance will be created
	 * automatically.
	 */
	addData: function(data) {
		var me = this;
		return me.add(data);
	},

	addRawData: function(data) {
		var me = this;
		var session = me.getSession();
		var result = me.getProxy().getReader().read(data, session ? {
			  recordCreator: session.recordCreator
		  } : undefined);
		var records = result.getRecords();
		var success = result.getSuccess();
		if (!success)
			return [];
		return me.add(records);
	},

	/**
	 * Inserts data into the store at the given index. The model instance
	 * will be created automatically.
	 */
	insertData: function(index, data) {
		var me = this;
		return me.insert(index, data);
	},

	/**
	 * Gets all values for each record in this store according the model
	 * definition.
	 * @return An array of object hash containing all the model's values.
	 */
	getModelData: function(options) {
		var me = this;
		var result = [];
		me.each(function(model) {
			var data = model.getData(options);
			Ext.Array.push(result, data);
		});
		return result;
	},

	/**
	 * Finds the first matching Record in this store by a specific field value.
	 * @param fieldName The name of the Record field to test.
	 * @param value The value to match the field against.
	 * @param The index to start searching at.
	 * @return The matched record or NULL.
	 */
	findExactRecord: function(fieldName, value, start) {
		var me = this;
		var index = me.findExact(fieldName, value, start);
		if (-1 == index)
			return null;
		return me.getAt(index);
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
// Ext.state.LocalStorageProvider
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.state.LocalStorageProvider.prototype, {
	/**
	 * Clear all states.
	 */
	clearAll: function() {
		var me = this;
		var keys = me.store.getKeys();
		Ext.Array.each(keys, function(key) {
			me.clear(key);
		});
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.Basic
////////////////////////////////////////////////////////////////////////////////

// When clicking very fast in the navigation tree this method fails because
// of the missing ckeck if 'this.monitor' is defined.
Ext.define("Ext.overrides.form.Basic", {
	override: "Ext.form.Basic",
	compatibility: "6.2.0.981",
	requires: [
		"Ext.form.Basic"
	],
	getFields: function() {
		if (!Ext.isObject(this.monitor))
			return new Ext.util.MixedCollection();
		return this.monitor.getItems();
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.field.Checkbox
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.form.field.Checkbox.prototype, {
	// Set the default values.
	inputValue: true,
	uncheckedValue: false
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.CheckboxGroup
////////////////////////////////////////////////////////////////////////////////

Ext.apply(Ext.form.CheckboxGroup.prototype, {
	invalidCls: Ext.baseCSSPrefix + "form-invalid"
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.field.Field
////////////////////////////////////////////////////////////////////////////////

// Do not reset a form field that is read-only.
Ext.Function.createInterceptor(Ext.form.field.Field.prototype, "reset",
  function() {
	if (this.readOnly)
		return false;
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.field.Text
////////////////////////////////////////////////////////////////////////////////

Ext.form.field.Text.prototype.afterRender = Ext.Function.createInterceptor(
  Ext.form.field.Text.prototype.afterRender, function() {
	var me = this;
	// Set 'autocomplete="on|off"' if the property autoComplete is set.
	if (Ext.isDefined(me.autoComplete)) {
		me.inputEl.set({
			autocomplete: me.autoComplete ? "on" : "off"
		});
	}
	// Set 'autocapitalize="on|off"' if the property autoCapitalize is set.
	if (Ext.isDefined(me.autoCapitalize)) {
		// See https://developer.apple.com/library/safari/documentation/AppleApplications/Reference/SafariHTMLRef/Articles/Attributes.html
		me.inputEl.set({
			autocapitalize: me.autoCapitalize ? "sentences" : "none"
		});
	}
	// Set 'autocorrect="on|off"' if the property autoCorrect is set.
	if (Ext.isDefined(me.autoCorrect)) {
		me.inputEl.set({
			autocorrect: me.autoCorrect ? "on" : "off"
		});
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.form.field.ComboBox
////////////////////////////////////////////////////////////////////////////////

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
	},

	/**
	 * @function findObject
	 * Search an array of objects and find an object by its key/value.
	 * @param array The array to process.
	 * @param key The key of the element.
	 * @param value The value to search for.
	 * @return The first object in the array which matches the key/value,
	 *   or NULL if none was found.
	 */
	findObject: function(array, key, value) {
		var object = Ext.Array.findBy(array, function(item) {
			if (!Ext.isObject(item))
				return false;
			return (item[key] == value);
		});
		return object;
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
	 * @return An array of class names.
	 */
	getClassNamesByExpression: function(expression) {
		var names = [], name, regex;
		expression = Ext.String.escapeRegex(expression);
		expression = expression.replace(/\\\*/g, '(.*?)');
		regex = new RegExp('^' + expression + '$');
		for (name in this.classes) {
			if (name.search(regex) !== -1)
				names.push(name);
		}
		return names;
	},

	/**
	 * Converts a string expression to an array of matching aliases.
	 * @param expression The alias expression to search for.
	 * @return An array of aliases.
	 */
	getAliasesByExpression: function(expression) {
		var aliases = [], name, regex;
		expression = Ext.String.escapeRegex(expression);
		expression = expression.replace(/\\\*/g, '(.*?)');
		regex = new RegExp('^' + expression + '$');
		for (name in this.classes) {
			var object = this.classes[name];
			if (!Ext.isDefined(object.prototype) || !Ext.isArray(
			  object.prototype.alias))
				continue;
			Ext.Array.each(object.prototype.alias, function(alias) {
				if (alias.search(regex) !== -1)
					aliases.push(alias);
			});
		}
		return aliases;
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
// Ext.dom.Element
////////////////////////////////////////////////////////////////////////////////

// https://www.sencha.com/forum/showthread.php?336762-Examples-don-t-work-in-Firefox-52-touchscreen&p=1174857&viewfull=1#post1174857
Ext.define('EXTJS_23846.Element', {
	override: 'Ext.dom.Element'
}, function(Element) {
	var supports = Ext.supports,
		proto = Element.prototype,
		eventMap = proto.eventMap,
		additiveEvents = proto.additiveEvents;

	if (Ext.os.is.Desktop && supports.TouchEvents && !supports.PointerEvents) {
		eventMap.touchstart = 'mousedown';
		eventMap.touchmove = 'mousemove';
		eventMap.touchend = 'mouseup';
		eventMap.touchcancel = 'mouseup';

		additiveEvents.mousedown = 'mousedown';
		additiveEvents.mousemove = 'mousemove';
		additiveEvents.mouseup = 'mouseup';
		additiveEvents.touchstart = 'touchstart';
		additiveEvents.touchmove = 'touchmove';
		additiveEvents.touchend = 'touchend';
		additiveEvents.touchcancel = 'touchcancel';

		additiveEvents.pointerdown = 'mousedown';
		additiveEvents.pointermove = 'mousemove';
		additiveEvents.pointerup = 'mouseup';
		additiveEvents.pointercancel = 'mouseup';
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.event.publisher.Gesture
////////////////////////////////////////////////////////////////////////////////

// https://www.sencha.com/forum/showthread.php?336762-Examples-don-t-work-in-Firefox-52-touchscreen&p=1174857&viewfull=1#post1174857
Ext.define('EXTJS_23846.Gesture', {
	override: 'Ext.event.publisher.Gesture'
}, function(Gesture) {
	var me = Gesture.instance;

	if (Ext.supports.TouchEvents && !Ext.isWebKit && Ext.os.is.Desktop) {
		me.handledDomEvents.push('mousedown', 'mousemove', 'mouseup');
		me.registerEvents();
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.ZIndexManager
////////////////////////////////////////////////////////////////////////////////

// https://twasink.net/2016/09/20/upgrading-to-extjs-6-2/
Ext.define('Ext.overrides.ZIndexManager', {
	override: 'Ext.ZIndexManager',
	compatibility: '6.2.0.981',
	requires: [
		'Ext.ZIndexManager'
	],
	privates: {
		syncModalMask: function(comp) {
			// ExtJS 6.2.0.981 has a bug where it doesn't look to see if the
			// mask is rendered before trying to sync it. That's not the best
			// thing. Here's a tentative fix: do nothing if the mask doesn't
			// have a target.
			if (!this.mask || !this.mask.maskTarget) { return; }
			this.callParent(arguments);
		}
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.window.Window
////////////////////////////////////////////////////////////////////////////////

// https://github.com/JarvusInnovations/sencha-hotfixes/blob/ext/6/2/0/981/overrides/window/ResizeChildren.js
// Works around issue where a panel inside a window doesn't get resized
// correctly after the window is resized.
Ext.define('Ext.overrides.window.Window', {
	override: 'Ext.window.Window',
	compatibility: '6.2.0.981',
	requires: [
		'Ext.window.Window'
	],
	onShowComplete: function() {
		var me = this;
		me.callParent(arguments);
		if (me.child('container:not(header)')) {
			me.on('resize', 'doResizeChildrenHotfix', me);
		}
	},
	doClose: function() {
		this.un('resize', 'doResizeChildrenHotfix', this);
		this.callParent(arguments);
	},
	doResizeChildrenHotfix: function() {
		this.updateLayout();
	}
});

////////////////////////////////////////////////////////////////////////////////
// Ext.view.Table
////////////////////////////////////////////////////////////////////////////////

// https://github.com/JarvusInnovations/sencha-hotfixes/blob/ext/6/2/0/981/overrides/view/TableReplaceScroll.js
// Works around issue where a grid's scroll will jump back to last focused
// record when a group is expanded/collapsed
Ext.define('Ext.overrides.view.Table', {
	override: 'Ext.view.Table',
	compatibility: '6.2.0.981',
	requires: [
		'Ext.view.Table'
	],
	onReplace: function() {
		this.saveScrollState();
		this.callParent(arguments);
		this.restoreScrollState();
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
	 * @param defaults A different object that will also be applied
	 *   for default values.
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
	isUuid: function(value) {
		if (!Ext.isDefined(value) || !Ext.isString(value) ||
		  Ext.isEmpty(value))
			return false;
		return /^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}$/i.test(value);
	},

	/**
	 * Returns true if the passed value is NULL.
	 * @param value The variable being evaluated.
	 * @return TRUE if variable is NULL, otherwise FALSE.
	 */
	isNull: function(value) {
		return (value == null);
	},

	/**
	 * Finds out whether a variable describes a devicefile.
	 * @param value The variable being evaluated.
	 * @return TRUE if the variable describes a devicefile, otherwise FALSE.
	 */
	isDeviceFile: function(value) {
		if (!Ext.isString(value))
			return false;
		return /^\/dev\/.+$/i.test(value);
	}
});
