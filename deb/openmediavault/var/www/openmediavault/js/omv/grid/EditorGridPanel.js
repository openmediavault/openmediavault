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

Ext.ns("OMV.grid");

/**
 * @class OMV.grid.EditorGridPanel
 * @derived Ext.grid.EditorGridPanel
 * Generic editable grid panel implementation.
 */
OMV.grid.EditorGridPanel = function(config) {
	var initialConfig = {
		loadMask: true,
		autoScroll: true,
		stripeRows: true,
		columnLines: true,
		layout: "fit",
		clicksToEdit: 1,
		stateful: true,
		viewConfig: {
			forceFit: true
		}
	};
	Ext.apply(initialConfig, config);
	OMV.grid.EditorGridPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.grid.EditorGridPanel, Ext.grid.EditorGridPanel, {
});
