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
// require("js/omv/NavigationPanel.js")

Ext.ns("OMV.Module.Information");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("information", "about", {
	text: "About",
	icon: "images/about.png",
	position: 30
});

/**
 * @class OMV.Module.Information.About
 * @derived Ext.Panel
 */
OMV.Module.Information.About = function(config) {
	var initialConfig = {
		border: false
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Information.About.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.Information.About, Ext.Panel, {
	initComponent : function() {
		this.html = "<form style='overflow: auto; height: 100%;'>";
		// Copyright
		this.html += this.createBox("OpenMediaVault is Copyright © 2009-2012 by Volker Theile (volker.theile@openmediavault.org).<br/>" +
		  "All rights reserved.<br/><br/>" +
		  "OpenMediaVault is free software: you can redistribute it and/or modify " +
		  "it under the terms of the GNU General Public License v3 as published by " +
		  "the Free Software Foundation.<br/><br/>" +
		  "OpenMediaVault is distributed in the hope that it will be useful, " +
		  "but WITHOUT ANY WARRANTY; without even the implied warranty of " +
		  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the " +
		  "GNU General Public License for more details.<br/><br/>" +
		  "You should have received a copy of the GNU General Public License " +
		  "along with OpenMediaVault. If not, see &lt;<a href='http://www.gnu.org/licenses' " +
		  "target='_blank'>http://www.gnu.org/licenses</a>&gt;.");
		// Render list of used software and licenses
		this.html += this.createBox(this.renderSoftware() + "<br/><br/>" +
		  this.renderLicenses());
		this.html += "<br/></form>";
		OMV.Module.Information.About.superclass.initComponent.apply(this,
		  arguments);
	},

	createBox : function(msg) {
		return [
			'<div class="x-box-aboutbox">',
			'<div class="x-box-tl"><div class="x-box-tr"><div class="x-box-tc"></div></div></div>',
			'<div class="x-box-ml"><div class="x-box-mr"><div class="x-box-mc"><h3 class="x-icon-text"></h3>', msg, '</div></div></div>',
			'<div class="x-box-bl"><div class="x-box-br"><div class="x-box-bc"></div></div></div>',
			'</div>'
		].join('');
	},

	renderSoftware : function() {
		var html = "";
		var packages = [{
			name: "ExtJS",
			url: "http://www.sencha.com"
		},{
			name: "Apache",
			url: "http://httpd.apache.org"
		},{
			name: "PHP",
			url: "http://www.php.net"
		},{
			name: "PHP JSON Schema",
			url: "http://code.google.com/p/json-schema-php"
		},{
			name: "Xmlstarlet",
			url: "http://xmlstar.sourceforge.net"
		},{
			name: "Postfix",
			url: "http://www.postfix.org"
		},{
			name: "RSyslog",
			url: "http://www.rsyslog.com"
		},{
			name: "Smartmontools",
			url: "http://smartmontools.sourceforge.net"
		},{
			name: "OpenSSL",
			url: "http://www.openssl.org"
		},{
			name: "NFS",
			url: "http://nfs.sourceforge.net"
		},{
			name: "ProFTPD",
			url: "http://www.proftpd.org"
		},{
			name: "TFTP",
			url: "http://www.kernel.org/pub/software/network/tftp"
		},{
			name: "Samba",
			url: "http://www.samba.org"
		},{
			name: "RSync",
			url: "http://rsync.samba.org"
		},{
			name: "snmpd",
			url: "http://net-snmp.sourceforge.net"
		},{
			name: "Avahi",
			url: "http://avahi.org"
		},{
			name: "iptables",
			url: "http://www.netfilter.org"
		},{
			name: "Monit",
			url: "http://mmonit.com/monit"
		},{
			name: "RRDtool",
			url: "http://oss.oetiker.ch/rrdtool"
		},{
			name: "Collectd",
			url: "http://collectd.org"
		},{
			name: "Cron",
			url: "http://ftp.isc.org/isc/cron"
		},{
			name: "LVM2",
			url: "http://sources.redhat.com/lvm2"
		},{
			name: "Anacron",
			url: "http://anacron.sourceforge.net"
		}];
		packages.each(function(o) {
			if (!Ext.isEmpty(html)) html += "<br/>";
			html += String.format("<p>{0}<br/><a href='{1}' target='_blank'>" +
			  "{2}<a></p>", o.name, o.url, o.url);
		}, this);
		return "OpenMediaVault is based upon various free software " +
		  "like:<br/><br/>" + html + "...";
	},

	renderLicenses : function() {
		var html = "";
		var licenses = [{
			name: "GNU GPLv2",
			url: "licenses/gpl-2_0.txt",
		},{
			name: "GNU GPLv3",
			url: "licenses/gpl-3_0.txt",
		},{
			name: "PHP License",
			url: "licenses/php-3_01.txt",
		},{
			name: "IBM Public License",
			url: "licenses/ibm-public-1_0.txt",
		}]
		licenses.each(function(o) {
			if (!Ext.isEmpty(html)) html += ", ";
			html += String.format("<a href='{0}' target='_blank'>" +
			  "{1}</a>", o.url, o.name);
		}, this);
		return "Some of the software is licensed under " + html + ", ...";
	}
});
OMV.NavigationPanelMgr.registerPanel("information", "about", {
	cls: OMV.Module.Information.About
});
