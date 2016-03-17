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
namespace OMV\Config;

/**
 * @ingroup api
 */
class DatabaseBackendQueryBuilder {
	private $model = null;

	/**
	 * @param model The data model identifier or object.
	 */
	public function __construct($model) {
		if ($model instanceof DataModel) {
			$this->model = $model;
		} else {
			$mngr = \OMV\DataModel\Manager::getInstance();
			$this->model = $mngr->getModel($model);
		}
	}

	private function getModel() {
		return $this->model;
	}

	private function getQueryInfo()  {
		// The queryinfo information looks like:
		// "queryinfo": {
		// 	   "xpath": "//system/certificates/sslcertificate",
		//     "iterable": true,
		// 	   "idproperty": "uuid",
		// 	   "refproperty": "sslcertificateref"
		// }
		return $this->getModel()->getQueryInfo();
	}

	final public function buildGetQuery(...$args) {
		$qi = $this->getQueryInfo();
		$xpath = $qi['xpath'];
		if (TRUE === boolval($qi['iterable'])) {
			if (0 < count($args)) {
				// Get the element with the specified identifier.
				$xpath = sprintf("%s[%s='%s']", $qi['xpath'],
				  $qi['idproperty'], $args[0]);
			}
		}
		return $xpath;
	}

	final public function buildSetQuery(ConfigObject $object) {
		$qi = $this->getQueryInfo();
		$xpath = $qi['xpath'];
		if (TRUE === boolval($qi['iterable'])) {
			if (FALSE === $object->isNew()) {
				// Update the element with the specified identifier.
				$xpath = sprintf("%s[%s='%s']", $qi['xpath'],
				  $qi['idproperty'], $object->get($qi['idproperty']));
			} else {
				// Insert a new element.
				$parts = explode("/", $xpath);
				$elementName = array_pop($parts);
				$xpath = substr($xpath, 0, strrpos($xpath, $elementName));
			}
		}
		return $xpath;
	}

	final public function buildDeleteQuery(ConfigObject $object) {
		$qi = $this->getQueryInfo();
		$xpath = $qi['xpath'];
		if (TRUE === boolval($qi['iterable'])) {
			$xpath = sprintf("%s[%s='%s']", $qi['xpath'], $qi['idproperty'],
			  $object->get($qi['idproperty']));
		}
		return $xpath;
	}

	final public function buildIsReferencedQuery(ConfigObject $object) {
		$qi = $this->getQueryInfo();
		return sprintf("//%s[contains(.,'%s')]", $qi['xpath'],
		  $qi['refproperty'], $object->get($qi['idproperty']));
	}

	final public function buildExistsQuery($property, $value) {
		$qi = $this->getQueryInfo();
		return sprintf("//%s[contains('%s','%s')]", $qi['xpath'],
		  $property, $value);
	}

	final public function getValues(ConfigObject $object) {
		$qi = $this->getQueryInfo();
		$result = $object->getAssoc();
		if (TRUE === boolval($qi['iterable'])) {
			if (("uuid" == $qi['idproperty']) && (TRUE === $object->isNew())) {
				$parts = explode("/", $qi['xpath']);
				$elementName = array_pop($parts);
				$result = array($elementName => $result);
			}
		}
		return $result;
	}
}
