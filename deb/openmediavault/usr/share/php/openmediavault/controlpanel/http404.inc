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

class Http404 extends ControlPanelAbstract {
	protected $prd = null;

	public function __construct() {
		$this->prd = new \OMV\ProductInfo();
	}

	public function getJavascriptIncludes() {
		$incList = [];
		$this->scanFiles("js/omv/window/MessageBox.js", $incList);
		return $incList;
	}

	protected function getTitle() {
		return sprintf("%s - %s", $this->prd->getName(), gettext("HTTP 404 error"));
	}

	protected function getBodyClass() {
		return "error-page";
	}

	protected function getBodyContent() {
		$title = sprintf("%s - %s", $this->prd->getName(), gettext("Page not found"));
		return <<<EOF
		<a title='{$this->prd->getName()}' href='{$this->prd->getURL()}' target='_blank'><div class="product-logo"></div></a>
		<script type="application/javascript">
			Ext.onReady(function() {
				OMV.MessageBox.guru({
					closable: false,
					msg: _("The requested page was not found.")
				});
			});
		</script>
EOF;
	}
}
