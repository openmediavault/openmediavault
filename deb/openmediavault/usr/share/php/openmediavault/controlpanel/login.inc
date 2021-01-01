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

class Login extends ControlPanelAbstract {
	public function getJavascriptIncludes() {
		$incList = [];
		$this->scanFiles("js/omv/window/Login.js", $incList);
		return $incList;
	}

	protected function getBodyClass() {
		return "login-page";
	}

	protected function getBodyContent() {
		$prd = new \OMV\ProductInfo();
		return <<<EOF
		<a title='{$prd->getName()}' href='{$prd->getURL()}' target='_blank'><div class="product-logo"></div></a>
		<script type="application/javascript">
			Ext.onReady(function() {
				Ext.tip.QuickTipManager.init(true, {
					dismissDelay: 0
				});
				Ext.create("OMV.window.Login", {
					listeners: {
						login: function(wnd, response) {
							// Close the window now.
							wnd.close();
							// Display loading progress dialog. This will
							// be displayed until the web administration
							// interface is loaded and rendered.
							Ext.getBody().mask(
								_("Loading, please wait ..."));
							// Reload page to render the web
							// administration interface.
							document.location.reload(true);
						}
					}
				}).show();
			});
		</script>
EOF;
	}
}
