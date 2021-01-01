<?php
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
namespace OMV\ControlPanel;

class Administration extends ControlPanelAbstract {
	const MODE_ADMINISTRATOR = "admin";
	const MODE_USER = "user";

	protected $mode = null;

	/**
	 * Constructor
	 * @param mode The mode of the WebGUI, e.g. MODE_ADMINISTRATOR or
	 *   MODE_USER.
	 */
	public function __construct($mode = Administration::MODE_USER) {
		$this->setMode($mode);
	}

	/**
	 * Set the control panel mode.
	 * @param mode The mode of the WebGUI, e.g. MODE_ADMINISTRATOR or
	 *   MODE_USER.
	 */
	public function setMode($mode) {
		$this->mode = $mode;
	}

	public function getJavascriptIncludes() {
		$incList = [];
		$this->scanFiles("js/omv/service/SystemInfo.js", $incList);
		$this->scanFiles("js/omv/SessionManager.js", $incList);
		$this->scanFiles("js/omv/workspace/Workspace.js", $incList);
		$this->scanDir("js/omv/module/public", $incList, TRUE);
		$this->scanDir("js/omv/module/{$this->mode}", $incList, TRUE);
		return $incList;
	}

	protected function getCacheFileId() {
		return $this->mode;
	}

	protected function getBodyClass() {
		return "workspace-page";
	}

	protected function getBodyContent() {
		$session = &\OMV\Session::getInstance();
		return <<<EOF
		<script type="application/javascript">
			// Request user confirmation before leaving page.
			OMV.confirmPageUnload = true;
			window.onbeforeunload = function() {
				if (true === OMV.confirmPageUnload) {
					OMV.confirmPageUnload = false;
					return _("Do you really want to leave the page?");
				}
			};
			// Initialize default session data.
			Ext.apply(OMV.SessionManager, {
				username: "{$session->getUsername()}",
				role: "{$session->getRole()}"
			});
			// Use LocalStorage API if available, otherwise use
			// cookies to store the state of the UI components.
			var provider;
			if (Ext.util.LocalStorage.supported) {
				provider = Ext.create("Ext.state.LocalStorageProvider");
			} else {
				provider = Ext.create("Ext.state.CookieProvider", {
					expires: new Date(new Date().getTime() +
					  (1000*60*60*24*365))
				});
			}
			Ext.state.Manager.setProvider(provider);
			// Create the viewport if the page is ready.
			Ext.onReady(function() {
				Ext.tip.QuickTipManager.init(true, {
					dismissDelay: 0
				});
				OMV.viewport = Ext.create("OMV.workspace.Workspace");
			});
		</script>
EOF;
	}
}
