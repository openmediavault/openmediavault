<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class QuotaModule extends OMVModuleAbstract implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "quota";
	}

	/**
	 * Update filesystem quota.
	 */
	public function mkConf() {
		$cmd = "sudo omv-mkconf quota 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete the associated quota configuration if a filesystem is
	 * going to be deleted.
	 * @param object The filesystem object to be deleted.
	 */
	public function deleteFilesystem($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Does a quota configuration exists for the deleted filesystem?
		$xpath = sprintf("//system/storage/filesystem/quota[fsuuid='%s']",
		  $object['uuid']);
		$qObject = $xmlConfig->get($xpath);
		if (!is_null($qObject)) {
			// Execute RPC to delete the configuration object.
			OMVRpc::exec("Quota", "delete", array($qObject['uuid']));
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.filesystem",
		  array($this, "deleteFilesystem"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.filesystem.quota",
		  array($this, "mkConf"));
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new QuotaModule());
?>
