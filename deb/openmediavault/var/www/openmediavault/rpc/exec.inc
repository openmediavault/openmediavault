<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");

/**
 * A RPC used to manage a asynchronously started command. The necessary
 * informations are stored in a session variable:
 * $_SESSION['execjob'][$pid] = array(
 *   "pid" => $pid,
 *   "outputFile" => $outputFile
 * );
 */
class ExecRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"stop" => array(
				'{"type":"integer"}'
			),
			"getOutput" => array(
				'{"type":"integer"}',
				'{"type":"integer"}'
			),
			"isRunning" => array(
				'{"type":"integer"}'
			)
		);
	}

	/**
	 * Start an execution job.
	 * !!! Note !!! Because of security reasons it is forbidden to start
	 * an command via RPC directly, instead a job should be started
	 * by another RPC function. Check out the cron job implementation.
	 * @param cmd The command to execute.
	 * @return The id to identify the job later. Normally this will be
	 * the pid of the executed command.
	 */
//	public function start($cmd) {
//	}

	/**
	 * Stop the given command.
	 * @param id The id of the execution job to stop.
	 * @return None
	 */
	public function stop($id) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the given job exists. Do not throw an exception if it
		// does not exist. There are some situations where the command has
		// been finished right before the stop RPC has been called.
		if (array_key_exists($id, $_SESSION['execjob'])) {
			// Get the job informations stored in session
			$execInfo = $_SESSION['execjob'][$id];
			// Stop the process
			$cmd = "sudo kill {$execInfo['pid']}";
			OMVUtil::exec($cmd, $output, $res);
			if ($res !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			// Remove the output file and cleanup the session data
			unset($_SESSION['execjob'][$id]);
			unlink($execInfo['outputFile']);
		}
	}

	/**
	 * Get the output of the given command.
	 * @param id The id of the execution job to get the command output.
	 * @param pos The file position withing the command output file.
	 * @return An array with the fields \em running which indicates the
	 * command execution state, \em output which contains the command output
	 * and \em pos which is the current position within the output file (more
	 * detailed it is the size of the output file or EOF position) will be
	 * returned.
	 */
	public function getOutput($id, $pos) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$result = array(
			"pos" => 0,
			"output" => "",
			"running" => false
		);
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the given job exists
		if (!array_key_exists($id, $_SESSION['execjob'])) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "The job does not exist");
		}
		// Get the job informations stored in session
		$execInfo = $_SESSION['execjob'][$id];
		// Check if process is still running
		if (posix_access("/proc/{$execInfo['pid']}", POSIX_R_OK)) {
			$result['running'] = true;
		}
		// Get the command output
		$cmd = "sudo cat '{$execInfo['outputFile']}'";
		OMVUtil::exec($cmd, $output, $res);
		if ($res !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$output = implode("\n", $output);
		// Prepare result
		$result['pos'] = strlen($output);
		// Only return the output from the given position. This will reduce
		// the size of data to transmit back.
		$output = substr($output, $pos); // Note, result may be FALSE
		$result['output'] = $output ? $output : "";
		// If process is not running anymore, then remove the output file
		// and cleanup the session data.
		if ($result['running'] === false) {
			unset($_SESSION['execjob'][$id]);
			unlink($execInfo['outputFile']);
		}
		return $result;
	}

	/**
	 * Check if the given command is still running.
	 * @param id The id of the execution job to get the command output.
	 * @return TRUE if the command is still running, otherwise FALSE.
	 */
	public function isRunning($id) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$result = false;
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the given job exists
		if (!array_key_exists($id, $_SESSION['execjob'])) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "The job does not exist");
		}
		// Get the job informations stored in session
		$execInfo = $_SESSION['execjob'][$id];
		// Check if process is still running
		if (posix_access("/proc/{$execInfo['pid']}", POSIX_R_OK)) {
			$result = true;
		}
		// If process is not running anymore, then remove the output file
		// and cleanup the session data.
		if ($result === false) {
			unset($_SESSION['execjob'][$id]);
			unlink($execInfo['outputFile']);
		}
		return $result;
	}
}
?>
