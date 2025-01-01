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
namespace Engined\Rpc;

class Exec extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Exec";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("stop");
		$this->registerMethod("getOutput");
		$this->registerMethod("isRunning");
		$this->registerMethod("enumerate");
		$this->registerMethod("attach");
	}

	/**
	 * Stop the given background process.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the background process status file.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function stop($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.exec.stop");
		// Get the background process status.
		$status = new \OMV\Rpc\BgStatus($this->getBgProcStatus(
			$params['filename']));
		// Kill the background process.
		$this->debug("Sending SIGTERM to process %d", $status->getPid());
		if (!posix_kill_ex($status->getPid(), SIGTERM)) {
			throw new \OMV\Exception("Failed to kill process (pid=%d).",
			  $status->getPid());
		}
		// Unlink the background process status file.
		$this->unlinkBgProcStatus($params['filename']);
	}

	/**
	 * Get the output of the given background process.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the background process status file.
	 *   \em pos The file position within the command output file.
	 *   \em length Maximum number of bytes to be read. Note,
	 *   the command output is read in 1KiB chunks.
	 * @return Returns an array with the fields \em filename containing
	 *   the background process status file, \em running which indicates
	 *   the command execution state, \em output which contains the
	 *   command output, \em pos which is the current position within
	 *   the command output file and \em pendingOutput is TRUE when the
	 *   command output has not been read completely (only if parameter
	 *   \em length has been set). If all command output is read and
	 *   \em pendingOutput is set to FALSE then \em pos contains the size
	 *   of the command output file and EOF position.
	 * @throw \OMV\Exception
	 * @throw \OMV\Rpc\TraceException
	 */
	public function getOutput($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.exec.getoutput");
		// Get the background process status file.
		$status = new \OMV\Rpc\BgStatus($this->getBgProcStatus(
			$params['filename']));
		// Check if there was an error and trigger an exception if
		// necessary. Inject the stack trace coming from the background
		// process.
		if ($status->hasError()) {
			$error = $status->getError();
			throw new \OMV\Rpc\TraceException($error['message'],
				$error['code'], $error['trace']);
		}
		// Prepare the result object.
		$result = [
			"filename" => $params['filename'],
			"pos" => 0,
			"output" => "",
			"pendingOutput" => FALSE,
			"running" => $status->isRunning()
		];
		// Check if there is a background process output file and try to
		// read the content starting from the given position to the end
		// of the file.
		if ($status->hasOutputFilename()) {
			$bgOutputFilename = $status->getOutputFilename();
			// Read the command output from the given file.
			if (FALSE === ($fp = fopen($bgOutputFilename, "r"))) {
				throw new \OMV\Exception(
					"Failed to open the command output file (filename=%s).",
					$bgOutputFilename);
			}
			// Get the requested command output from the given position to
			// the end of the file.
			fseek($fp, $params['pos'], SEEK_SET);
			// Read the file content and convert line breaks.
			$bytesRead = 0;
			$chunkLength = 1024; // 1KiB
			while (!feof($fp)) {
				if (array_key_exists("length", $params)) {
					if ($bytesRead >= $params['length']) {
						$result['pendingOutput'] = TRUE;
						break;
					}
				}
				$result['output'] .= str_replace([ "\r\n", "\r" ],
					"\n", fread($fp, $chunkLength));
				$bytesRead += $chunkLength;
			}
			// Get the current position within the command output file.
			$result['pos'] = ftell($fp);
			fclose($fp);
		}
		// Cleanup if the process is not running anymore and all command
		// output has been read.
		if (!$status->isRunning() && !$result['pendingOutput']) {
			$this->unlinkBgProcStatus($params['filename']);
		}
		return $result;
	}

	/**
	 * Check if the given background process is still running.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the background process status file.
	 * @param context The context of the caller.
	 * @return An array with the fields \em filename containing the background
	 *   process status file and \em running which indicates the command
	 *   execution state.
	 * @throw \OMV\Rpc\TraceException
	 */
	public function isRunning($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.exec.isrunning");
		// Get the background process status.
		$status = new \OMV\Rpc\BgStatus($this->getBgProcStatus(
			$params['filename']));
		// Check if there was an error and trigger an exception if
		// necessary. Inject the stack trace coming from the background
		// process.
		if ($status->hasError()) {
			$error = $status->getError();
			throw new \OMV\Rpc\TraceException($error['message'],
			  $error['code'], $error['trace']);
		}
		// Cleanup if process is not running anymore.
		if (FALSE === $status->isRunning())
			$this->unlinkBgProcStatus($params['filename']);
		// Return the running state.
		return [
			"filename" => $params['filename'],
			"running" => $status->isRunning()
		];
	}

	/**
	 * Determine all running background processes.
	 * @param params Optional method parameters.
	 * @param context The context of the caller.
	 * @return An associative array containing status information of all
	 *   running background processes. The `key` is the name of the
	 *   status file and the `value` is an associative array containing
	 *   some information about the corresponding background process.
	 */
	public function enumerate($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		return $this->enumerateRunningBgProcStatus();
	}

	/**
	 * Attach to the specified background process.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the background process status file.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	public function attach($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.exec.attach");
		return $params['filename'];
	}
}
