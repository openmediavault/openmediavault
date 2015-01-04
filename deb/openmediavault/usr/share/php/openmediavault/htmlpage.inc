<?php
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
require_once("openmediavault/object.inc");
require_once("openmediavault/session.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/product.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/util.inc");

/**
 * Base class for a HTML page.
 * @ingroup api
 */
abstract class OMVHtmlPageAbstract extends OMVObject {
	/**
	 * Includes the given file
	 * @param fileName The name of the file to add
	 * @param incList The list where to add the files to include
	 */
	protected function scanFiles($fileName, &$incList) {
		// Add the given file to the list of files to be included. Note,
		// the file must be inserted at the beginning of the list to respect
		// the prerequisite order.
		array_unshift($incList, $fileName);
		$fh = fopen($fileName, "r");
		if ($fh === FALSE)
			throw new Exception(sprintf("Failed to open file '%s'", $fileName));
		$abort = FALSE;
		$scanList = array();
		$regex = "/^(\/\/\s+require\(\"([^\s]+)\"\)|\/\*+|\*\/|\*.*|\s*)$/";
		while (!feof($fh)) {
			// Read line by line. Strip whitespace or other characters from
			// the beginning and end of the line.
			$content = trim(fgets($fh));
			// Abort scanning this file if we find the first line that does
			// not match the regex. Accepted lines are:
			// - /**
			// - *
			// - * xyz abc
			// - */
			// - // require("xyz")
			// - Blank lines
			if (1 !== preg_match($regex, $content))
				break;
			if (1 !== preg_match("/\/\/\s+require\(\"([^\s]+)\"\)/", $content,
			  $matches))
				continue;
			$scanList[] = $matches[1];
		}
		fclose($fh);
		// Process all files that are included in the given file.
		foreach ($scanList as $fileName)
			$this->scanFiles($fileName, $incList);
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
					$this->scanFiles("{$dirPath}/{$pathInfo['basename']}",
					  $incList);
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
		$extAllFile = "ext-all.js";

		if (boolvalEx($GLOBALS['OMV_DEBUG_EXTJS']) === TRUE) {
			$extAllFile = "ext-all-debug.js";
		}

		echo <<<EOF
<!DOCTYPE html>
<html>
	<head>
		<title>{$title}</title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
		<meta name="ROBOTS" content="NOINDEX, NOFOLLOW"/>
		<meta name="viewport" content="width=device-width, height=device-height, initial-scale=1">
		<link rel="shortcut icon" type="image/x-icon" href="favicon.ico"/>
		<link rel="stylesheet" type="text/css" href="/extjs/resources/css/ext-all.css"/>
		<link rel="stylesheet" type="text/css" href="/extjs/resources/css/ext-all-gray.css"/>
		<link rel="stylesheet" type="text/css" href="css/omv.css"/>

EOF;

		// Add additional CSS files.
		$incList = array_values(array_unique($this->getCssIncludes()));
		$fileName = "css/omv-custom.css";
		if (file_exists($fileName))
			$incList[] = $fileName;
		foreach ($incList as $incListv) {
			print "<link rel='stylesheet' type='text/css' href='{$incListv}'/>\n";
		}

		// Add core javascript files.
echo <<<EOF
		<script type="application/javascript" src="/extjs/{$extAllFile}"></script>
		<script type="application/javascript" src="js/omv/util/i18n.js"></script>
		<script type="application/javascript" src="js/omv/util/i18nDict.js"></script>
		<script type="application/javascript" src="js/ext-overrides.js"></script>
		<script type="application/javascript" src="js/js-overrides.js"></script>
		<script type="application/javascript" src="js/omv/globals.js"></script>

EOF;

		// Get additional Javascript files. To improve loading speed the
		// list of include files will be cached.
		$parts = array();
		$parts[] = strtolower(get_class($this));
		$parts[] = $this->getCacheFileId();
		$parts[] = "js";
		$jsonFile = new OMVJsonFile(sprintf("%s/cache.%s.json",
		  $GLOBALS['OMV_CACHE_DIR'], implode("_", array_filter($parts))));
		if ($jsonFile->exists() && !(boolvalEx($GLOBALS['OMV_DEBUG_PHP']) ||
		  boolvalEx($GLOBALS['OMV_DEBUG_EXTJS']))) {
			$jsonFile->open("r");
			$incList = $jsonFile->read();
		} else {
			$incList = array_values(array_unique(
			  $this->getJavascriptIncludes()));
			$jsonFile->open("w+");
			$jsonFile->write($incList);
		}
		$jsonFile->close();
		// Append the additional Javascript files.
		foreach ($incList as $incListv) {
			print "<script type='application/javascript' src='{$incListv}'></script>\n";
		}

		// Get the body code to be rendered
		$bodyId = $this->getBodyId();
		$bodyContent = $this->getBodyContent();

		echo <<<EOF
	</head>
	<body id='{$bodyId}'>{$bodyContent}</body>
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
	 * Get the identifier to make the cache file unique, e.g. "xyz".
	 * Defaults to empty.
	 * @return The cache file identifier.
	 */
	protected function getCacheFileId() {
		return "";
	}

	/**
	 * Get the body ID.
	 */
	abstract protected function getBodyId();

	/**
	 * Get the HTML body code
	 */
	abstract protected function getBodyContent();

	/**
	 * Get an list of javascript files to be included.
	 * @return An array of files to be included.
	 */
	public function getJavascriptIncludes() {
		return array();
	}

	/**
	 * Get an list of CSS files to be included.
	 * @return An array of files to be included.
	 */
	public function getCssIncludes() {
		return array();
	}
}

/**
 * @ingroup api
 */
class OMVWebGui extends OMVHtmlPageAbstract {
	const MODE_ADMINISTRATOR = "admin";
	const MODE_USER = "user";

