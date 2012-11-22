<?php
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
require_once("openmediavault/object.inc");
require_once("openmediavault/session.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/product.inc");
require_once("openmediavault/system.inc");

class OMVHtmlPage extends OMVObject {
	/**
	 * Includes the given file
	 * @param fileName The name of the file to add
	 * @param incList The list where to add the files to include
	 */
	protected function scanFiles($fileName, &$incList) {
		array_unshift($incList, $fileName);
		$fh = fopen($fileName, "r");
		if ($fh === FALSE) {
			throw new Exception("Failed to open file {$fileName}.");
		}
		for ($row = 0; $row < 100 && !feof($fh); ++$row) {
			$content = rtrim(fgets($fh));
			if (preg_match("/\/\/\s+require\(\"([^\s]+)\"\)/", $content,
				$matches)) {
				array_unshift($incList, $matches[1]);
				$this->scanFiles($matches[1], $incList);
			}
		}
		fclose($fh);
	}

	/**
	 * Scan the given directory for Javascript files to be added
	 * @param dirPath The name of the directory to scan
	 * @param incList The list where to add the files to include
	 * @param recursive Scan subdirs
	 */
	protected function scanDir($dirPath, &$incList, $recursive = TRUE) {
		$dir = new DirectoryIterator($dirPath);
		foreach ($dir as $item) {
			if ($item->isDot())
				continue;
			if ($item->isFile()) {
				$pathInfo = pathinfo($item->getFilename());
				if (array_key_exists("extension", $pathInfo) &&
				  (strtolower($pathInfo['extension']) === "js")) {
					$this->scanFiles("{$dirPath}/{$pathInfo['basename']}", $incList);
				}
			}
			if ($item->isDir() && (TRUE === $recursive)) {
				$newIncPath = $dirPath . "/" . $item->getFilename();
				$this->scanDir($item->getPathname(), $incList, $recursive);
				continue;
			}
		}
	}

	/**
	 * Renders the HTML page
	 */
	public function render() {
		$title = $this->getTitle();
		$extBaseFile = "ext-base.js";
		$extAllFile = "ext-all.js";

		if (boolval($GLOBALS['OMV_DEBUG_EXTJS']) === TRUE) {
			$extBaseFile = "ext-base-debug.js";
			$extAllFile = "ext-all-debug.js";
		}

		echo <<<EOF
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
	<head>
		<title>{$title}</title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<link rel="shortcut icon" type="image/x-icon" href="favicon.ico"/>
		<link rel="stylesheet" type="text/css" href="/extjs/resources/css/ext-all-notheme.css"/>
		<link rel="stylesheet" type="text/css" href="/extjs/resources/css/xtheme-gray.css"/>
		<link rel="stylesheet" type="text/css" href="css/omv-ext.css"/>
		<script type="text/javascript" src="/extjs/adapter/ext/{$extBaseFile}"></script>
		<script type="text/javascript" src="/extjs/{$extAllFile}"></script>
		<script type="text/javascript" src="js/omv/i18n.js"></script>
		<script type="text/javascript" src="js/omv/i18nDict.js"></script>
		<script type="text/javascript" src="js/ext-overrides.js"></script>
		<script type="text/javascript" src="js/js-overrides.js"></script>
		<script type="text/javascript" src="js/omv/globals.js"></script>

EOF;

		// Add additionbal javascript files
		$incList = array_unique($this->getJavascriptIncludes());
		foreach ($incList as $incListv) {
			print "<script type='text/javascript' src='{$incListv}'></script>\n";
		}

		// Get the body code to be rendered
		$body = $this->getBody();

		echo <<<EOF
	</head>
	<body>{$body}</body>
</html>
EOF;
	}

	/**
	 * Get the title of the HTML page.
	 * @return The page title string.
	 */
	protected function getTitle() {
		$prd = new OMVProduct();
		return sprintf("%s %s - %s", $prd->getName(),
		  gettext("web administration interface"),
		  OMVNetwork::getHostname("--fqdn"));
	}

	/**
	 * Get the HTML body code
	 */
	protected function getBody() {
		return "";
	}

