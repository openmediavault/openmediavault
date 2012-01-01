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

Ext.ns("OMV.data");

OMV.data.DataRequest = function() {
};
OMV.data.DataRequest.prototype = {
	init : function(url) {
		this.url = url;
		this.method = "post";
	},

	request : function(service, method, params) {
		var body = Ext.getBody();
		var form = body.createChild({
			tag: "form",
			cls: "x-hidden",
			action: this.url,
			method: this.method
		});
		form.createChild({
			tag: "input",
			name: "service",
			value: service
		});
		form.createChild({
			tag: "input",
			name: "method",
			value: method
		});
		// Additional parameters must be encoded as string because they are
		// submitted as POST parameters. They will be decoded by the RPC
		// implementation automatically.
		form.createChild({
			tag: "input",
			name: "params",
			value: Ext.util.JSON.encode(params).htmlspecialchars()
		});
		form.dom.submit();
	}
}

OMV.Download = new OMV.data.DataRequest();
OMV.Download.init("download.php");
