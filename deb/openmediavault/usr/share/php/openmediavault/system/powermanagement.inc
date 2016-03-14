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
namespace OMV\System;

/**
 * Helper class that implements functions regarding the power management.
 * References:
 * https://www.kernel.org/doc/Documentation/power/swsusp.txt
 * https://wiki.archlinux.org/index.php/Suspend_and_Hibernate
 * @ingroup api
 */
class PowerManagement {
	const PM_STATE_NONE = 0x0;
	const PM_STATE_SUSPEND = 0x1;
	const PM_STATE_HIBERNATE = 0x2;
	const PM_STATE_SUSPENDHYBRID = 0x4;

	/**
	 * Helper function.
	 * @return TRUE if command has been executed sucessfully, otherwise
	 *   FALSE.
	 */
	private function exec($action) {
		$mapCommand = array(
			"reboot" => array("shutdown", "-r", "now"),
			"shutdown" => array("shutdown", "-h", "-P", "now"),
			"suspendhybrid" => array("pm-suspend-hybrid"),
			"hibernate" => array("pm-hibernate"),
			"suspend" => array("pm-suspend")
		);
		$cmd = new \OMV\System\Process($mapCommand[$action]);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Checks if the given power management state is supported.
	 * @param state The state to check for.
	 * @return TRUE if the state is supported, if not then FALSE. In case
	 *   of an error NULL is returned.
	 */
	public function isStateSupported($state) {
		$cmdArgs = [];
		switch ($state) {
		case self::PM_STATE_SUSPEND:
			$cmdArgs[] = "--suspend";
			break;
		case self::PM_STATE_HIBERNATE:
			$cmdArgs[] = "--hibernate";
			break;
		case self::PM_STATE_SUSPENDHYBRID:
			$cmdArgs[] = "--suspend-hybrid";
			break;
		}
		$cmd = new \OMV\System\Process("pm-is-supported", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		if (!(($exitStatus == 0) || ($exitStatus == 1)))
			throw new \OMV\ExecException($cmd->getCommandLine(), $output);
		$supported = ($exitStatus == 0) ? TRUE : FALSE;
/*
		// In case of PM_STATE_HIBERNATE and PM_STATE_SUSPENDHYBRID the
		// swap space must be greater than the physical memory size to
		// work correctly.
		if ((TRUE === $supported) && (self::PM_STATE_SUSPEND !== $state)) {
			if (FALSE === ($stats = OMVSystem::getMemoryStats()))
				return NULL;
			if ($stats['mem']['total'] > $stats['swap']['total']) {
				$supported = FALSE;
				syslog(LOG_NOTICE, gettext("Hybrid-suspend and hibernate mode are supported but ignored because the RAM size is greater than the available swap space."));
			}
		}
*/
		return $supported;
	}

	/**
	 * Get all supported power management states.
	 * @return The supported power management states.
	 */
	public function getSupportedStates() {
		$states = self::PM_STATE_NONE;
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPEND))
			$states |= self::PM_STATE_SUSPEND;
		if (TRUE === $this->isStateSupported(self::PM_STATE_HIBERNATE))
			$states |= self::PM_STATE_HIBERNATE;
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPENDHYBRID))
			$states |= self::PM_STATE_SUSPENDHYBRID;
		return $states;
	}

	/**
	 * Reboot the machine.
	 * @return None.
	 */
	public function reboot() {
		$this->exec("reboot");
	}

	/**
	 * Shut down the machine.
	 * @return None.
	 */
	public function shutdown() {
		$this->exec("shutdown");
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
	 * @return None.
	 */
	public function standby() {
		$action = "shutdown";
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPENDHYBRID))
			$action = "suspendhybrid";
		else if (TRUE === $this->isStateSupported(self::PM_STATE_HIBERNATE))
			$action = "hibernate";
		else if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPEND))
			$action = "suspend";
		$this->exec($action);
	}

	/**
	 * Put the machine into suspend to RAM (STR) mode. If this state is not
	 * supported the system will be shut down.
	 * @return None.
	 */
	public function suspend() {
		$action = "shutdown";
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPEND))
			$action = "suspend";
		$this->exec($action);
	}

	/**
	 * Put the machine into suspend to disk (STD) mode. If this state is not
	 * supported the system will be shut down.
	 * @return None.
	 */
	public function hibernate() {
		$action = "shutdown";
		if (TRUE === $this->isStateSupported(self::PM_STATE_HIBERNATE))
			$action = "hibernate";
		$this->exec($action);
	}

	/**
	 * Put the machine into suspend-hybrid (STB) mode. If this state is not
	 * supported the system will be shut down.
	 * @return None.
	 */
	public function suspendHybrid() {
		$action = "shutdown";
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPENDHYBRID))
			$action = "suspendhybrid";
		$this->exec($action);
	}
}
