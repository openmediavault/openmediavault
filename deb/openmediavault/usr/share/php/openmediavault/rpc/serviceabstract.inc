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
namespace OMV\Rpc;

require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");

/**
 * The core RPC service class.
 * @ingroup api
 */
abstract class ServiceAbstract {
	use \OMV\Debugable;

	private $registeredMethods = array();
	private $registeredMethodSequences = array();

	/**
	 * Get the name of the RPC service.
	 * @return The name of the RPC service.
	 */
	abstract public function getName();

	/**
	 * Initialize the RPC service.
	 */
	abstract public function initialize();

	/**
	 * Register a RPC service method. Only those methods can be
	 * executed via RPC.
	 * @param rpcName The name of the RPC service method.
	 * @param methodName The name of the class method that implements the
	 *   RPC sevice method. If set to NULL the name given in \em rpcName
	 *   is used. Defaults to NULL.
	 * @return TRUE on success, otherwise an error is thrown.
	 */
	final protected function registerMethod($rpcName, $methodName = NULL) {
		$methodName = is_null($methodName) ? $rpcName : $methodName;
		if(!method_exists($this, $methodName)) {
			throw new Exception(
			  "The method '%s' does not exist for RPC service '%s'",
			  $methodName, $this->getName());
		}
		$this->registeredMethods[$rpcName] = $methodName;
		return TRUE;
	}

	/**
	 * Create a combined RPC service method call sequence of the original
	 * method and the passed method. The passed method is called with the
	 * parameters of the original RPC service method.
	 * @param service The name of the RPC service.
	 * @param method The name of the original RPC service method.
	 * @param method2 The name of the method that should be called after
	 *   the original RPC service method.
	 * @return TRUE on success, otherwise an exception is thrown.
	 */
	final protected function registerMethodSequence($service, $method,
	  $method2) {
		$rpcServiceMngr = &\OMV\Rpc\ServiceManager::getInstance();
		if (FALSE === ($rpcService = $rpcServiceMngr->getService(
		  $service))) {
			throw new Exception("RPC service not found (name=%s)",
			  $service);
		}
		if (FALSE === $rpcService->hasMethod($method)) {
			throw new Exception(
			  "The method '%s' does not exist for RPC service '%s'",
			  $method, $service);
		}
		$rpcService->registeredMethodSequences[$method] = array(
		  "service" => $this->getName(),
		  "method" => $method2);
	}

	/**
	 * Check if the given service method exists.
	 * @return TRUE if the service method exists, otherwise FALSE.
	 */
	final public function hasMethod($name) {
		return in_array($name, $this->registeredMethods);
	}

