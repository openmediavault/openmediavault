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

class LogFile extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "LogFile";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getList");
		$this->registerMethod("clear");
		$this->registerMethod("getContent");
	}

	/**
	 * Helper method to parse a logfile.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 *   \em id The identifier of the registered log file type.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 * @throw \OMV\Exception
	 */
	function getList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.logfile.getlist");
		// Create the log file specification object.
		$spec = new \OMV\System\LogFileSpec($params['id']);
		// Get the file path. If no permanent file is available (e.g. when
		// the log is retrieved via journalctl), then execute the specified
		// command.
		$filePath = $spec->getFilePath();
		if (TRUE === empty($filePath)) {
			// Create a temporary file.
			$filePath = tempnam(sys_get_temp_dir(), "logfile");
			// Execute the given command to get the log file content.
			$cmd = new \OMV\System\Process($spec->getCommand());
			$cmd->setRedirect1toFile($filePath);
			$cmd->setRedirect2to1();
			$cmd->execute();
		}
		// Limit the number of lines to process because of memory issues.
		$maxLines = \OMV\Environment::getInteger("OMV_MAX_LOGFILE_LINES");
		// Read the log file content.
		// - Get the total number of lines of the file.
		// - Seek to the position from where OMV_MAX_LOGFILE_LINES can be read.
		$file = new \SplFileObject($filePath, "r");
		$file->seek(PHP_INT_MAX);
		$linesTotal = $file->key();
		$file->rewind();
		if ($linesTotal > $maxLines) {
			$file->seek($linesTotal - $maxLines - 1);
		}
		$objects = [];
		$rowNum = 0;
		while (!$file->eof()) {
			$line = $file->fgets();
			// Skip invalid pattern matches.
			$result = preg_match($spec->getRegex(), $line, $matches);
			if ((FALSE === $result) || (0 == $result))
				continue;
			$object = [
				"rownum" => $rowNum++
			];
			foreach ($spec->getColumns() as $columnk => $columnv) {
				// Execute user defined function, e.g. to convert
				// column content?
				if (is_array($columnv)) {
					if (!is_callable($columnv['func']))
						continue;
					$object[$columnk] = $columnv['func'](
					  $matches[$columnv['index']]);
				} else {
					$object[$columnk] = $matches[$columnv];
				}
			}
			$objects[] = $object;
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Clear a syslog file.
	 * @param params An array containing the following fields:
	 *   \em id The identifier of the registered log file type.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	function clear($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.logfile.clear");
		// Check if the given log file type if registered.
		$spec = new \OMV\System\LogFileSpec($params['id']);
		// Get the file path. If no permanent file is available (e.g. when
		// the log is retrieved via journalctl), then exit immediately
		// and notify the user.
		$filePath = $spec->getFilePath();
		if (TRUE === empty($filePath)) {
			throw new \OMV\Exception(
			  "The clean up of entries is not supported for this log file.");
		}
		// Clear the log file.
		if (FALSE === ($fh = fopen($filePath, "w"))) {
			throw new \OMV\Exception(
			  "Failed to open the log file (filename=%s).",
			  $filePath);
		}
		if (FALSE === ftruncate($fh, 0)) {
			throw new \OMV\Exception(
			  "Failed to truncate the log file (filename=%s).",
			  $filePath);
		}
		rewind($fh);
		if (FALSE === fclose($fh)) {
			throw new \OMV\Exception(
			  "Failed to close the log file (filename=%s).",
			  $filePath);
		}
		// Force rsyslog to close and reopen the syslog files.
		$systemCtl = new \OMV\System\SystemCtl("rsyslog.service");
		if (TRUE === $systemCtl->isEnabled())
			$systemCtl->kill("SIGHUP");
	}

	/**
	 * Get the content of a log file. Note, the RPC does not return the
	 * file content itself, instead the file path is returned which is
	 * used by generic download RPC mechanism to deliver the file content.
	 * @param params An array containing the following fields:
	 *   \em id The identifier of the registered log file type.
	 * @param context The context of the caller.
	 * @return An array with the keys \em filename, \em filepath and
	 *   \em unlink.
	 * @throw \OMV\Exception
	 */
	function getContent($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.logfile.getcontent");
		// Check if the given log file type if registered.
		$spec = new \OMV\System\LogFileSpec($params['id']);
		// Create a temporary file which will contain the content of
		// the requested log file.
		$tmpFilePath = tempnam(sys_get_temp_dir(), "logfile");
		// Get the file path. If no permanent file is available (e.g. when
		// the log is retrieved via journalctl), then execute the specified
		// command.
		$filePath = $spec->getFilePath();
		if (TRUE === empty($filePath)) {
			// Execute the given command to get the log file content.
			$cmd = new \OMV\System\Process($spec->getCommand());
			$cmd->setRedirect1toFile($tmpFilePath);
			$cmd->setRedirect2to1();
			$cmd->execute();
		} else {
			// Copy the log file to a temporary location. and modify the file
			// mode/owner to allow the WebGUI PHP backend to unlink it.
			if (FALSE === copy($filePath, $tmpFilePath)) {
				throw new \OMV\Exception(
				  "Failed to copy the log file (source=%s, dest=%s).",
				  $filePath, $tmpFilePath);
			}
		}
		// Modify the file mode/owner to allow the WebGUI PHP backend to
		// unlink it.
		chmod($tmpFilePath, 0700);
		chown($tmpFilePath, \OMV\Environment::get(
		  "OMV_WEBGUI_FILE_OWNERGROUP_NAME"));
		// Return values required by generic download RPC implementation.
		return [
			"filename" => $spec->getFileName(),
			"filepath" => $tmpFilePath,
			"unlink" => TRUE
		];
	}
}
