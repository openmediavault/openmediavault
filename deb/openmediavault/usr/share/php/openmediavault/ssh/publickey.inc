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
namespace OMV\Ssh;

class TmpKeyFile extends \OMV\System\TmpFile {
	function __construct() {
		parent::__construct();
		chmod($this->getFileName(), 0600);
	}
}

class PublicKey {
	private $data = '';

	public function __construct(string $data) {
		$this->data = $data;
	}

	public static function isRfc4716(string $data): bool {
		if (!preg_match(
				'/^---- BEGIN SSH2 PUBLIC KEY ----(\n|\r|\f)(.+)'.
				'(\n|\r|\f)---- END SSH2 PUBLIC KEY ----$/sm',
			 	$data)) {
			return FALSE;
		}
		return TRUE;
	}

	public static function isOpenSSH(string $data): bool {
		if (!preg_match(
				'/^ssh-rsa AAAA[0-9A-Za-z+\/]+[=]{0,3}\s*(.+)*$/',
				$data)) {
			return FALSE;
		}
		return TRUE;
	}

	public function toRfc4716(): string {
		if (self::isRfc4716($this->data)) {
			return $this->data;
		}
		$keyFile = new TmpKeyFile();
		$keyFile->write($this->data);
		$cmdArgs = [];
		$cmdArgs[] = "-e";
		$cmdArgs[] = sprintf("-f %s", escapeshellarg(
			$keyFile->getFileName()));
		$cmd = new \OMV\System\Process("ssh-keygen", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return implode("\n", $output);
	}

	public function toOpenSSH(): string {
		if (self::isOpenSSH($this->data)) {
			return $this->data;
		}
		$keyFile = new TmpKeyFile();
		$keyFile->write($this->data);
		$cmdArgs = [];
		$cmdArgs[] = "-i";
		$cmdArgs[] = sprintf("-f %s", escapeshellarg(
			$keyFile->getFileName()));
		$cmd = new \OMV\System\Process("ssh-keygen", $cmdArgs);
		$cmd->setRedirect2to1();
		return $cmd->execute();
	}
}
