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
namespace OMV\DataModel;

require_once("openmediavault/functions.inc");

class DataModel {
	private $model = NULL;

	/**
	 * @param json The data model as JSON object.
	 */
	public function __construct($model) {
		$this->model = $model;
	}

	/**
	 * Get the data model as JSON object.
	 * @return The data model as JSON object.
	 */
	public function getModel() {
		return $this->model;
	}

	/**
	 * Get the model identifier, e.g. 'conf.service.rsyncd.module'.
	 * @return The model identifier.
	 */
	public function getId() {
		return array_value($this->getModel(), "id", "");
	}

	/**
	 * Get the model identifier alias.
	 * @return The model identifier alias.
	 */
	public function getAlias() {
		return array_value($this->getModel(), "alias", "");
	}

	/**
	 * Get the model title, e.g. 'SSH certificate'.
	 * @return The model titel.
	 */
	public function getTitle() {
		return array_value($this->getModel(), "title", "");
	}

	/**
	 * Get the model description.
	 * @return The model description.
	 */
	public function getDescription() {
		return array_value($this->getModel(), "description", "");
	}
}
