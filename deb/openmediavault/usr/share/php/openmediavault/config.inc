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
require_once("openmediavault/globals.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/error.inc");

/**
 * @class OMVConfigEngine
 * See http://www.zvon.org/xxl/XPathTutorial/General/examples.html for
 * some helpful XPath examples.
 */
class OMVConfigEngine extends OMVObject {
	private $domdoc = null;
	private $filePath = "";
	// These fields must be handled as arrays
	private $enums = array(
		// Usermanagement
		"user", "group",
		// Network interfaces
		"iface", "ifaceref", "bondiface", "bondifaceref", "bondslave",
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
		"job", "module", "sslcertificate"
	);

	/**
	 * Constructor
	 */
	public function __construct($filePath) {
		libxml_use_internal_errors(true);
		$this->filePath = $filePath;
	}

	/**
	 * Add an enum.
	 */
	public function addEnum($enum) {
		$this->enums[] = $enum;
	}

	/**
	 * Retrieve an error description.
	 * @return The error description if available, otherwise empty string.
	 */
	public function getError() {
		$text = "";
		$errLevels = array(
			LIBXML_ERR_WARNING => gettext("Warning"),
			LIBXML_ERR_ERROR => gettext("Error"),
			LIBXML_ERR_FATAL => gettext("Fatal error")
		);
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
	 * Load the configuration file.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function load() {
		$this->domdoc = new DOMDocument();
		return $this->domdoc->load($this->filePath, LIBXML_NOBLANKS |
		  LIBXML_NONET);
	}

	/**
	 * Save configuration file.
	 * @param error[out] The error message in case of a failure.
	 * @return TRUE on success, otherwise FALSE.
	 */
	public function save() {
		$this->domdoc->encoding = "UTF-8";
		$this->domdoc->formatOutput = true;
		return (FALSE === $this->domdoc->save($this->filePath,
		  LIBXML_NOEMPTYTAG)) ? FALSE : TRUE;
	}

	/**
	 * Get the configuration for the given XPath.
	 * @param xpath The path to the configuration object(s).
	 * @return The requested value or an array containing the requested value
	 * if the XPath matches multiple configuration objects. If XPath does not
	 * match NULL will be returned.
	 */
	public function get($xpath) {
		$domxpath = new DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		$result = NULL;
		foreach ($nodeList as $nodek => $nodev) {
			$data = $this->_nodeToArray($nodev);
			if ($nodeList->length == 1) {
				$result = $data;
			} else {
				if (!is_array($result)) $result = array();
				$result[] = $data;
			}
		}
		return $result;
	}

	/**
	 * Get the configuration for the given XPath as a list.
	 * @param xpath The path to the configuration object(s).
	 * @return An array containing the requested configuration objects,
	 * otherwise an empty array.
	 */
	public function getList($xpath) {
		$domxpath = new DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		$result = array();
		foreach ($nodeList as $nodek => $nodev) {
			$result[] = $this->_nodeToArray($nodev);
		}
		return $result;
	}

	/**
	 * Set the data at the given XPath.
	 * @param xpath The path to the configuration data
	 * @param data The data to set at the given XPath position
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function set($xpath, $data) {
		$domxpath = new DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		if (0 >= $nodeList->length)
			return FALSE;
		foreach ($nodeList as $node) {
			$this->_nodeFromArray($data, $node);
		}
		return TRUE;
	}

	/**
	 * Replace the data at the given XPath. If the given XPath does not
	 * exist the method will exit immediatelly.
	 * @param xpath The path to the configuration data
	 * @param data The data to set at the given XPath position
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function replace($xpath, $data) {
		$domxpath = new DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		if (0 >= $nodeList->length)
			return FALSE;
		foreach ($nodeList as $node) {
			$this->_deleteChildren($node);
			$this->_nodeFromArray($data, $node);
		}
		return TRUE;
	}

	/**
	 * Delete the nodes matching the given XPath expression.
	 * @param xpath The XPath expression to delete.
	 * @return Returns the deleted nodes, otherwise FALSE.
	 */
	public function delete($xpath) {
		if (FALSE === ($result = $this->get($xpath)))
			return FALSE;
		$domxpath = new DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		foreach ($nodeList as $node) {
			$this->_deleteNode($node);
		}
		return $result;
	}

	/**
	 * Check if the data at the given XPath exists.
	 * @param xpath The XPath expression to execute.
	 * @return TRUE if data exists, otherwise FALSE. If the XPath expression
	 * is malformed FALSE will be returned.
	 */
	public function exists($xpath) {
		if (FALSE === ($result = $this->count($xpath)))
			return FALSE;
		return (0 < $result);
	}

	/**
	 * Get the number of nodes matching the given XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @return Get the number of nodes matching the given XPath expression
	 * or FALSE on failure (e.g. XPath expression is malformed).
	 */
	public function count($xpath) {
		$domxpath = new DOMXPath($this->domdoc);
		$nodeList = $domxpath->query($xpath);
		if (FALSE === $nodeList)
			return FALSE;
		return $nodeList->length;
	}

	/**
	 * @private
	 */
	private function _deleteNode($node) {
		$this->_deleteChildren($node);
		$parent = $node->parentNode;
		$parent->removeChild($node);
	}

	/**
	 * @private
	 */
	private function _deleteChildren($node) {
		while (isset($node->firstChild)) {
			$this->_deleteChildren($node->firstChild);
			$node->removeChild($node->firstChild);
		}
	}

	/**
	 * @private
	 * Convert a PHP array to a DOMNode object.
	 */
	private function _nodeFromArray($mixed, DOMNode $domNode) {
		if (is_array($mixed)) {
			foreach ($mixed as $mixedk => $mixedv) {
				if (is_int($mixedk)) {
					if ($mixedk == 0) {
						$node = $domNode;
					} else {
						$node = $this->domdoc->createElement($domNode->tagName);
						$domNode->parentNode->appendChild($node);
					}
				} else {
					$node = $this->domdoc->createElement($mixedk);
					$domNode->appendChild($node);
				}
				$this->_nodeFromArray($mixedv, $node);
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
	private function _nodeToArray(DOMNode $domNode = null) {
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
						$childElem = $domNode->getElementsByTagName(
						  $childv->nodeName);
						$numElements = 0;
						foreach ($childElem as $childElemv) {
							if ($childElemv->parentNode->isSameNode(
							  $childv->parentNode)) {
								$numElements++;
							}
						}
						$value = $this->_nodeToArray($childv);
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
						$result = $this->_nodeToArray($childv);
					}
				}
			}
		}
		return $result;
	}
}

// Load the configuration file and save it in a global variable.
$xmlConfig = new OMVConfigEngine($GLOBALS['OMV_CONFIG_FILE']);
if (FALSE === $xmlConfig->load()) {
	throw new OMVException(OMVErrorMsg::E_CONFIG_LOAD_FAILED,
	  $xmlConfig->getError());
}
?>
