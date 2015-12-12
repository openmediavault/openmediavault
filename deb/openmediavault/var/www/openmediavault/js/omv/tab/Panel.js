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

/**
 * @ingroup webgui
 * @class OMV.tab.Panel
 * @derived Ext.tab.Panel
 * Enhanced tab panel implementation.
 */
Ext.define("OMV.tab.Panel", {
	extend: "Ext.tab.Panel",

	/**
	 * Mark the given card as invalid.
	 * @param card The card to mark invalid. Either an ID, index or the
	 *   component itself.
	 * @param bubble TRUE to bubble up and mark all above cards as invalid.
	 *   Defaults to TRUE.
	 */
	markInvalidTab: function(card, bubble) {
		var me = this;
		bubble = Ext.isDefined(bubble) ? bubble : true;
		card = me.getComponent(card);
		if(card) {
			card.bubble(function(c) {
				if(Ext.isDefined(c.tab)) {
					c.tab.addCls("x-tab-invalid");
					if(!bubble)
						return false;
				}
			});
		}
	},

	/**
	 * Clears the invalid style in this card.
	 * @param card The card to be processed. Either an ID, index or the
	 *   component itself.
	 * @param bubble TRUE to bubble up and mark all above cards as invalid.
	 *   Defaults to TRUE.
	 */
	clearInvalidTab: function(card, bubble) {
		var me = this;
		bubble = Ext.isDefined(bubble) ? bubble : true;
		card = me.getComponent(card);
		if(card) {
			card.bubble(function(c) {
				if(Ext.isDefined(c.tab)) {
					c.tab.removeCls("x-tab-invalid");
					if(!bubble)
						return false;
				}
			});
		}
	}
});
