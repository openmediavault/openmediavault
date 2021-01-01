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

/**
 * @ingroup webgui
 * @class OMV.form.SshPublicKey
 * @derived Ext.form.field.Text
 */
Ext.define("OMV.form.field.SshPublicKey", {
	extend: "Ext.form.field.Text",
	alias: [ "widget.sshpublickeyfield" ],

	editable: false,
	vtype: "sshPubKeyOpenSSH",
	triggers: {
		copy: {
			cls: Ext.baseCSSPrefix + "form-copy-trigger",
			hideOnReadOnly: false,
			onClick: function() {
				var me = this;
				var handler = me.handler;
				var field = me.field;
				if (handler && me.isFieldEnabled()) {
					Ext.callback(me.handler, me.scope, [ field, me ],
					  0, field);
				}
			},
			handler: "onTrigger1Click"
		}
	},

	initComponent: function() {
		var me = this;
		me.callParent(arguments);
		me.on("afterrender", function() {
			// Add quick tip to the trigger button.
			var trigger = this.getTrigger("copy");
			Ext.tip.QuickTipManager.register({
				target: trigger.getEl(),
				text: _("Copy to clipboard")
			});
		}, me);
		me.on("beforedestroy", function() {
			// Remove the quick tip from the trigger button.
			var trigger = this.getTrigger("copy");
			Ext.tip.QuickTipManager.unregister(trigger.getEl());
			// Destroy hidden element.
			Ext.destroy(me.hiddenEl);
		}, me);
	},

	getHiddenEl: function () {
		var me = this;
		return me.hiddenEl || (me.hiddenEl = Ext.getBody().createChild({
			tag: "textarea",
			tabIndex: -1, // Don't tab through this hidden field.
			style: {
				position: "absolute",
				top: "-1000px",
				width: "1px",
				height: "1px"
			}
		}));
	},

	onTrigger1Click: function(c) {
		var value = c.getValue();
		if (window.clipboardData && window.clipboardData.setData) {
			window.clipboardData.setData("text", value);
		} else {
			var el = c.getHiddenEl().dom;
			var focusEl = Ext.Element.getActiveElement();
			el.value = value;
			el.focus();
			el.select();
			document.execCommand("copy")
			Ext.defer(function () {
				el.value = "";
				if (focusEl) {
					focusEl.focus();
				}
			}, 50);
		}
	}
});
