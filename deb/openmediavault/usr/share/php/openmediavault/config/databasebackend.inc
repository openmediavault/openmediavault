<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
namespace OMV\Config;

require_once("openmediavault/functions.inc");

/**
 * See http://www.zvon.org/xxl/XPathTutorial/General/examples.html for
 * some helpful XPath examples.
 * @ingroup api
 */
class DatabaseBackend {
	use \OMV\DebugTrait;

	private $forceReload = TRUE;
	private $autoCommit = TRUE;
	private $versioning = FALSE;
	private $domdoc = NULL;
	private $filename = "";
	// These fields must be handled as arrays.
	private $enums = [
		// Usermanagement
		"user", "group", "sshpubkey",
		// Network interfaces
		"interface",
		// Iptables
		"rule",
		// Storage
		"filesystem", "hdparm",
		// fstab
		"mntent",
		// Filesystem
		"usrquota", "grpquota",
		// Shared folder
		"sharedfolder", "privilege", "share",
		// Miscellaneous
		"job", "module", "sslcertificate", "sshcertificate",
		"service", "device",
		// If none of the names already listed fits, then the following
		// generally used name should be used.
		"item"
	];

	/**
	 * Constructor
	 * @param filename The path to the XML document.
	 * @param forceReload Set to TRUE to reload the XML document everytime
	 *   before any action is performed on the XML document. Defaults to
	 *   TRUE.
	 * @param autoCommit Set to TRUE to automatically commit the XML document
	 *   modifications after write-access. Defaults to TRUE.
	 * @param versioning Set to TRUE to automatically create a revision file
	 *   for all changes. Files will look like <filename>.<revision>, e.g.
	 *   <filename>.0015. Defaults to FALSE.
	 */
	public function __construct($filename, $forceReload = TRUE,
	  $autoCommit = TRUE, $versioning = FALSE) {
	  	// Enable libxml errors to fetch error information as needed.
		libxml_use_internal_errors(true);
		// Enable the ability to load external entities.
		// See https://bugs.php.net/bug.php?id=64938 for more information.
		libxml_disable_entity_loader(false);
		// Initialize member variables.
		$this->domdoc = NULL;
		$this->forceReload = $forceReload;
		$this->autoCommit = $autoCommit;
		$this->versioning = $versioning;
		$this->filename = $filename;
	}

	/**
	 * Lock the database backend.
	 * @param boolean $auto Automatically acquire the lock and release
	 *   it on destruction. Defaults to TRUE.
	 * @return The mutex object.
	 */
	public function lock($auto = TRUE): \OMV\Mutex {
		return new \OMV\Mutex($this->filename, $auto);
	}

	/**
	 * Add an enum.
	 */
	public function addEnum($enum) {
		$this->enums[] = $enum;
	}

	/**
	 * Create a revision for all changes.
	 * @param enable  Set to TRUE to automatically create a revision file
	 *   for all changes. Files will be created like <filename>.<revision>,
	 *   e.g. <filename>.0015. Set to FALSE to disable versioning on
	 *   changes.
	 */
	public function setVersioning($enable) {
		$this->versioning = $enable;
	}

	/**
	 * Load the configuration file.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function load() {
		$this->domdoc = new \DOMDocument();
		if (FALSE === ($result = $this->domdoc->load($this->filename,
				LIBXML_NOBLANKS | LIBXML_NONET))) {
			throw new DatabaseException($this->getLibXmlError());
		}
		return $result;
	}

	/**
	 * Save configuration file. This function is protected by mutual exclusion,
	 * thus the function will be blocked until it can access the protected
	 * section.
	 * @return TRUE on success, otherwise FALSE.
	 */
	public function save() {
		$mutex = $this->lock();
		return $this->commit(TRUE);
	}

	/**
	 * Get the configuration for the given XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @return The requested value or an array containing the requested value
	 *   if the XPath expression matches multiple configuration objects. If
	 *   the XPath expression does not match NULL will be returned.
	 */
	public function get($xpath) {
		$this->reload();
		$domxpath = new \DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		$result = NULL;
		foreach ($nodeList as $nodek => $nodev) {
			$data = $this->nodeToArray($nodev);
			if ($nodeList->length == 1) {
				$result = $data;
			} else {
				if (!is_array($result)) $result = [];
				$result[] = $data;
			}
		}
		return $result;
	}

