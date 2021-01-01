<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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

/**
 * A mutex provides mutual exclusion. Create the mutex at the beginning
 * of the scope you want to protect. The mutex will be auto-released when
 * leaving the scope.
 * @ingroup api
 */
class Mutex {
	private $auto = TRUE;
	private $semIdentifier = NULL;

	/**
	 * Constructor
	 * @param string $key The unique identifier.
	 * @param boolean $auto Automatically acquire the lock on
	 *   creation and release it on destruction. Defaults
	 *   to TRUE.
	 */
	public function __construct($key, $auto = TRUE) {
		$this->auto = $auto;
		$this->semIdentifier = sem_get(crc32($key));
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
			throw new \OMV\MutexLockException();
		}
	}

	/**
	 * Release the lock.
	 */
	function unlock() {
		return sem_release($this->semIdentifier);
	}
}