	/**
	 * Call the given RPC service method. Registered method hooks will be
	 * called after the origin method has been successfully called.
	 * @param name The name of the method.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return Returns the return value of the RPC service method.
	 */
	final public function callMethod($name, $params, $context) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Do not check if the method is registered, but ensure that the
		// class implements the given method. Thus we can call other public
		// PHP class methods from within the service class.
		if (!method_exists($this, $name)) {
			throw new Exception(
			  "The method '%s' does not exist for RPC service '%s'",
			  $name, $this->getName());
		}
		$result = call_user_func_array(array($this, $name),
		  array($params, $context));
		// Process registered RPC service method hooks.
		if (array_key_exists($name, $this->registeredMethodSequences)) {
			foreach ($this->registeredMethodSequences[$name] as
			  $hookk => $hookv) {
				\OMV\Rpc\Rpc::call($hookv['service'], $hookv['method'],
				  $params, $context);
			}
		}
		return $result;
	}

	/**
	 * Call the given RPC service method in a background process.
	 * Registered method hooks will be called after the origin method
	 * has been successfully called.
	 * @param name The name of the method.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	final public function callMethodBg($name, $params, $context) {
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if ($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			// Execute the given RPC service method.
			$this->callMethod($name, $params, $context);
			// Update the background process status file.
			$this->finalizeBgProcStatus($bgStatusFilename);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Helper function to validate the given method parameters using
	 * JSON schema.
	 * @param params The parameters to be validated.
	 * @param schema The JSON schema that describes the method parameters.
	 *   Can be given as UTF-8 encoded JSON or an associative array.
	 *   Alternatively this can be the identifier of a data model used
	 *   for validation.
	 * @return None.
	 */
	final protected function validateMethodParams($params, $schema) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Convert the given paramaters into JSON. This is necessary because
		// the 'params' variable contains an associative array which can not
		// be processed by the validator.
		$validator = new ParamsValidator($schema);
		$validator->validate(json_encode_safe($params));
	}

	/**
	 * Helper function to validate the method caller context.
	 * @param context The caller context to be validated.
	 * @param required The required context.
	 * @return None.
	 * @throw \OMV\Rpc\Exception
	 */
	final protected function validateMethodContext($context, $required) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Validate the method calling context:
		// - Check the username
		if (array_key_exists("username", $required)) {
			if (!is_array($required['username']))
				$required['username'] = array($required['username']);
			foreach ($required['username'] as $usernamek => $usernamev) {
				if ($context['username'] !== $usernamev)
					throw new Exception("Invalid context");
			}
		}
		// - Check the role
		if (array_key_exists("role", $required)) {
			if (!($context['role'] & $required['role']))
				throw new Exception("Invalid context");
		}
	}

	/**
	 * Helper function to get the administrator context.
	 * @return The context object.
	 */
	final protected function getAdminContext() {
		return array(
			  "username" => "admin",
			  "role" => OMV_ROLE_ADMINISTRATOR
		  );
	}

	/**
	 * Helper function to fork the current running process.
	 * @return The PID of the child process.
	 * @throw \OMV\Rpc\Exception
	 */
	final protected function fork() {
		$pid = pcntl_fork();
		if ($pid == -1) {
			throw new Exception("Failed to fork process.");
		} else if ($pid > 0) { // Parent process
//			$this->debug("Child process forked (pid=%d)", $pid);
		}
		return $pid;
	}

	/**
	 * Helper function to create the file containing the background
	 * process status.
	 * @return The name of the background process status file.
	 * @throw \OMV\Rpc\Exception
	 */
	final protected function createBgProcStatus() {
		$filename = tempnam(sys_get_temp_dir(), "bgstatus");
		if (FALSE === touch($filename)) {
			throw new Exception(
			  "Failed to create background process status file (filename=%s).",
			  $filename);
		}
		return $filename;
	}

	/**
	 * Helper function to create the file containing the background
	 * process output.
	 * @return The name of the background process output file.
	 * @throw \OMV\Rpc\Exception
	 */
	final protected function createBgProcOutput($prefix = "bgoutput") {
		$filename = tempnam(sys_get_temp_dir(), $prefix);
		if (FALSE === touch($filename)) {
			throw new Exception(
			  "Failed to create file background process output (filename=%s).",
			  $filename);
		}
		return $filename;
	}

	/**
	 * Helper function to write a string to the background process output.
	 * @param filename Path to the file where to write the content.
	 * @param content The content to write.
	 * @return This function returns the number of bytes that were written
	 *   to the file, or FALSE on failure.
	 */
	final protected function writeBgProcOutput($filename, $content) {
		return file_put_contents($filename, $content, FILE_APPEND | LOCK_EX);
	}

	/**
	 * Helper function to update the background process status file.
	 * @param filename The name of the status file.
	 * @param pid The PID of the background process.
	 * @return The background process status.
	 */
	final protected function initializeBgProcStatus($filename, $pid) {
		$jsonFile = new \OMV\Json\File($filename);
		$jsonFile->open("r+");
		if ($jsonFile->isEmpty()) {
			$status = array(
				  "pid" => $pid,
				  "running" => TRUE
			  );
			$jsonFile->write($status);
		} else {
			// Note, by default the file should be empty, but it can
			// happen that the child process was faster and has already
			// been updated the file content.
			$status = $jsonFile->read();
		}
		$jsonFile->close();
		return $status;
	}

	/**
	 * Helper function to finalize the background process status file.
	 * @param filename The name of the status file.
	 * @param result The result of the background process, e.g. the
	 *   output of an executed command. Defaults to NULL.
	 * @param exception The exception that has been thrown. Defaults
	 *   to NULL.
	 * @return The background process status.
	 */
	final protected function finalizeBgProcStatus($filename,
	  $result = NULL, $exception = NULL) {
		$jsonFile = new \OMV\Json\File($filename);
		$jsonFile->open("r+");
		$status = $jsonFile->read();
		$status['pid'] = posix_getpid();
		$status['running'] = FALSE;
		$status['result'] = $result;
		$status['error'] = NULL;
		if (!is_null($exception)) {
			$status['error'] = array(
				"code" => $exception->getCode(),
				"message" => $exception->getMessage(),
				"trace" => $exception->__toString()
			);
		}
		$jsonFile->write($status);
		$jsonFile->close();
		return $status;
	}

	/**
	 * Helper function to update informations of the background process
	 * status file.
	 * @param filename The name of the status file.
	 * @param key The name of the field to be modified.
	 * @param value The new value of the field.
	 * @return The background process status.
	 */
	final protected function updateBgProcStatus($filename, $key, $value) {
		$jsonFile = new \OMV\Json\File($filename);
		$jsonFile->open("r+");
		$status = $jsonFile->read();
		$status['pid'] = posix_getpid();
		$status['running'] = TRUE;
		$status[$key] = $value;
		$jsonFile->write($status);
		$jsonFile->close();
		return $status;
	}

	/**
	 * Helper function to get the background process status file content.
	 * @param filename The name of the status file.
	 * @return The background process status.
	 */
	final protected function getBgProcStatus($filename) {
		$jsonFile = new \OMV\Json\File($filename);
		$jsonFile->open("r");
		$status = $jsonFile->read();
		$jsonFile->close();
		// Check if the process is really still running.
		if (TRUE === $status['running']) {
			if (!is_dir(sprintf("/proc/%d", $status['pid'])))
				$status['running'] = FALSE;
		}
		return $status;
	}

	/**
	 * Helper function to unlink the background process status file.
	 * @param filename The name of the status file.
	 * @return None.
	 */
	final protected function unlinkBgProcStatus($filename) {
		$jsonFile = new \OMV\Json\File($filename);
		$jsonFile->open("r");
		$status = $jsonFile->read();
		$jsonFile->close();
		// Unlink the command output file if defined.
		if (array_key_exists("outputfilename", $status) && !empty(
		  $status['outputfilename'])) {
			@unlink($status['outputfilename']);
		}
		$jsonFile->unlink();
	}

	/**
	 * Helper function to execute an external program. The command output
	 * will be redirected to the given file if set.
	 * @param command The command that will be executed.
	 * @param output If the output argument is present, then the specified
	 *   array will be filled with every line of the command output from
	 *   stdout. Trailing whitespace, such as \n, is not included in this
	 *   array.
	 * @param outputFilename The name of the file that receives the command
	 *   output from stdout. If set to NULL the command output will not be
	 *   redirected to a file.
	 * @return The exit code of the command or -1 in case of an error.
	 */
	final protected function exec($command, &$output = NULL,
	  $outputFilename = NULL) {
		$output = array();
		$descriptors = array(
			  0 => array("pipe", "r"), // STDIN
			  1 => array("pipe", "w"), // STDOUT
			  2 => array("pipe", "w")  // STDERR
		  );
		// Execute the command.
		$process = proc_open($command, $descriptors, $pipes);
		if ((FALSE === $process) || !is_resource($process))
			return -1;
		// Immediatelly close STDIN.
		fclose($pipes[0]);
		$pipes[0] = NULL;
		// Read from the pipes. Make STDIN/STDOUT/STDERR non-blocking.
		stream_set_blocking($pipes[1], 0);
		stream_set_blocking($pipes[2], 0);
		// Read the output from STDOUT/STDERR.
		while (TRUE) {
			$read = array();
			// Collect the reading streams to monitor.
			if(!is_null($pipes[1])) $read[] = $pipes[1];
			if(!is_null($pipes[2])) $read[] = $pipes[2];
			if (FALSE === ($r = stream_select($read, $write = NULL,
			  $except = NULL, 1)))
				break;
			foreach ($read as $readk => $readv) {
				if ($readv == $pipes[1]) { // STDOUT
					// Read the STDOUT command output.
					if (FALSE !== ($line = fgets($pipes[1]))) {
						// Redirect command output to file?
						if (is_string($outputFilename) && !empty(
						  $outputFilename)) {
							$this->writeBgProcOutput($outputFilename, $line);
						}
						$output[] = rtrim($line);
					}
					// Close the pipe if EOF has been detected.
					if (TRUE === feof($pipes[1])) {
						fclose($pipes[1]);
						$pipes[1] = NULL;
					}
				} else if ($readv == $pipes[2]) { // STDERR
					// Read the STDERR command output.
					$line = fgets($pipes[2]);
					// Close the pipe if EOF has been detected.
					if (TRUE === feof($pipes[2])) {
						fclose($pipes[2]);
						$pipes[2] = NULL;
					}
				}
			}
			// Everything read?
			if (is_null($pipes[1]) && is_null($pipes[2]))
				break;
		}
		return proc_close($process);
	}

	/**
	 * Helper function to executes specified program in current process space.
	 * @param path The path to a binary executable or a script with a
	 *   valid path pointing to an executable in the shebang as the first
	 *   line.
	 * @param args An array of argument strings passed to the program.
	 * @param outputFilename The name of the file that receives the command
	 *   output from STDOUT. STDERR will be redirected to this file, too.
	 *   If set to NULL the command output will not be redirected to a file.
	 * @return Returns FALSE on error and does not return on success.
	 */
	final protected function execve($path, $args = NULL,
	  $outputFilename = NULL) {
		global $stdOut, $stdErr;
		// Redirect command output to file?
		$redirectOutput = (is_string($outputFilename) && !empty(
		  $outputFilename));
		if (TRUE === $redirectOutput) {
			// Close STDOUT and STDERR and create new files that will use
			// the file descriptors no. 1 and 2.
			(is_resource($stdOut)) ? fclose($stdOut) : fclose(STDOUT);
			(is_resource($stdErr)) ? fclose($stdErr) : fclose(STDERR);
			$stdOut = fopen($outputFilename, "w");
			$stdErr = fopen($outputFilename, "w");
		}
		$result = pcntl_exec($path, $args);
		// Note, this code path is only reached if pcntl_exec fails.
		if ((FALSE === $result) && (TRUE === $redirectOutput)) {
			// Note, STDOUT and STDERR are destroyed and can't be
			// used anymore.
			fclose($stdOut);
			fclose($stdErr);
		}
		return $result;
	}

	/**
	 * Helper function to filter the method result using the given
	 * filter arguments.
	 * @param array The array of objects to filter.
	 * @param start The index where to start.
	 * @param limit The number of elements to process.
	 * @param sortField The name of the column used to sort.
	 * @param sortDir The sort direction, ASC or DESC.
	 * @return An array containing the elements matching the given
	 *   restrictions. The field \em total contains the total number of
	 *   elements, \em data contains the elements as array. An exception
	 *   will be thrown in case of an error.
	 */
	final protected function applyFilter($array, $start, $limit,
	  $sortField = NULL, $sortDir = NULL) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$total = count($array);
		if ($total > 0) {
			if (!is_null($sortField))
				array_sort_key($array, $sortField);
			if (!is_null($sortDir) && $sortDir === "DESC")
				$array = array_reverse($array);
			if (($start >= 0) && ($limit >= 0))
				$array = array_slice($array, $start, $limit);
		}
		return array(
			"total" => $total,
			"data" => $array
		);
	}

	/**
	 * Helper function to delete an configuration object matching the
	 * given XPath expression. The notification types OMV_NOTIFY_PREDELETE
	 * and OMV_NOTIFY_DELETE will be submitted to its subscribers.
	 * @param xpath The XPath expression of the object to delete.
	 * @param message The message of the notification to be submitted.
	 * @return The deleted configuration object.
	 * @throw \OMV\Config\DatabaseException
	 */
	protected function deleteConfigObjectByPath($xpath, $message) {
		global $xmlConfig;
		// Get the configuration object.
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new \OMV\Config\DatabaseException(
			  "Failed to execute query '%s'.", $xpath);
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE, $message, $object);
		// Delete configuration object.
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new \OMV\Config\DatabaseException(
			  "Failed to delete configuration (xpath=%s).", $xpath);
		}
		// Notify configuration changes.
		$dispatcher->notify(OMV_NOTIFY_DELETE, $message, $object);
		// Return the deleted configuration object.
		return $object;
	}

	/**
	 * Helper function to mark a module as dirty.
	 * @param name The name of the module.
	 * @return The list of dirty modules.
	 */
	final protected function setModuleDirty($name) {
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		return $moduleMngr->setModuleDirty($name);
	}

	/**
	 * Helper function to check whether a module is marked dirty.
	 * @param name The name of the module.
	 * @return TRUE if the module is marked dirty, otherwise FALSE.
	 */
	final protected function isModuleDirty($name) {
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		return $moduleMngr->isModuleDirty($name);
	}
}
