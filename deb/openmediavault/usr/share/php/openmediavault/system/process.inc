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
namespace OMV\System;

/**
 * @ingroup api
 */
class Process {
	use \OMV\DebugTrait;

	private $cmdArgs = [];
	private $redirect1toFile = FALSE;
	private $redirect2toFile = FALSE;
	private $inputFromFile = FALSE;
	private $inputFromHereString = FALSE;
	private $quiet = FALSE;
	private $background = FALSE;
	private $env = [
		"PATH" => "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin",
		"LANG" => "C.UTF-8"
	];

	/**
	 * Constructor
	 * Example:
	 * $cmd = new \OMV\System\Process("ls -alh");
	 * $cmd = new \OMV\System\Process("ls", "-al", "-h");
	 * $cmd = new \OMV\System\Process(array("ls", "-alh"));
	 * $cmd = new \OMV\System\Process("ls", array("-a", "-l", "-h"));
	 */
	public function __construct() {
		$args = func_get_args();
		if (1 == func_num_args() && is_string($args[0])) {
			$this->cmdArgs[] = $args[0];
		} else if (1 == func_num_args() && is_array($args[0])) {
			$this->cmdArgs = $args[0];
		} else if (2 == func_num_args() && is_string($args[0])) {
			$this->cmdArgs[] = $args[0];
			if (is_array($args[1]))
				$this->cmdArgs = array_merge($this->cmdArgs, $args[1]);
			else
				$this->cmdArgs[] = $args[1];
		} else {
			$this->cmdArgs = $args;
		}
	}

	/**
	 * Redirect STDOUT to the given file. Set to FALSE to do not redirect
	 * STDOUT to a file.
	 * @param file The file path, e.g. '/tmp/xyz'.
	 * @return void
	 */
	public function setRedirect1toFile($file) {
		$this->redirect1toFile = $file;
	}

	/**
	 * Redirect STDERR to STDOUT.
	 * @return void
	 */
	public function setRedirect2to1() {
		$this->setRedirect2toFile("&1");
	}

	/**
	 * Redirect STDERR to the given file. Set to FALSE to do not redirect
	 * STDERR to a file.
	 * @param file The file path, e.g. '/dev/null' or '&1'.
	 * @return void
	 */
	public function setRedirect2toFile($file) {
		$this->redirect2toFile = $file;
	}

	/**
	 * Accept input from a file. Set to FALSE to do not accept input from
	 * a file.
	 * @param file The file path.
	 * @return void
	 */
	public function setInputFromFile($file) {
		$this->inputFromFile = $file;
	}

	/**
	 * Accept input from a 'here-string'. Set to FALSE to do not accept
	 * input from 'here-string'.
	 * @param data The input string.
	 * @return void
	 */
	public function setInputFromHereString($data) {
		$this->inputFromHereString = $data;
	}

	/**
	 * Do not throw an error exception if the command execution fails.
	 * @param quiet Set to TRUE to do not throw an exception.
	 *   Defaults to TRUE.
	 * @return void
	 */
	public function setQuiet($quiet = TRUE) {
		$this->quiet = $quiet;
	}

	/**
	 * Execute the command in background. The method \em execute immediatelly
	 * returns in this case.
	 * @param background Set to TRUE to do execute the command in background.
	 *   Defaults to TRUE.
	 * @return void
	 */
	public function setBackground($background = TRUE) {
		$this->background = $background;
	}

	/**
	 * Set an environment variable. To remove an environment variable
	 * simply set an empty string value.
	 * @param string $name The name of the environment variable.
	 * @param mixed $value The value of the environment variable.
	 * @return void
	 */
	public function setEnv($name, $value) {
		if (1 == func_num_args()) {
			$parts = explode("=", $name);
			$name = $parts[0];
			$value = $parts[1];
		}
		if (empty($value)) {
			array_remove_key($this->env, $name);
			return;
		}
		$this->env[$name] = $value;
	}

	/**
	 * Get the command line to be executed.
	 * @return The command line.
	 */
	public function getCommandLine() {
		$cmdArgs = [];
		foreach ($this->env as $envk => $envv)
			$cmdArgs[] = sprintf("export %s=%s;", $envk, strval($envv));
		$cmdArgs = array_merge($cmdArgs, $this->cmdArgs);
		if (FALSE === empty($this->redirect1toFile))
			$cmdArgs[] = sprintf("1>%s", $this->redirect1toFile);
		if (FALSE === empty($this->redirect2toFile))
			$cmdArgs[] = sprintf("2>%s", $this->redirect2toFile);
		if (FALSE === empty($this->inputFromFile))
			$cmdArgs[] = sprintf("<%s", $this->inputFromFile);
		if (FALSE === empty($this->inputFromHereString))
			$cmdArgs[] = sprintf("<<<\"%s\"", $this->inputFromHereString);
		if (TRUE === $this->background)
			$cmdArgs[] = "&";
		return implode(" ", $cmdArgs);
	}

	/**
	 * Execute the command.
	 * @param output If the output argument is present, then the specified
	 *   array will be filled with every line of output from the command.
	 * @param exitStatus If the argument is present along with the output
	 *   argument, then the return status of the executed command will be
	 *   written to this variable.
	 * @return The last line from the result of the command.
	 * @throw \OMV\ExecException
	 */
	public function execute(array &$output = NULL, &$exitStatus = NULL): string {
		// Build the command line.
		$cmdLine = $this->getCommandLine();
		// Execute the command.
		$this->debug("Executing command '%s'", $cmdLine);
		$result = exec($cmdLine, $output, $exitStatus);
		if ((FALSE === $this->quiet) && (0 !== $exitStatus))
			throw new \OMV\ExecException($cmdLine, $output, $exitStatus);
		return $result;
	}
}
