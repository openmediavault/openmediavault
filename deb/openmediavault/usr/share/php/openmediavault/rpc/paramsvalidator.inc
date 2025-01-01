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
namespace OMV\Rpc;

require_once("openmediavault/functions.inc");

/**
 * @ingroup api
 */
class ParamsValidator {
	private $schema = null;

	/**
	 * @param schema The JSON schema that describes the method parameters.
	 *   Can be given as UTF-8 encoded JSON or an associative array.
	 *   Alternatively this can be the identifier of a data model used
	 *   for validation, e.g. 'rpc.rsync.set'.
	 */
	public function __construct($schema) {
		// Get the data model/schema used to validate the parameters.
		if (!is_json($schema) && is_string($schema)) {
			$mngr = \OMV\DataModel\Manager::getInstance();
			$model = $mngr->getModel($schema);
			$this->schema = $model->getParamsSchema();
		} else {
			// Check if the schema is JSON. If it is an associative array,
			// then auto-convert it into JSON.
			if (is_assoc_array($schema))
				$schema = json_encode_safe($schema);
			// Create the data model schema.
			$this->schema = new \OMV\DataModel\Schema($schema);
		}
	}

	private function getSchema() {
		return $this->schema;
	}

	final public function validate($params) {
		$this->getSchema()->validate($params);
	}
}
