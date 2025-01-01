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

	final protected function buildQuery($predicate) {
		$qi = $this->getQueryInfo();
		return sprintf("%s[%s]", $qi['xpath'], $predicate);
	}

	/**
	 * Build the predicate for the specified filter.
	 * Supported operators:
	 * .-------------------------------------------------.
	 * | operator         | arg0          | arg1         |
	 * |------------------|---------------|--------------|
	 * | and              | assoc. array  | assoc. array |
	 * | or               | assoc. array  | assoc. array |
	 * | equals           | property name | value        |
	 * | notEquals        | property name | value        |
	 * | enum             | property name | array        |
	 * | stringEquals     | property name | value        |
	 * | stringNotEquals  | property name | value        |
	 * | stringContains   | property name | value        |
	 * | stringStartsWith | property name | value        |
	 * | stringEnum       | property name | array        |
	 * | not              | assoc. array  |              |
	 * | less             | property name | value        |
	 * | greater          | property name | value        |
	 * | lessEqual        | property name | value        |
	 * | greaterEqual     | property name | value        |
	 * | distinct         | property name |              |
	 * '-------------------------------------------------'
	 * Example 1:
	 * [type='bond' and devicename='bond0']
	 * The filter for the above predicate:
	 * [
	 *     "operator": "and",
	 *     "arg0": [
	 *         "operator" => "stringEquals",
	 *         "arg0" => "type",
	 *         "arg1" => "bond"
	 *     ],
	 *     "arg1": [
	 *         "operator" => "stringEquals",
	 *         "arg0" => "devicename",
	 *         "arg1" => "bond0"
	 *     ]
	 * ]
	 * Example 2:
	 * [type='bond' and contains(slaves,'eth0')]
	 * The filter for the above predicate:
	 * [
	 *     "operator": "and",
	 *     "arg0": [
	 *         "operator" => "stringEquals",
	 *         "arg0" => "type",
	 *         "arg1" => "bond"
	 *     ],
	 *     "arg1": [
	 *         "operator" => "stringContains",
	 *         "arg0" => "slaves",
	 *         "arg1" => "eth0"
	 *     ]
	 * ]
	 * Example 3:
	 * [not type='vlan']
	 * The filter for the above predicate:
	 * [
	 *     "operator": "not",
	 *     "arg0": [
	 *         "operator" => "stringEquals",
	 *         "arg0" => "type",
	 *         "arg1" => "vlan"
	 *     ]
	 * ]
	 * Example 4:
	 * [
	 *     "operator" => "and",
	 *     "arg0" => [
	 * 	       "operator" => "stringNotEquals",
	 *         "arg0" => "uuid",
	 *         "arg1" => $object->get("uuid")
	 *     ],
	 *     "arg1" => [
	 *         "operator" => "and",
	 *         "arg0" => [
	 *             "operator" => "stringEquals",
	 *             "arg0" => "mntentref",
	 *             "arg1" => $object->get("mntentref")
	 *         ],
	 *         "arg1" => [
	 *             "operator" => "stringEquals",
	 *             "arg0" => "reldirpath",
	 *             "arg1" => $object->get("reldirpath")
	 *         ]
	 *     ]
	 * ]
	 * @param filter A filter specifying constraints to build the
	 *   database query.
	 * @return The database query string.
	 * @throw \InvalidArgumentException
	 */
	final public function buildPredicate($filter) {
		if (FALSE === is_array($filter)) {
			throw new \InvalidArgumentException(
			  "The filter is not an associative array.");
		}
		if (TRUE === empty($filter)) {
			throw new \InvalidArgumentException(
			  "The filter must not be empty.");
		}
		if (FALSE === array_key_exists("operator", $filter)) {
			throw new \InvalidArgumentException(
			  "The filter is invalid. The 'operator' field is missing.");
		}
		$result = "";
		switch ($filter['operator']) {
		case "and":
		case "or":
			$result = sprintf("(%s %s %s)",
			  $this->buildPredicate($filter['arg0']),
			  $filter['operator'],
			  $this->buildPredicate($filter['arg1']));
			break;
		case "=":
		case "equals":
			$result = sprintf("%s=%s", $filter['arg0'], $filter['arg1']);
			break;
		case "!=":
		case "notEquals":
			$result = sprintf("%s!=%s", $filter['arg0'], $filter['arg1']);
			break;
		case "enum":
			$parts = [];
			foreach ($filter['arg1'] as $enumk => $enumv) {
				$parts[] = sprintf("%s=%s", $filter['arg0'], $enumv);
			}
			$result = sprintf("(%s)", implode(" or ", $parts));
			break;
		case "==":
		case "stringEquals":
			$result = sprintf("%s=%s", $filter['arg0'],
			  escapeshellarg($filter['arg1']));
			break;
		case "!==":
		case "stringNotEquals":
			$result = sprintf("%s!=%s", $filter['arg0'],
			  escapeshellarg($filter['arg1']));
			break;
		case "stringContains":
			$result = sprintf("contains(%s,%s)", $filter['arg0'],
			  escapeshellarg($filter['arg1']));
			break;
		case "stringStartsWith":
			$result = sprintf("starts-with(%s,%s)", $filter['arg0'],
			  escapeshellarg($filter['arg1']));
			break;
		case "stringEnum":
			$parts = [];
			foreach ($filter['arg1'] as $enumk => $enumv) {
				$parts[] = sprintf("%s=%s", $filter['arg0'],
				  escapeshellarg($enumv));
			}
			$result = sprintf("(%s)", implode(" or ", $parts));
			break;
		case "!":
		case "not":
			$result = sprintf("not(%s)",
			  $this->buildPredicate($filter['arg0']));
			break;
		case "<":
		case "less":
			$result = sprintf("%s<%s", $filter['arg0'], $filter['arg1']);
			break;
		case ">":
		case "greater":
			$result = sprintf("%s>%s", $filter['arg0'], $filter['arg1']);
			break;
		case "<=":
		case "lessEqual":
			$result = sprintf("%s<=%s", $filter['arg0'], $filter['arg1']);
			break;
		case ">=":
		case "greaterEqual":
			$result = sprintf("%s>=%s", $filter['arg0'], $filter['arg1']);
			break;
		case "distinct":
			$result = sprintf("not(%s=preceding-sibling::*/%s)",
				$filter['arg0'], $filter['arg0']);
			break;
		default:
			throw new \InvalidArgumentException(sprintf(
			  "The operator '%s' is not defined.",
			  $filter['operator']));
			break;
		}
		return $result;
	}

	final public function buildQueryByFilter($filter) {
		return $this->buildQuery($this->buildPredicate($filter));
	}

	final public function buildGetQuery($idValue = NULL) {
		$qi = $this->getQueryInfo();
		$xpath = $qi['xpath'];
		if ((TRUE === boolval($qi['iterable'])) && !is_null($idValue)) {
			$xpath = $this->buildQueryByFilter([
				  "operator" => "stringEquals",
				  "arg0" => $qi['idproperty'],
				  "arg1" => $idValue
			  ]);
		}
		return $xpath;
	}

	final public function buildSetQuery(ConfigObject $object) {
		$qi = $this->getQueryInfo();
		$xpath = $qi['xpath'];
		if (TRUE === boolval($qi['iterable'])) {
			if (FALSE === $object->isNew()) {
				// Update the element with the specified identifier.
				$xpath = $this->buildQueryByFilter([
					  "operator" => "stringEquals",
					  "arg0" => $qi['idproperty'],
					  "arg1" => $object->get($qi['idproperty'])
				  ]);
			} else {
				// Insert a new element.
				$parts = explode("/", $xpath);
				$elementName = array_pop($parts);
				$xpath = substr($xpath, 0, strrpos($xpath, $elementName) - 1);
			}
		}
		return $xpath;
	}

	final public function buildDeleteQuery(ConfigObject $object) {
		$qi = $this->getQueryInfo();
		$xpath = $qi['xpath'];
		if (TRUE === boolval($qi['iterable'])) {
			$xpath = $this->buildQueryByFilter([
				  "operator" => "stringEquals",
				  "arg0" => $qi['idproperty'],
				  "arg1" => $object->get($qi['idproperty'])
			  ]);
		}
		return $xpath;
	}

	final public function buildIsReferencedQuery(ConfigObject $object) {
		$qi = $this->getQueryInfo();
		return sprintf("//%s[%s]", $qi['refproperty'],
		  $this->buildPredicate([
			  "operator" => "stringContains",
			  "arg0" => ".",
			  "arg1" => $object->get($qi['idproperty'])
		  ]));
	}

	final public function buildExistsQuery($filter = NULL) {
		$qi = $this->getQueryInfo();
		if (is_null($filter))
			return $qi['xpath'];
		return $this->buildQueryByFilter($filter);
	}
}
