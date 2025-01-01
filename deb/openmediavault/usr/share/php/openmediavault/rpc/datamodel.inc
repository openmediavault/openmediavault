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

/**
 * @ingroup api
 */
class DataModel extends \OMV\DataModel\DataModel {
	private $paramsSchema = null;

	public function __construct($model) {
		// Validate the data model definition.
		$schema = new \OMV\Json\Schema('{
			"type":"object",
			"properties":{
				"type":{"type":"string","enum":["rpc"],"required":true},
				"id":{"type":"string","required":true},
				"alias":{"type":"string","required":false},
				"description":{"type":"string","required":false},
				"params":{"type":"any","required":true}
			}
		}');
		try {
			// Convert the associative array into a JSON string
			// before validating the model definition.
			$schema->validate(json_encode_safe($model));
		} catch(\OMV\Json\SchemaValidationException $e) {
			throw new \OMV\Exception(
			  "The data model of the RPC is invalid: %s",
			  $e->getMessage());
		}
		// Call the parent constructor.
		parent::__construct($model);
		// Create the schema describing the RPC parameters.
		$this->paramsSchema = new \OMV\DataModel\Schema($model['params']);
	}

	/**
	 * Get the JSON schema that describes the RPC parameters.
	 * @return The JSON schema describing the RPC parameters.
	 */
	final public function getParamsSchema() {
		return $this->paramsSchema;
	}

	/**
	 * Validate the specified data against the data model.
	 * @param data The JSON data to validate.
	 * @return void
	 * @throw \OMV\Json\SchemaException
	 * @throw \OMV\Json\SchemaValidationException
	 */
	final public function validate($data) {
		$this->getParamsSchema()->validate($data);
	}
}
