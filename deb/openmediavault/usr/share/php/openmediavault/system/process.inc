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
		"PATH" => [
			"value" => "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin",
			"obfuscate" => FALSE
		],
		"LC_ALL" => [
			"value" =>"C.UTF-8",
			"obfuscate" => FALSE
		],
		"LANGUAGE" => [
			"value" => "",
			"obfuscate" => FALSE
		]
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
	 * @param string $file The file path, e.g. '/tmp/xyz'.
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
	 * @param string $file The file path, e.g. '/dev/null' or '&1'.
	 * @return void
	 */
	public function setRedirect2toFile($file) {
		$this->redirect2toFile = $file;
	}

	/**
	 * Accept input from a file. Set to FALSE to do not accept input from
	 * a file.
	 * @param string $file The file path.
	 * @return void
	 */
	public function setInputFromFile($file) {
		$this->inputFromFile = $file;
	}

	/**
	 * Accept input from a 'here-string'. Set to FALSE to do not accept
	 * input from 'here-string'.
	 * @param string $data The input string.
	 * @return void
	 */
	public function setInputFromHereString($data) {
		$this->inputFromHereString = $data;
	}

	/**
	 * Do not throw an error exception if the command execution fails.
	 * @param bool $quiet Set to TRUE to do not throw an exception.
	 *   Defaults to TRUE.
	 * @return void
	 */
	public function setQuiet($quiet = TRUE) {
		$this->quiet = $quiet;
	}

	/**
	 * Execute the command in background. The method \em execute immediately
	 * returns in this case.
	 * @param bool $background Set to TRUE to do execute the command in background.
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
	 * @param bool $obfuscate Set to `TRUE` to obfuscate the value
	 *   of the variable when an instance of this class is used in
	 *   an exception. Defaults to `FALSE`.
	 * @return void
	 */
	public function setEnv($name, $value, $obfuscate = FALSE) {
		if (1 == func_num_args()) {
			$parts = explode("=", $name);
			$name = $parts[0];
			$value = $parts[1];
		}
		if (empty($value)) {
			array_remove_key($this->env, $name);
			return;
		}
		$this->env[$name] = [
			"value" => $value,
			"obfuscate" => $obfuscate
		];
	}

	/**
	 * Set environment variable(s) from a file.
	 * @param string $file The name of the file to load from environment
	 *   variable(s) from.
	 * @return void
	 */
	public function setEnvFromFile($file) {
		if (file_exists($file)) {
			$envs = file($file);
			foreach ($envs as $env) {
				if (FALSE === empty($env)) {
					$parts = explode("=", $env);
					$name = trim($parts[0]);
					$value = trim($parts[1]);
					$this->setEnv($name, $value);
				}
			}
		}
	}

	/**
	 * Get the command line to be executed.
	 * @return string The command line.
	 */
	public function getCommandLine() {
		$cmdArgs = [];
		foreach ($this->env as $envk => $envv) {
			$cmdArgs[] = sprintf("export %s=%s;", $envk, strval(
				$envv['value']));
		}
		$cmdArgs = array_merge($cmdArgs, $this->cmdArgs);
		if (FALSE === empty($this->redirect1toFile)) {
			$cmdArgs[] = sprintf("1>%s", $this->redirect1toFile);
		}
		if (FALSE === empty($this->redirect2toFile)) {
			$cmdArgs[] = sprintf("2>%s", $this->redirect2toFile);
		}
		if (FALSE === empty($this->inputFromFile)) {
			$cmdArgs[] = sprintf("<%s", $this->inputFromFile);
		}
		// https://wiki.ubuntu.com/DashAsBinSh#A.3C.3C.3C
		if (FALSE === empty($this->inputFromHereString)) {
			$cmdArgs[] = sprintf("<<EOF\n%s\nEOF", $this->inputFromHereString);
		}
		if (TRUE === $this->background) {
			$cmdArgs[] = "&";
		}
		return implode(" ", $cmdArgs);
	}

	public function __toString(): string {
		return $this->getCommandLine();
	}

	/**
	 * Execute the command.
	 * @param array | null $output If the output argument is present, then
	 *   the specified array will be filled with every line of output from
	 *   the command.
	 *   Trailing whitespace, such as \n, is not included in this array.
	 *   Note that if the array already contains some elements, execute()
	 *   will append to the end of the array.
	 * @param int $exitStatus If the argument is present along with the output
	 *   argument, then the return status of the executed command will be
	 *   written to this variable.
	 * @return string The last line from the result of the command.
	 * @throws \OMV\ExecException
	 */
	public function execute(array &$output = NULL, &$exitStatus = NULL): string {
		// Build the command line.
		$cmdLine = $this->getCommandLine();
		// Execute the command.
		$this->debug("Executing command '%s'", $cmdLine);
		$result = exec($cmdLine, $output, $exitStatus);
		if ((FALSE === $this->quiet) && (0 !== $exitStatus)) {
			throw new \OMV\ExecException($cmdLine, $output, $exitStatus);
		}
		return $result;
	}

	/**
	 * Mark an environment variable to obfuscate.
	 * @param string $name The name of the environment variable.
	 * @param bool $obfuscate Set to `TRUE` to obfuscate the value
	 *   of the variable. Defaults to `TRUE`.
	 * @return void
	 */
	public function obfuscateEnv($name, $obfuscate = TRUE) {
		if (array_key_exists($name, $this->env)) {
			$this->env[$name]['obfuscate'] = $obfuscate;
		}
	}

	/**
	 * Obfuscate the values of all specified environment variables.
	 * Note, the changes are irreversible.
	 * @return void
	 */
	public function obfuscateEnvs() {
		// Process all registered environment variables.
		foreach ($this->env as $envk => &$envv) {
			if ($envv['obfuscate']) {
				$envv['value'] = str_repeat("x", rand(4, 8));
			}
		}
	}
}
