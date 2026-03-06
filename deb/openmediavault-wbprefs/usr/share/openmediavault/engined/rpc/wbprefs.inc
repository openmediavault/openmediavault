<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
namespace Engined\Rpc;

class WbPrefs extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "WbPrefs";
	}

	public function initialize() {
		$this->registerMethod("setWallpaper");
	}

	/**
	 * Update the specified wallpaper.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	function setWallpaper($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.wbprefs.setwallpaper");
		if (FALSE === filter_var($params['url'], FILTER_VALIDATE_URL,
		 		FILTER_FLAG_PATH_REQUIRED)) {
			throw new \OMV\Exception("Invalid URL '%s'.", $params['url']);
		}
		$imgInfo = getimagesize($params['url']);
		$supportedTypes = [ IMAGETYPE_JPEG, IMAGETYPE_PNG ];
		if (FALSE === $imgInfo || !in_array($imgInfo[2], $supportedTypes)) {
			throw new \OMV\Exception(
				"The specified file at '%s' is not a JPEG or PNG image.",
				$params['url']);
		}
		$contextOptions = stream_context_create([
			'http' => [ 'timeout' => 30, 'follow_location' => 1 ],
			'https' => [ 'timeout' => 30, 'follow_location' => 1 ]
		]);
		$data = file_get_contents($params['url'], false, $contextOptions);
		if (FALSE === $data) {
			throw new \OMV\Exception(
				"Failed to download the image from '%s'.",
				$params['url']);
		}
		$imgMap = [
			"login" => "/var/www/openmediavault/assets/images/login.jpg",
			"shutdown" => "/var/www/openmediavault/assets/images/shutdown.jpg",
			"standby" => "/var/www/openmediavault/assets/images/standby.jpg"
		];
		$filename = $imgMap[$params['page']];
		if (FALSE === file_put_contents($filename, $data)) {
			throw new \OMV\Exception(
				"Failed to save the downloaded image to '%s'.",
				$filename);
		}
		$divertTo = build_path(DIRECTORY_SEPARATOR, '/tmp', str_replace(
			DIRECTORY_SEPARATOR, '_', $filename));
		$cmd = new \OMV\System\Process('dpkg-divert', [
			'--add', '--local', '--rename', '--divert', $divertTo, $filename
		]);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
	}
}
