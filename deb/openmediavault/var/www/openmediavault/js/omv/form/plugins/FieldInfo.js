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

Ext.ns("OMV.form.plugins");

/**
 * Inspired by http://www.extjs.com/forum/showthread.php?p=375683#post375683
 * Example:
 * ... = new Ext.form.TextField({
 * 		xtype: "textfield",
 * 		name: "devicefile",
 * 		hiddenName: "devicefile",
 * 		hidden: true,
 * 		value: ""
 * 		plugins: [ OMV.form.plugins.FieldInfo ]
 * 		infoText: "xxxyyyzzz",
 * 		infoTextAlign: "top"
 * 	})
 */
OMV.form.plugins.FieldInfo = {
	init : function(container) {
		Ext.apply(container, {
			onRender: container.onRender.createSequence(function(ct, position) {
				if (Ext.isString(this.infoText)) {
					var element = this.getEl().up("div.x-form-element");
					if (!Ext.isEmpty(element)) {
						element[this.infoTextAlign == "top" ?
							"insertFirst" : "createChild"]({
							tag: "div",
							cls: this.infoTextClass || "x-form-fieldinfo",
							html: this.infoText
						});
					}
				}
			})
		});
	}
}
