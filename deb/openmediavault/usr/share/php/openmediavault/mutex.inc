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
namespace OMV;

require_once("openmediavault/functions.inc");

/**
 * A mutex provides mutual exclusion. Create the mutex at the beginning
 * of the scope you want to protect. The mutex will be auto-released when
 * leaving the scope of the variable.
 * @ingroup api
 */
class Mutex {
	private $auto = TRUE;
	private $semIdentifier = NULL;

	/**
	 * Constructor
	 * @param string $key The unique identifier.
	 * @param boolean $auto Automatically acquire the lock on creation
	 *   and release it on destruction (e.g. when leaving the scope of
	 *   the variable). Defaults to TRUE.
	 */
	public function __construct($key, $auto = TRUE) {
		$this->auto = $auto;
		$this->semIdentifier = sem_get(crc32($key));
		if (FALSE === $this->semIdentifier) {
			throw new \OMV\MutexException("Failed to get semaphore ID: %s",
				last_error_msg());
		}
		if ($this->auto) {
			$this->lock();
		}
	}

	function __destruct() {
		if ($this->auto) {
			$this->unlock();
		}
	}

	/**
	 * Acquire the lock.
	 */
	function lock() {
		if (!sem_acquire($this->semIdentifier)) {
			throw new \OMV\MutexException("Failed to acquire semaphore: %s",
				last_error_msg());
		}
	}

	/**
	 * Release the lock.
	 */
	function unlock(): bool {
		return sem_release($this->semIdentifier);
	}
}
