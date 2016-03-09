<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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

require_once("openmediavault/functions.inc");

/**
 * @ingroup api
 */
abstract class ControlPanelAbstract {
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
			throw new Exception("Failed to open file '%s'", $fileName);
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
			if (1 !== preg_match("/\/\/\s+require\(\"([^\s]+)\"\)/",
			  $content, $matches))
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
		$dir = new \DirectoryIterator($dirPath);
		foreach ($dir as $item) {
			if ($item->isDot())
				continue;
			if ($item->isFile()) {
				if ("js" !== strtolower($item->getExtension()))
					continue;
				$this->scanFiles($item->getPathname(), $incList);
			}
			if ($item->isDir() && (TRUE === $recursive)) {
				$this->scanDir($item->getPathname(), $incList, $recursive);
			}
		}
	}

	/**
	 * Renders the HTML page
	 */
	public function render() {
		$title = $this->getTitle();

		echo <<<EOF
<!DOCTYPE html>
<html>
	<head>
		<title>{$title}</title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
		<meta name="ROBOTS" content="NOINDEX, NOFOLLOW"/>
		<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
		<link rel="shortcut icon" type="image/x-icon" href="favicon.ico"/>

EOF;

		// Set core CSS files.
		$cssList = array(
			"/extjs/packages/ext-theme-gray/resources/ext-theme-gray-all.css",
			"css/omv.css"
		);
		// Add additional CSS files.
		$cssList = array_merge($cssList, array_values(array_unique(
		  $this->getCssIncludes())));
		$fileName = "css/omv-custom.css";
		if (file_exists($fileName))
			$cssList[] = $fileName;
		foreach ($cssList as $cssListv) {
			print "<link rel='stylesheet' type='text/css' href='{$cssListv}'/>\n";
		}

		// Add core javascript files.
		$jsList = array(
			"js/omv/util/i18n.js",
			"js/omv/util/i18nDict.js",
			"js/ext-overrides.js",
			"js/js-overrides.js",
			"js/omv/globals.js"
		);
		if (TRUE === boolvalEx($GLOBALS['OMV_DEBUG_EXTJS'])) {
			$jsList = array_merge(array(
				"/extjs/ext-all-debug.js",
				"/extjs/packages/ext-theme-gray/ext-theme-gray-debug.js"
			), $jsList);
		} else {
			$jsList = array_merge(array(
				"/extjs/ext-all.js",
				"/extjs/packages/ext-theme-gray/ext-theme-gray.js"
			), $jsList);
		}
		foreach ($jsList as $jsListv) {
			print "<script type='application/javascript' src='{$jsListv}'></script>\n";
		}

		// Get additional Javascript files. To improve loading speed the
		// list of include files will be cached.
		$jsList = array();
		$parts = array();
		$parts[] = strtolower(get_class($this));
		$parts[] = $this->getCacheFileId();
		$parts[] = "js";
		$jsonFile = new \OMV\Json\File(sprintf("%s/cache.%s.json",
		  $GLOBALS['OMV_CACHE_DIR'], implode("_", array_filter($parts))));
		if ($jsonFile->exists() && !(boolvalEx($GLOBALS['OMV_DEBUG_PHP']) ||
		  boolvalEx($GLOBALS['OMV_DEBUG_EXTJS']))) {
			$jsonFile->open("r");
			$jsList = $jsonFile->read();
		} else {
			$jsList = array_values(array_unique(
			  $this->getJavascriptIncludes()));
			$jsonFile->open("w+");
			$jsonFile->write($jsList);
		}
		$jsonFile->close();
		// Append the additional Javascript files.
		if (is_array($jsList)) {
			foreach ($jsList as $jsListv) {
				print "<script type='application/javascript' src='{$jsListv}'></script>\n";
			}
		}

		// Get the body code to be rendered
		$bodyClass = $this->getBodyClass();
		$bodyContent = $this->getBodyContent();

		echo <<<EOF
	</head>
	<body class='{$bodyClass}'>
{$bodyContent}
	</body>
</html>
EOF;
	}

	/**
	 * Get the title of the HTML page.
	 * @return The page title string.
	 */
	protected function getTitle() {
		$prd = new \OMV\ProductInfo();
		return sprintf("%s %s - %s", $prd->getName(), gettext("control panel"),
		  \OMV\System\Network\Network::getHostname("--fqdn"));
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
	 * Get the body class.
	 */
	abstract protected function getBodyClass();

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