	/**
	 * Get the configuration for the given XPath expression as a list.
	 * @param xpath The XPath expression to execute.
	 * @return An array containing the requested configuration objects,
	 *   otherwise an empty array.
	 */
	public function getList($xpath) {
		$this->reload();
		$domxpath = new \DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		$result = [];
		foreach ($nodeList as $nodek => $nodev) {
			$result[] = $this->nodeToArray($nodev);
		}
		return $result;
	}

	/**
	 * Get the XPath location path of the objects addressed by the given
	 * XPath as a list.
	 * @param xpath The XPath expression to execute.
	 * @return An array containing the XPath to the objects, otherwise an
	 *   empty array.
	 */
	public function getXPathList($xpath) {
		$this->reload();
		$domxpath = new \DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		$result = [];
		foreach ($nodeList as $nodek => $nodev) {
			$result[] = $nodev->getNodePath();
		}
		return $result;
	}

	/**
	 * Set the data at the given XPath expression. This function is protected
	 * by mutual exclusion, thus the function will be blocked until it can
	 * acces the protected section.
	 * @param xpath The XPath expression to execute.
	 * @param data The data to set for the given XPath expression.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function set($xpath, $data) {
		$mutex = $this->lock();
		$this->reload();
		$domxpath = new \DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		if (0 >= $nodeList->length)
			return FALSE;
		foreach ($nodeList as $node) {
			$this->nodeFromArray($data, $node);
		}
		$this->commit();
		return TRUE;
	}

	/**
	 * Replace the data at the given XPath expression. If the given XPath
	 * expression does not exist, then the method will exit immediately.
	 * This function is protected by mutual exclusion, thus the function
	 * will be blocked until it can acces the protected section.
	 * @param xpath The XPath expression to execute.
	 * @param data The data to set for the given XPath expression.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function replace($xpath, $data) {
		$mutex = $this->lock();
		$this->reload();
		$domxpath = new \DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		if (0 >= $nodeList->length)
			return FALSE;
		foreach ($nodeList as $node) {
			$this->deleteChildren($node);
			$this->nodeFromArray($data, $node);
		}
		$this->commit();
		return TRUE;
	}

	/**
	 * Update the existing configuration for the given XPath expression.
	 * The given data will override the existing configuration. If the
	 * given XPath expression does not exist, then the method will exit
	 * immediately.
	 * This function is protected by mutual exclusion, thus the function
	 * will be blocked until it can acces the protected section.
	 * @param xpath The XPath expression to execute.
	 * @param data The data to set for the given XPath expression.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function update($xpath, $data) {
		$mutex = $this->lock();
		// Get the configuration for the given XPath expression.
		$currentData = $this->get($xpath);
		if (is_null($currentData))
			return FALSE;
		// Merge the configuration.
		$data = array_merge($currentData, $data);
		// Replace the old configuration with the merged one.
		$domxpath = new \DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		if (0 >= $nodeList->length)
			return FALSE;
		foreach ($nodeList as $node) {
			$this->deleteChildren($node);
			$this->nodeFromArray($data, $node);
		}
		$this->commit();
		return TRUE;
	}

	/**
	 * Delete the nodes matching the given XPath expression. This function is
	 * protected by mutual exclusion, thus the function will be blocked until
	 * it can acces the protected section.
	 * @param xpath The XPath expression to execute.
	 * @return Returns the deleted nodes, otherwise FALSE.
	 */
	public function delete($xpath) {
		$mutex = $this->lock();
		if (is_null($result = $this->get($xpath))) {
			return FALSE;
		}
		$domxpath = new \DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		foreach ($nodeList as $node) {
			$this->deleteNode($node);
		}
		$this->commit();
		return $result;
	}

	/**
	 * Check if the data at the given XPath expression exists.
	 * @param xpath The XPath expression to execute.
	 * @return TRUE if data exists, otherwise FALSE. If the XPath expression
	 *   is malformed FALSE will be returned, too.
	 */
	public function exists($xpath) {
		$this->reload();
		if (FALSE === ($result = $this->count($xpath)))
			return FALSE;
		return (0 < $result);
	}

	/**
	 * Get the number of nodes matching the given XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @return Get the number of nodes matching the given XPath expression
	 *   or FALSE on failure (e.g. XPath expression is malformed).
	 */
	public function count($xpath) {
		$this->reload();
		$domxpath = new \DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		if (FALSE === $nodeList)
			return FALSE;
		return $nodeList->length;
	}

