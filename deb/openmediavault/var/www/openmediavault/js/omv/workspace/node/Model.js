/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
// require("js/omv/workspace/node/Node.js")

/**
 * @ingroup webgui
 * @class OMV.workspace.node.Model
 * @derived Ext.data.Model
 */
Ext.define("OMV.workspace.node.Model", {
	extend: "Ext.data.Model",
	requires: [
		"OMV.workspace.node.Node"
	],

	idProperty: "uri",
	fields: [
		{ name: "id", type: "string" },
		{ name: "path", type: "string" },
		{ name: "className", type: "string" },
		{ name: "text", type: "string" },
		{ name: "position", type: "int" },
		{ name: "icon16", type: "string" },
		{ name: "icon32", type: "string" },
		{ name: "iconSvg", type: "string" },
		{ name: "leaf", type: "boolean" },
		{ name: "uri", type: "string", convert: function(v, rec) {
			return OMV.workspace.node.Node.buildUri([ rec.get("path"),
			  rec.get("id") ]);
		} }
	]
});