	/**
	 * Get an list of javascript files to be included.
	 * @return An array of files to be included.
	 */
	protected function getJavascriptIncludes() {
		return array();
	}
}

class OMVWebGui extends OMVHtmlPage {
	const MODE_ADMINISTRATOR = "admin";
	const MODE_USER = "user";

	protected $mode = null;

	public function __construct($mode = OMVWebGui::MODE_USER) {
		$this->setMode($mode);
	}

	/**
	 * Set the WebGUI mode.
	 * @param mode The mode of the WebGUI.
	 */
	public function setMode($mode) {
		$this->mode = $mode;
	}

	protected function getJavascriptIncludes() {
		$incList = array();
		$this->scanFiles("js/omv/SessionMgr.js", $incList);
		$this->scanFiles("js/omv/Viewport.js", $incList);
		$this->scanFiles("js/omv/data/DataRequest.js", $incList);
		$this->scanDir("js/omv/module", $incList, FALSE);
		$this->scanDir("js/omv/module/{$this->mode}", $incList, TRUE);
		return $incList;
	}

	protected function getBody() {
		$prd = new OMVProduct();
		$session = &OMVSession::getInstance();
		// Define global constants.
		$globalDefs = sprintf("OMV.UUID_UNDEFINED = '%s'; ".
		  "OMV.HTTPREQUEST_TIMEOUT = %s; ".
		  "OMV.PRODUCT_NAME = '%s'; ".
		  "OMV.PRODUCT_URL = '%s'; ".
		  "OMV.PRODUCT_COPYRIGHT = '%s'; ".
		  "OMV.PRODUCT_PACKAGENAME = '%s';",
		  $GLOBALS['OMV_UUID_UNDEFINED'], $GLOBALS['OMV_HTTPREQUEST_TIMEOUT'],
		  $prd->getName(), $prd->getURL(), $prd->getCopyright(),
		  $prd->getPackageName());
		$errorCodes = sprintf("OMV.E_SESSION_NOT_AUTHENTICATED = %d; ".
		  "OMV.E_SESSION_TIMEOUT = %d;",
		  OMVErrorMsg::E_SESSION_NOT_AUTHENTICATED,
		  OMVErrorMsg::E_SESSION_TIMEOUT);
		return <<<EOF
		<script type="text/javascript">
			// Global defines
			{$globalDefs}
			// RPC error codes
			{$errorCodes}
			// Request user confirmation before leaving page
			OMV.confirmPageUnload = true;
			window.onbeforeunload = function() {
				if (true === OMV.confirmPageUnload) {
					OMV.confirmPageUnload = false;
					return _("Do you really want to leave the page?");
				}
			};
			// Initialize default session data
			Ext.apply(OMV.SessionMgr, {
				username: "{$session->getUsername()}"
			});
			Ext.QuickTips.init();
			Ext.state.Manager.setProvider(new Ext.state.CookieProvider({
				expires: new Date(new Date().getTime() + (1000*60*60*24*365))
			}));
			Ext.onReady(function() {
				OMV.viewport = new OMV.Viewport({
					listeners: {
						beforerender: function(c) {
							OMV.Download = new OMV.data.DataRequest();
							OMV.Download.init("download.php");
						},
						scope: this
					}
				});
			});
		</script>
EOF;
	}
}

class OMVWebGuiLogin extends OMVHtmlPage {
	protected function getJavascriptIncludes() {
		$incList = array();
		$this->scanFiles("js/omv/LoginDialog.js", $incList);
		return $incList;
	}

	protected function getBody() {
		$prd = new OMVProduct();
		return <<<EOF
		<link rel="stylesheet" type="text/css" href="css/omv.css"/>
		<a title='{$prd->getName()}' href='{$prd->getURL()}' target='_blank'><div id="projectlogo"></div></a>
		<script type="text/javascript">
			Ext.QuickTips.init();
			Ext.onReady(function() {
				new OMV.LoginDialog({
					listeners: {
						login: function(data) {
							if (data.authenticated === true) {
								// Reload page to render the web
								// administration interface
								document.location.reload(true);
							} else {
								OMV.MessageBox.failure(null,
								  "Incorrect username or password.");
							}
						}
					}
				}).show();
			});
		</script>
EOF;
	}
}