	/**
	 * Compare the configuration from the given XPath expression with the
	 * given data.
	 * @param xpath The XPath expression to execute.
	 * @param data The data to compare.
	 * @return Returns 0 if the data is equal, otherwise -1. On error
	 *   boolean FALSE is returned.
	 */
	public function compare($xpath, $data) {
		if (is_null($result = $this->get($xpath)))
			return FALSE;
		// Convert everything into an array.
		if (!is_array($result))
			$result = array($result);
		if (!is_array($data))
			$result = array($data);
		// Convert all values to strings before comparison. This is necessary
		// because the values returned from the configuration database are
		// strings.
		if (FALSE === array_walk_recursive($data, function(&$item, $key) {
			if (is_string($item)) {
				if (!mb_check_encoding($item, "UTF-8")) {
					$item = utf8_encode($item);
				}
			} else {
				$item = utf8_encode(strval($item));
			}
		})) {
			return FALSE;
		}
		// Compare the arrays.
		// Note, we can not use `array_diff` here because this function
		// does not support multidimensional arrays.
		if (0 === strcmp(json_encode_safe($result),
				json_encode_safe($data))) {
			return 0;
		}
		return -1;
	}

	/**
	 * Revert changes. All existing revision files will be deleted.
	 * @param filename The revision file. Defaults to NONE.
	 * @note This only takes action if versioning is enabled.
	 * @return void
	 */
	public function revert($filename) {
		if (TRUE !== $this->versioning)
			throw new DatabaseException("Versioning is not enabled.");
		$mutex = $this->lock();
		if (empty($filename)) {
			// Determine the lowest revision number.
			$revision = PHP_INT_MAX;
			foreach (new \DirectoryIterator(dirname($this->filename)) as $item) {
				if (!$item->isFile())
					continue;
				// Identify files like '<filename>.<revision>'.
				$pathInfo = pathinfo($item->getFilename());
				if (0 !== strcasecmp($pathInfo['filename'], basename(
				  $this->filename)))
					continue;
				if (!is_numeric($pathInfo['extension']))
					continue;
				$value = intval($pathInfo['extension']);
				if ($value < $revision) {
					$revision = $value;
					$filename = $item->getPathname();
				}
			}
			// Any revision file found?
			if (!file_exists($filename))
				return;
		}
		// Replace the configuration file with the given revision.
		// !!! Note, the file permissions of the target file will
		// not be touched when using the copy operation.
		if (!copy($filename, $this->filename)) {
			throw new DatabaseException(last_error_msg());
		}
		// Unlink all revision files.
		$this->unlinkRevisions();
	}

	/**
	 * Unlink all revision files. Note, this only takes action if versioning
	 * is enabled.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function unlinkRevisions() {
		if (TRUE !== $this->versioning)
			return FALSE;
		$pattern = sprintf("%s.*", $this->filename);
		array_map('unlink', glob($pattern));
		return TRUE;
	}

	/**
	 * Commit the changes of the internal XML document to the given
	 * configuration file. A revision file will be created if versioning
	 * is enabled.
	 * @private
	 * @param force Set to TRUE to force commit.
	 * @return Returns TRUE on success, otherwise FALSE.
	 */
	private function commit($force = FALSE) {
		if (!((TRUE === $force) || (TRUE === $this->autoCommit)))
			return TRUE;
		// Create a revision of the XML document.
		if (TRUE === $this->versioning) {
			// Determine the current revision number.
			$revision = 0;
			foreach (new \DirectoryIterator(dirname($this->filename)) as $item) {
				if (!$item->isFile())
					continue;
				// Identify files like '<filename>.<revision>'.
				$pathInfo = pathinfo($item->getFilename());
				if (0 !== strcasecmp($pathInfo['filename'], basename(
				  $this->filename)))
					continue;
				if (!is_numeric($pathInfo['extension']))
					continue;
				$value = intval($pathInfo['extension']);
				if ($value > $revision)
					$revision = $value;
			}
			// Calculate the next revision number.
			$revision += 1;
			// Build the new filename, e.g. '<filename>.0815'.
			$filename = sprintf("%s.%04d", $this->filename, $revision);
			// Create a copy of the existing document. Exit immediately
			// on failure to do not corrupt the original file. Note, the
			// current changes can get lost.
			if (!copy($this->filename, $filename)) {
				throw new DatabaseException(last_error_msg());
			}
			// Update file permissions.
			chmod($filename, 0600);
		}
		// Save document.
		$this->domdoc->encoding = "UTF-8";
		$this->domdoc->formatOutput = true;
		if (FALSE === ($result = $this->domdoc->save($this->filename,
				LIBXML_NOEMPTYTAG))) {
			throw new DatabaseException($this->getLibXmlError());
		}
		return $result;
	}

