<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
 * along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
 */
namespace OMV\System;

/**
 * This class provides a simple interface to handle a system group.
 * @ingroup api
 */
class Group {
	protected $name = NULL;
	protected $gid = NULL;
	protected $members = NULL;
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param id The name or GID of the system group.
	 *   If \em id is an integer, then it is assumed to be a GID,
	 *   otherwise it is handled as a group name. Numeric strings
	 *   are always handled as group names.
	 * @see https://manpages.debian.org/buster/passwd/groupadd.8.en.html#CAVEATS
	 */
	public function __construct($id) {
		if (is_int($id))
			$this->gid = $id;
		else
			$this->name = $id;
	}

	/**
	 * Get the system group data.
	 * @private
	 * @return void
	 * @throw \OMV\Exception
	 */
	private function getData() {
		if ($this->dataCached !== FALSE) {
			return;
		}
		// Query group information.
		$groupInfo = FALSE;
		if (!is_null($this->gid)) {
			$groupInfo = posix_getgrgid($this->gid);
		} else {
			$groupInfo = posix_getgrnam($this->name);
		}
		if (FALSE === $groupInfo) {
			throw new \OMV\Exception(
			  "Failed to get information about the group '%s': %s",
			  !is_null($this->gid) ? strval($this->gid) : $this->name,
			  posix_strerror(posix_errno()));
		}
		$this->name = $groupInfo['name'];
		$this->gid = $groupInfo['gid'];
		$this->members = $groupInfo['members']; // Supplementary group members only.
		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return void
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		$this->getData();
	}

	/**
	 * Check whether the system group exists.
	 * @return TRUE if the system group exists, otherwise FALSE.
	 */
	public function exists() {
		try {
			$this->getData();
		} catch(\Exception $e) {
			return FALSE;
		}
		return !is_null($this->name) && !is_null($this->gid);
	}

	/**
	 * Assert that the system group exists.
	 * @throw \OMV\AssertException
	 */
	public function assertExists() {
		if (FALSE === $this->exists()) {
			throw new \OMV\AssertException("The group '%s' does not exist.",
			  !is_null($this->gid) ? strval($this->gid) : $this->name);
		}
	}

	/**
	 * Assert that the system group not exists.
	 * @throw \OMV\AssertException
	 */
	public function assertNotExists() {
		if (TRUE === $this->exists()) {
			throw new \OMV\AssertException("The group '%s' already exists.",
			  !is_null($this->gid) ? strval($this->gid) : $this->name);
		}
	}

	/**
	 * Get the group name.
	 * @return The group name.
	 */
	public function getName() {
		$this->getData();
		return $this->name;
	}

	/**
	 * Get the group ID.
	 * @return The group ID.
	 */
	public function getGid() {
		$this->getData();
		return $this->gid;
	}

	/**
	 * Get the user names who are member of this group.
	 * @param bool $includePrimary If set to `TRUE`, the primary group members
	 *   are included as well, otherwise only the supplementary group members
	 *   are returned. Defaults to `FALSE`.
	 * @return An array of user names that are member of this group.
	 */
	public function getMembers($includePrimary = FALSE) {
		$this->getData();
		$result = $this->members;
		if (TRUE === $includePrimary) {
			// Append the primary group members.
			$users = \OMV\System\User::enumerateUsers();
			foreach ($users as $userk => $userv) {
				if ($userv->getGid() == $this->gid) {
					$result[] = $userk;
				}
			}
			// Remove duplicates and re-index the array.
			$result = array_values(array_unique($result));
		}
		return $result;
	}

	/**
	 * Get the group quotas.
	 * @return An array containing the quotas.
	 * @throw \OMV\ExecException
	 */
	public function getQuotas() {
		$cmdArgs = [];
		$cmdArgs[] = "-g";
		$cmdArgs[] = escapeshellarg($this->getName());
		$cmd = new \OMV\System\Process("edquota", $cmdArgs);
		$cmd->setEnv("EDITOR", "cat");
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Parse command output:
		// Filesystem                   blocks       soft       hard     inodes     soft     hard
		// /dev/sdb1                     10188          0      12288          4        0        0
		// /dev/sdc1                         0          0      45056          0        0        0
		$result = [];
		foreach ($output as $outputk => $outputv) {
			if (preg_match("/^\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+".
			  "(\d+)\s+(\d+)$/i", $outputv, $matches)) {
				$result[$matches[1]] = [
					"devicefile" => $matches[1],
					"blocks" => $matches[2],
					"bsoft" => $matches[3],
					"bhard" => $matches[4],
					"inodes" => $matches[5],
					"isoft" => $matches[6],
					"ihard" => $matches[7]
				];
			}
		}
		return $result;
	}

	/**
	 * Check if the given group is a system account.
	 * @return TRUE if the group is a system account, otherwise FALSE.
	 */
	public function isSystemAccount() {
		if (FALSE === ($gid = $this->getGid()))
			return FALSE;
		// Get shadow password suite configuration.
		$ld = \OMV\System\System::getLoginDefs();
		// Get the min/max values of the non-system account ID sequence.
		$min = intval(array_value($ld, "GID_MIN", 1000));
		$max = intval(array_value($ld, "GID_MAX", 60000));
		// Check if the given account ID is within the sequence.
		return !in_range($gid, $min, $max);
	}

	/**
	 * Create a `Group` object from a `/etc/group` file line.
	 * @param string $line A line from the `/etc/group` file.
	 * @return The `Group` object.
	 * @throw \ValueError
	 */
	public static function fromGroupLine($line): self {
		// https://www.cyberciti.biz/faq/understanding-etcgroup-file/
		// Examples:
		// shadow:x:42:openmediavault
		// snmp:x:112:
		// sambashare:x:113:
		// openmediavault:x:999:
		// nut:x:114:
		// foo:x:1002:vagrant,vot
		$fields = explode(":", $line);
		if (4 != count($fields)) {
			throw new \ValueError("Invalid group line '%s'.", $line);
		}
		$group = new self($fields[0]);
		$group->gid = intval($fields[2]);
		$group->members = explode(",", $fields[3]); // Supplementary group members only.
		$group->dataCached = TRUE;
		return $group;
	}

	/**
	 * Enumerate groups.
	 * Note, the `members` field of the `Group` objects contains only the
	 * supplementary group members, the primary group members are not listed
	 * there. If you need them, call the `refresh` method before `getMembers`.
	 * @return An array of `Group` objects indexed by group name.
	 * @throw \OMV\ExecException
	 */
	public static function enumerateGroups(): array {
		$cmd = new \OMV\System\Process("getent", "group");
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = [];
		foreach ($output as $outputk => $outputv) {
			$group = self::fromGroupLine($outputv);
			$result[$group->getName()] = $group;
		}
		return $result;
	}

	/**
	 * Enumerate group names.
	 * @return An array of group names.
	 * @throw \OMV\ExecException
	 */
	public static function enumerateGroupNames(): array {
		$result = self::enumerateGroups();
		return array_keys($result);
	}
}
