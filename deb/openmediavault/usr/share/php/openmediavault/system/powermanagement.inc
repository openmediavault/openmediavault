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
namespace OMV\System;

/**
 * Helper class that implements functions regarding the power management.
 * References:
 * https://www.kernel.org/doc/Documentation/power/swsusp.txt
 * https://wiki.archlinux.org/index.php/Suspend_and_Hibernate
 * @ingroup api
 */
class PowerManagement {
	const STATE_NONE = 0x0;
	const STATE_SUSPEND = 0x1;
	const STATE_HIBERNATE = 0x2;
	const STATE_SUSPENDHYBRID = 0x4;

	/**
	 * Helper function.
	 * @param action The systemctl action to execute.
	 * @return void
	 */
	private function exec($action) {
		$cmd = new \OMV\System\Process("systemctl", [$action]);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Checks if the given power management state is supported.
	 * @param state The state to check for.
	 * @return TRUE if the state is supported, if not then FALSE. In case
	 *   of an error NULL is returned.
	 * @see https://cgit.freedesktop.org/pm-utils/tree/pm/pm-functions.in#n294
	 */
	public function isStateSupported($state) {
		$result = FALSE;
		$sys_power_states = array_map("trim", explode(" ", file_get_contents(
			"/sys/power/state")));
		switch ($state) {
		case self::STATE_SUSPEND:
			$result = in_array("mem", $sys_power_states);
			break;
		case self::STATE_HIBERNATE:
			if (in_array("disk", $sys_power_states)) {
				if (is_file("/sys/power/disk")) {
					$contents = file_get_contents("/sys/power/disk");
					if ("[disabled]" !== $contents) {
						$result = (count(file("/proc/swaps")) > 1);
					}
				}
			}
			break;
		case self::STATE_SUSPENDHYBRID:
			// ToDo...
			break;
		}
		return $result;
	}

	/**
	 * Get all supported power management states.
	 * @return The supported power management states.
	 */
	public function getSupportedStates() {
		$states = self::STATE_NONE;
		if (TRUE === $this->isStateSupported(self::STATE_SUSPEND))
			$states |= self::STATE_SUSPEND;
		if (TRUE === $this->isStateSupported(self::STATE_HIBERNATE))
			$states |= self::STATE_HIBERNATE;
		if (TRUE === $this->isStateSupported(self::STATE_SUSPENDHYBRID))
			$states |= self::STATE_SUSPENDHYBRID;
		return $states;
	}

	/**
	 * Reboot the machine.
	 * @return void
	 */
	public function reboot() {
		$this->exec(\OMV\Environment::get("OMV_POWERMGMT_REBOOT",
			"reboot"));
	}

	/**
	 * Shut down the machine.
	 * @return void
	 */
	public function shutdown() {
		$this->exec(\OMV\Environment::get("OMV_POWERMGMT_POWEROFF",
			"poweroff"));
	}

	/**
	 * Put the machine in a sleep state. If suspend to disk (STD) or RAM (STR)
	 * is not supported the system will be shut down. The system will be put
	 * into one of the following state depending on which state is supported:
	 * <ul>
	 * \li Hybrid suspend (STB)
	 * \li Suspend to disk (STD)
	 * \li Suspend to RAM (STR)
	 * \li Shut down and turn of system
	 * </ul>
	 * @return void
	 */
	public function standby() {
		if (TRUE === $this->isStateSupported(self::STATE_SUSPENDHYBRID))
			$this->suspendHybrid();
		else if (TRUE === $this->isStateSupported(self::STATE_HIBERNATE))
			$this->hibernate();
		else if (TRUE === $this->isStateSupported(self::STATE_SUSPEND))
			$this->suspend();
		else
			$this->shutdown();
	}

	/**
	 * Put the machine into suspend to RAM (STR) mode. If this state is not
	 * supported the system will be shut down.
	 * @return void
	 */
	public function suspend() {
		if (FALSE === $this->isStateSupported(self::STATE_SUSPEND))
			$this->shutdown();
		$this->exec(\OMV\Environment::get("OMV_POWERMGMT_SUSPEND",
			"suspend"));
	}

	/**
	 * Put the machine into suspend to disk (STD) mode. If this state is not
	 * supported the system will be shut down.
	 * @return void
	 */
	public function hibernate() {
		if (FALSE === $this->isStateSupported(self::STATE_HIBERNATE))
			$this->shutdown();
		$this->exec(\OMV\Environment::get("OMV_POWERMGMT_HIBERNATE",
			"hibernate"));
	}

	/**
	 * Put the machine into suspend-hybrid (STB) mode. If this state is not
	 * supported the system will be shut down.
	 * @return void
	 */
	public function suspendHybrid() {
		if (FALSE === $this->isStateSupported(self::STATE_SUSPENDHYBRID))
			$this->shutdown();
		$this->exec(\OMV\Environment::get("OMV_POWERMGMT_HYBRIDSLEEP",
			"hybrid-sleep"));
	}
}