	/**
	 * Retrieve an error description.
	 * @private
	 * @return The error description if available, otherwise empty string.
	 */
	private function getLibXmlError() {
		$text = "";
		$errLevels = [
			LIBXML_ERR_WARNING => gettext("Warning"),
			LIBXML_ERR_ERROR => gettext("Error"),
			LIBXML_ERR_FATAL => gettext("Fatal error")
		];
		foreach (libxml_get_errors() as $errork => $errorv) {
			if ($errork > 0) $text .= "; ";
			$text .= sprintf("%s %d: %s (line=%d, column=%d)",
			  $errLevels[$errorv->level], $errorv->code,
			  trim($errorv->message), $errorv->line, $errorv->column);
		}
		libxml_clear_errors();
		return $text;
	}

	/**
	 * @private
	 */
	private function reload() {
		if (TRUE === $this->forceReload) {
			$this->domdoc = NULL;
			return $this->load();
		}
		return TRUE;
	}

	/**
	 * @private
	 */
	private function deleteNode($node) {
		$this->deleteChildren($node);
		$parent = $node->parentNode;
		$parent->removeChild($node);
	}

	/**
	 * @private
	 */
	private function deleteChildren($node) {
		while (isset($node->firstChild)) {
			$this->deleteChildren($node->firstChild);
			$node->removeChild($node->firstChild);
		}
	}

	/**
	 * @private
	 * Convert a PHP array to a DOMNode object.
	 */
	private function nodeFromArray($mixed, \DOMNode $domNode) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Is it an array/dictionary?
		if (is_array($mixed)) {
			foreach ($mixed as $mixedk => $mixedv) {
				// Check if we are processing an array.
				if (is_int($mixedk)) { // Array
					if ($mixedk == 0) {
						$node = $domNode;
					} else {
						$node = $this->domdoc->createElement($domNode->tagName);
						$domNode->parentNode->appendChild($node);
					}
				} else { // Dictionary
					// Skip enumerable elements that are empty.
					if (in_array($mixedk, $this->enums) && empty($mixedv))
						continue;
					$node = $this->domdoc->createElement($mixedk);
					$domNode->appendChild($node);
				}
				$this->nodeFromArray($mixedv, $node);
			}
		} else {
			$value = $mixed;
			// Convert boolean values: true => 1, false => 0
			if (is_bool($mixed)) $value = ($mixed) ? 1 : 0;
			$domNode->appendChild($this->domdoc->createTextNode($value));
		}
	}

	/**
	 * @private
	 * Convert a DOMNode object to a PHP array.
	 */
	private function nodeToArray(\DOMNode $domNode = null) {
		$result = "";
		if (is_null($domNode) && !$this->hasChildNodes()) {
			return $result;
		}
		if ($domNode->nodeType == XML_TEXT_NODE) {
			$result = $domNode->nodeValue;
		} else {
			if ($domNode->hasChildNodes()){
				foreach ($domNode->childNodes as $childv) {
					if ($childv->nodeName == '#comment')
						continue;
 					if ($childv->nodeName !== '#text') {
						if (!is_array($result)) $result = [];
						$childElem = $domNode->getElementsByTagName(
						  $childv->nodeName);
						$numElements = 0;
						foreach ($childElem as $childElemv) {
							if ($childElemv->parentNode->isSameNode(
							  $childv->parentNode)) {
								$numElements++;
							}
						}
						$value = $this->nodeToArray($childv);
						$key = $childv->nodeName;
						if (in_array($key, $this->enums)) $numElements++;
						if ($numElements > 1) {
							$result[$key][] = $value;
						}
						else {
							$result[$key] = $value;
						}
					}
					else if ($childv->nodeName == '#text') {
						$result = $this->nodeToArray($childv);
					}
				}
			}
		}
		return $result;
	}
}