	protected $mode = null;

	/**
	 * Constructor
	 * @param mode The mode of the WebGUI, e.g. MODE_ADMINISTRATOR or
	 *   MODE_USER.
	 */
	public function __construct($mode = OMVWebGui::MODE_USER) {
		$this->setMode($mode);
	}

	/**
	 * Set the WebGUI mode.
	 * @param mode The mode of the WebGUI, e.g. MODE_ADMINISTRATOR or
	 *   MODE_USER.
	 */
	public function setMode($mode) {
		$this->mode = $mode;
	}

	public function getJavascriptIncludes() {
		$incList = array();
		$this->scanFiles("js/omv/SessionManager.js", $incList);
		$this->scanFiles("js/omv/workspace/Workspace.js", $incList);
		$this->scanDir("js/omv/module/public", $incList, TRUE);
		$this->scanDir("js/omv/module/{$this->mode}", $incList, TRUE);
		return $incList;
	}

	protected function getCacheFileId() {
		return $this->mode;
	}

	protected function getBodyId() {
		return "workspace-page";
	}

	protected function getBodyContent() {
		$prd = new OMVProduct();
		$session = &OMVSession::getInstance();
		// Define global constants.
		$globalDefs = sprintf(
		  "OMV.UUID_UNDEFINED = '%s'; ".
		  "OMV.ROLE_ADMINISTRATOR = %d; ".
		  "OMV.HTTPREQUEST_TIMEOUT = %s; ".
		  "OMV.PRODUCT_NAME = '%s'; ".
		  "OMV.PRODUCT_URL = '%s'; ".
		  "OMV.PRODUCT_COPYRIGHT = '%s'; ".
		  "OMV.PRODUCT_PACKAGENAME = '%s';",
		  $GLOBALS['OMV_UUID_UNDEFINED'], OMV_ROLE_ADMINISTRATOR,
		  $GLOBALS['OMV_HTTPREQUEST_TIMEOUT'], $prd->getName(),
		  $prd->getURL(), $prd->getCopyright(), $prd->getPackageName());
		$errorCodes = sprintf(
		  "OMV.E_SESSION_NOT_AUTHENTICATED = %d; ".
		  "OMV.E_SESSION_TIMEOUT = %d;".
		  "OMV.E_ENGINED_CONNECT_SOCKET = %d",
		  OMVErrorMsg::E_SESSION_NOT_AUTHENTICATED,
		  OMVErrorMsg::E_SESSION_TIMEOUT,
		  OMVErrorMsg::E_ENGINED_CONNECT_SOCKET);
		return <<<EOF
		<script type="application/javascript">
			// Global defines
			{$globalDefs}
			// RPC error codes
			{$errorCodes}
			// Request user confirmation before leaving page.
			OMV.confirmPageUnload = true;
			window.onbeforeunload = function() {
				if(true === OMV.confirmPageUnload) {
					OMV.confirmPageUnload = false;
					return _("Do you really want to leave the page?");
				}
			};
			// Initialize default session data.
			Ext.apply(OMV.SessionManager, {
				username: "{$session->getUsername()}",
				role: "{$session->getRole()}"
			});
			Ext.QuickTips.init();
			Ext.state.Manager.setProvider(new Ext.state.CookieProvider({
				expires: new Date(new Date().getTime() + (1000*60*60*24*365))
			}));
			Ext.onReady(function() {
				OMV.viewport = Ext.create("OMV.workspace.Workspace");
			});
		</script>
EOF;
	}
}

/**
 * @ingroup api
 */
class OMVWebGuiLogin extends OMVHtmlPageAbstract {
	public function getJavascriptIncludes() {
		$incList = array();
		$this->scanFiles("js/omv/window/Login.js", $incList);
		return $incList;
	}

	protected function getBodyId() {
		return "login-page";
	}

	protected function getBodyContent() {
		$prd = new OMVProduct();
		return <<<EOF
		<a title='{$prd->getName()}' href='{$prd->getURL()}' target='_blank'><div id="projectlogo"></div></a>
		<script type="application/javascript">
			Ext.QuickTips.init();
			Ext.onReady(function() {
				Ext.create("OMV.window.Login", {
					listeners: {
						login: function(wnd, response) {
							if(response.authenticated === true) {
								// Close the window now.
								wnd.close();
								// Display loading progress dialog. This will
								// be displayed until the web administration
								// interface is loaded and rendered.
								Ext.create("Ext.LoadMask", {
									target: Ext.getBody(),
									msg: _("Loading, please wait ...")
								}).show();
								// Reload page to render the web
								// administration interface.
								document.location.reload(true);
							} else {
								OMV.MessageBox.guru({
									msg: _("Incorrect username or password.")
								});
							}
						}
					}
				}).show();
			});
		</script>
EOF;
	}
}
