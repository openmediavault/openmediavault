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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/panel/Panel.js")

/**
 * @class OMV.module.public.info.about.About
 * @derived OMV.workspace.panel.Panel
 */
Ext.define("OMV.module.public.info.about.About", {
	extend: "OMV.workspace.panel.Panel",

	initComponent: function() {
		var me = this;
		me.html = "<form style='overflow: auto; height: 100%;'>";
		// Copyright
		me.html += me.createBox("openmediavault is Copyright © 2009-2021 by Volker Theile (volker.theile@openmediavault.org).<br/>" +
		  "All rights reserved.<br/><br/>" +
		  "openmediavault is free software: you can redistribute it and/or modify " +
		  "it under the terms of the GNU General Public License v3 as published by " +
		  "the Free Software Foundation.<br/><br/>" +
		  "openmediavault is distributed in the hope that it will be useful, " +
		  "but WITHOUT ANY WARRANTY; without even the implied warranty of " +
		  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the " +
		  "GNU General Public License for more details.<br/><br/>" +
		  "You should have received a copy of the GNU General Public License " +
		  "along with openmediavault. If not, see &lt;<a href='http://www.gnu.org/licenses' " +
		  "target='_blank'>http://www.gnu.org/licenses</a>&gt;.");
		// Render list of used software and licenses
		me.html += me.createBox(me.renderSoftware() + "<br/>" +
		  me.renderLicenses());
		me.html += "<br/></form>";
		me.callParent(arguments);
	},

	createBox: function(msg) {
		return [ '<div class="x-box-aboutbox">', msg, '</div>' ].join('');
	},

	renderSoftware: function() {
		var html = "";
		var packages = [{
			name: "ExtJS",
			url: "http://www.sencha.com"
		},{
			name: "nginx",
			url: "http://www.nginx.net"
		},{
			name: "PHP",
			url: "http://www.php.net"
		},{
			name: "Python",
			url: "http://www.python.net"
		},{
			name: "xmlstarlet",
			url: "http://xmlstar.sourceforge.net"
		},{
			name: "Postfix",
			url: "http://www.postfix.org"
		},{
			name: "rsyslog",
			url: "http://www.rsyslog.com"
		},{
			name: "smartmontools",
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
			name: "Samba",
			url: "http://www.samba.org"
		},{
			name: "PHP JSON schema",
			url: "http://code.google.com/p/json-schema-php"
		},{
			name: "rsync",
			url: "http://rsync.samba.org"
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
			name: "collectd",
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
		},{
			name: "OpenSSH",
			url: "http://www.openssh.org"
		},{
			name: "ethtool",
			url: "http://sourceforge.net/projects/gkernel"
		},{
			name: "ntfs-3g",
			url: "http://www.tuxera.com"
		},{
			name: "mdadm",
			url: "http://neil.brown.name/blog/mdadm"
		},{
			name: "ntp",
			url: "http://support.ntp.org"
		},{
			name: "uuid",
			url: "http://www.ossp.org/pkg/lib/uuid"
		},{
			name: "wget",
			url: "http://www.gnu.org/software/wget"
		},{
			name: "acpid",
			url: "http://acpid.sourceforge.net"
		},{
			name: "quota",
			url: "http://sourceforge.net/projects/linuxquota"
		},{
			name: "Material Design Icons",
			url: "https://materialdesignicons.com"
		}];
		// Shuffle list of packages.
		packages.sort(function() {return 0.5 - Math.random()});
		// Display some packages.
		for(var i = 0; i < 25; i++) {
			var fontSizeMin = 9, fontSizeMax = 24;
			var fontSize = Math.floor((fontSizeMax - fontSizeMin + 1) *
			  Math.random()) + fontSizeMin;
			if(!Ext.isEmpty(html)) html += " ";
			html += Ext.String.format("<a href='{0}' target='_blank' " +
			  "style='font-size:{1}px;font-style:normal;'>{2}</a>",
			  packages[i].url, fontSize, packages[i].name);
		}
		return "openmediavault is based upon various free software " +
		  "like:<br/><br/><div id='wordcloud'>" + html + "...</div>";
	},

	renderLicenses: function() {
		var me = this;
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
			name: "Python License",
			url: "licenses/python.txt",
		},{
			name: "IBM Public License",
			url: "licenses/ibm-public-1_0.txt",
		},{
			name: "SIL Open Font License",
			url: "licenses/sil-1_1.txt",
		}]
		licenses.forEach(function(o) {
			if(!Ext.isEmpty(html)) html += ", ";
			html += Ext.String.format("<a href='{0}' target='_blank'>" +
			  "{1}</a>", o.url, o.name);
		}, me);
		return "Some of the software is licensed under " + html + ", ...";
	}
});

OMV.WorkspaceManager.registerNode({
	id: "about",
	path: "/info",
	text: _("About"),
	iconCls: "mdi mdi-information-outline",
	position: 30
});

OMV.WorkspaceManager.registerPanel({
	id: "about",
	path: "/info/about",
	text: _("About"),
	position: 10,
	className: "OMV.module.public.info.about.About"
});
