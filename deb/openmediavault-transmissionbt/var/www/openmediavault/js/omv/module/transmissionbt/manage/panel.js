/**
 * Created by JetBrains PhpStorm.
 * User: mbeck
 * Date: 29.11.11
 * Time: 00:03
 * To change this template use File | Settings | File Templates.
 */

// require("js/omv/NavigationPanel.js")

// require("js/omv/module/transmissionbt/NavigationPanel.js")

// require("js/omv/module/transmissionbt/manage/grid/List.js")

Ext.ns("OMV.Module.Services.TransmissionBT.Manage");

/**
 * @class OMV.Module.Services.TransmissionBT.Manage.TabPanel
 * @derived Ext.TabPanel
 */
OMV.Module.Services.TransmissionBT.Manage.TabPanel = function(config) {
	var initialConfig = {
		border: false,
		activeTab: 0,
		layoutOnTabChange: true
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.TransmissionBT.Manage.TabPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Services.TransmissionBT.Manage.TabPanel, Ext.TabPanel, {
	initComponent : function() {
		this.items = [
			new OMV.Module.Services.TransmissionBT.Manage.TorrentListGrid
		];
		OMV.Module.Services.TransmissionBT.Manage.TabPanel.superclass.initComponent.apply(
		  this, arguments);
	}
});
OMV.NavigationPanelMgr.registerPanel("services", "transmissionbt", {
	cls: OMV.Module.Services.TransmissionBT.Manage.TabPanel,
	title: "Jobs",
	position: 10
});