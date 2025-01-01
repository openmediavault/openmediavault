<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Michael Myrcik <michael.myrcik@web.de>
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

class Bcache extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "bcache";
	}

	public function initialize() {
		$this->registerMethod("getCacheList");
		$this->registerMethod("createCache");
		$this->registerMethod("deleteCache");
		$this->registerMethod("unregisterCache");
		$this->registerMethod("getCacheConfiguration");
		$this->registerMethod("setCacheConfiguration");
		$this->registerMethod("getCacheStatistic");
		$this->registerMethod("clearCacheStatistic");
		$this->registerMethod("getBackingList");
		$this->registerMethod("getCacheCandidates");
		$this->registerMethod("createBacking");
		$this->registerMethod("deleteBacking");
		$this->registerMethod("getBacking");
		$this->registerMethod("updateBacking");
		$this->registerMethod("unregisterBacking");
		$this->registerMethod("getBackingConfiguration");
		$this->registerMethod("setBackingConfiguration");
		$this->registerMethod("forceRunBacking");
		$this->registerMethod("register");
		$this->registerMethod("getBackingStatistic");
		$this->registerMethod("clearBackingStatistic");
		$this->registerMethod("getSuperBlock");
	}

	/**
	 * Get list of available cache devices.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getCacheList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get all existing cache devices.
		$devs = \OMV\System\Storage\Bcache\CacheDevice::enumerateBcache();
		$result = [];
		$deviceFiles = [];
		foreach ($devs as $devk => $devv) {
			$bcd = new \OMV\System\Storage\Bcache\CacheDevice(
				$devv['devicefile']);
			$backingDevices = $bcd->getBackingDevices();
			$priorityState = $bcd->getPriorityState();
			$result[] = [
				"devicefile" => $bcd->getDeviceFile(),
				"state" => $bcd->getState(),
				"size" => $bcd->getSize(),
				"unused" => $priorityState['unused'],
				"clean" => $priorityState['clean'],
				"dirty" => $priorityState['dirty'],
				"metadata" => $priorityState['metadata'],
				"backingdevices" => $backingDevices,
				"_used" => sizeof($backingDevices) > 0
			];
			$deviceFiles[] = $bcd->getPredictableDeviceFile();
		}
		// Get the configured cache devices.
		$db = \OMV\Config\Database::getInstance();
		$caches = $db->get("conf.service.bcache.cache");
		foreach ($caches as $cachek => $cachev) {
			$devicefile = $cachev->get("devicefile");
			if (in_array($devicefile, $deviceFiles))
				continue;
			$result[] = [
				"devicefile" => $devicefile,
				"state" => 3, // Missing
				"size" => null,
				"unused" => null,
				"clean" => null,
				"dirty" => null,
				"metadata" => null,
				"backingdevices" => null,
				"_used" => false
			];
		}
		// Filter the result list.
		return $this->applyFilter($result, $params['start'], $params['limit'],
			$params['sortfield'], $params['sortdir']);
	}

	/**
	 * Create a Bcache cache device.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function createCache($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.bcache.createCache");
		// Create the cache device.
		$bcd = new \OMV\System\Storage\Bcache\CacheDevice(
			$params['devicefile']);
		$bcd->create();
	}

	/**
	 * Delete a cache device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The cache device file to be deleted, e.g. /dev/sde.
	 * @param context The context of the caller.
	 * @return void or deleted config object.
	 */
	public function deleteCache($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Delete the cache device.
		$devicefile = $params['devicefile'];
		$bcd = new \OMV\System\Storage\Bcache\CacheDevice($devicefile);
		if ($bcd->exists()) {
			if ($bcd->isRegistered()) {
				$bcd->stop();
			}
			$bcd->wipe();
			$devicefile = $bcd->getPredictableDeviceFile();
		}
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.service.bcache.cache", [
			"operator" => "stringEquals",
			"arg0" => "devicefile",
			"arg1" => $devicefile
		]);
		if (sizeof($objects) !== 0) {
			$object = $objects[0];
			// Delete the configuration object.
			$db->delete($object);
			// Return the deleted configuration object.
			return $object->getAssoc();
		}
	}

	/**
	 * Unregister device from kernel.
	 * @param params An array containing the following fields:
	 *   \em devicefile The bcache device file to unregister.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function unregisterCache($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Unregister the device as bcache device.
		$device = new \OMV\System\Storage\Bcache\CacheDevice(
			$params['devicefile']);
		$device->stop();
	}

	/**
	 * Get cache configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getCacheConfiguration($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the backing device details.
		$bcd = new \OMV\System\Storage\Bcache\CacheDevice(
			$params['devicefile']);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.service.bcache.cache", [
			"operator" => "stringEquals",
			"arg0" => "csetuuid",
			"arg1" => $bcd->getCsetUuid()
		]);
		if (sizeof($objects) === 0) {
			// The cache device has not been configured yet.
			// Set current values.
			$result = [
				"devicefile" => $bcd->getDeviceFile(),
				"congesttracking" => TRUE,
				"congestedreadthreshold" => (int) $bcd->getValue(
					"congested_read_threshold_us"),
				"congestedwritethreshold" => (int) $bcd->getValue(
					"congested_write_threshold_us")
			];
		} else {
			// Use existing configuration
			$result = $objects[0]->getAssoc();
		}
		if ($result["congestedreadthreshold"] === 0
				&& $result["congestedwritethreshold"] === 0) {
			$result["congesttracking"] = FALSE;
		} else {
			$result["congesttracking"] = TRUE;
		}
		$result["description"] = $bcd->getDescription();
		return $result;
	}

	/**
	 * Configure a Bcache cache device.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function setCacheConfiguration($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.bcache.setCacheConfiguration");
		// Get the backing device details.
		$bcd = new \OMV\System\Storage\Bcache\CacheDevice(
			$params['devicefile']);
		// Notify the configuration change.
		if ($params['congesttracking'] === TRUE) {
			$congestedreadthreshold = $params['congestedreadthreshold'];
			$congestedwritethreshold = $params['congestedwritethreshold'];
		} else {
			$congestedreadthreshold = 0;
			$congestedwritethreshold = 0;
		}
		$object = new \OMV\Config\ConfigObject("conf.service.bcache.cache");
		$object->setAssoc($params, TRUE, TRUE);
		$object->setAssoc([
			"csetuuid" => $bcd->getCsetUuid(),
			"devicefile" => $bcd->getPredictableDeviceFile(),
			"congestedreadthreshold" => $congestedreadthreshold,
			"congestedwritethreshold" => $congestedwritethreshold
		]);
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			// Check uniqueness.
			$db->assertIsUnique($object, "devicefile");
			$db->assertIsUnique($object, "csetuuid");
		}
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get the statistic of a cache device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The bcache device file to get details from.
	 * @param context The context of the caller.
	 * @return The statistic data table of the given bcache device as string.
	 */
	public function getCacheStatistic($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the bcache device and make sure it exists.
		$bd = new \OMV\System\Storage\Bcache\CacheDevice(
			$params['devicefile']);
		$bd->assertExists();
		return $bd->getStatistic();
	}

	/**
	 * Clears the statistic of a cache device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The bcache device file to get details from.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function clearCacheStatistic($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the bcache device and make sure it exists.
		$bd = new \OMV\System\Storage\Bcache\CacheDevice(
			$params['devicefile']);
		$bd->assertExists();
		$bd->setValue("clear_stats", 1);
	}

	/**
	 * Get list of available backing devices.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getBackingList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		$devs = \OMV\System\Storage\Bcache\BackingDevice::enumerateBcache();
		$result = [];
		$deviceFiles = [];
		foreach ($devs as $devk => $devv) {
			$bbd = new \OMV\System\Storage\Bcache\BackingDevice(
				$devv['devicefile']);
			$bcacheDeviceFile = $bbd->getBcacheDeviceFile();
			$isMounted = FALSE;
			if (\OMV\System\Filesystem\Filesystem::hasFilesystem($bcacheDeviceFile)) {
				$fs = new \OMV\System\Filesystem\Filesystem($bcacheDeviceFile);
				$isMounted = $fs->isMounted();
			}
			$result[] = [
				"devicefile" => $bbd->getDeviceFile(),
				"bcachedevicefile" => $bcacheDeviceFile,
				"cachedevicefile" => $bbd->getCacheDeviceFile(),
				"cachemode" => $bbd->getCacheMode(),
				"size" => $bbd->getSize(),
				"dirtydata" => $bbd->getValue("dirty_data"),
				"state" => $bbd->getState(),
				"mounted" => $isMounted
			];
			$deviceFiles[] = $bbd->getPredictableDeviceFile();
		}
		// Get the configured backing devices.
		$db = \OMV\Config\Database::getInstance();
		$backings = $db->get("conf.service.bcache.backing");
		foreach ($backings as $backingk => $backingv) {
			$devicefile = $backingv->get("devicefile");
			if (in_array($devicefile, $deviceFiles))
				continue;
			$result[] = [
				"devicefile" => $devicefile,
				"bcachedevicefile" => null,
				"cachedevicefile" => null,
				"cachemode" => null,
				"size" => null,
				"dirtydata" => null,
				"state" => "missing",
				"mounted" => false
			];
		}
		// Filter the result list.
		return $this->applyFilter($result, $params['start'], $params['limit'],
			$params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get list of devices that can be used as bcache cache device.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size, vendor, serialnumber and description.
	 */
	public function getCacheCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get a list of all potential usable bcache cache devices.
		$devs = \OMV\System\Storage\Bcache\CacheDevice::enumerateBcache();
		return $devs;
	}

	/**
	 * Create a Bcache backing device.
	 * @param params The method parameters.
	 *   \em devicefile The backing device to be used.
	 *   \em cachedevicefile The cache device to be used.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function createBacking($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.bcache.setBacking");
		// Create the backing device and attach to cache.
		$bbd = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		$bbd->create();
		if ($params['cachedevicefile'] !== "") {
			$bcd = new \OMV\System\Storage\Bcache\CacheDevice(
				$params['cachedevicefile']);
			$bbd->attach($bcd->getCsetUuid());
		}
	}

	/**
	 * Update a Bcache backing device.
	 * @param params The method parameters.
	 *   \em device The device to be used.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function updateBacking($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.bcache.setBacking");
		// Detach from current cache and attach to new cache.
		$bbd = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		$bbd->setValue("detach", 1);
		$csetuuid = "";
		$bindex = "";
		if ($params['cachedevicefile'] !== "") {
			$bcd = new \OMV\System\Storage\Bcache\CacheDevice(
				$params['cachedevicefile']);
			$csetuuid = $bcd->getCsetUuid();
			$bbd->attach($csetuuid);
			$bindex = $bcd->getBackingDeviceIndex($bbd->getDeviceName(TRUE));
		}
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.service.bcache.backing", [
			"operator" => "stringEquals",
			"arg0" => "devicefile",
			"arg1" => $bbd->getPredictableDeviceFile()
		]);
		if (sizeof($objects) !== 0) {
			$object = $objects[0];
			$object->setAssoc([
				"csetuuid" => $csetuuid,
				"bindex" => $bindex
			]);
			// Set the configuration object.
			$db->set($object);
			// Return the configuration object.
			return $object->getAssoc();
		}
	}

	/**
	 * Delete a backing device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The backing device file to be deleted, e.g. /dev/sde.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function deleteBacking($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		$devicefile = $params['devicefile'];
		$bbd = new \OMV\System\Storage\Bcache\BackingDevice($devicefile);
		if ($bbd->exists()) {
			if ($bbd->isRegistered()) {
				$bbd->stop();
			}
			$bbd->wipe();
			$devicefile = $bbd->getPredictableDeviceFile();
		}
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.service.bcache.backing", [
			"operator" => "stringEquals",
			"arg0" => "devicefile",
			"arg1" => $devicefile
		]);
		if (sizeof($objects) !== 0) {
			$object = $objects[0];
			// Delete the configuration object.
			$db->delete($object);
			// Return the deleted configuration object.
			return $object->getAssoc();
		}
	}

	/**
	 * Get backing device details.
	 * @param params An array containing the following fields:
	 *   \em devicefile The devicefile of the backing device, e.g. /dev/sda.
	 * @param context The context of the caller.
	 * @return The backing device details containing the fields \em devicefile,
	 * 	\em cachedevicefile and \em description.
	 */
	public function getBacking($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the backing device details.
		$bbd = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		return [
			"devicefile" => $bbd->getDeviceFile(),
			"cachedevicefile" => $bbd->getCacheDeviceFile(),
			"description" => $bbd->getDescription()
		];
	}

	/**
	 * Unregister device from kernel.
	 * @param params An array containing the following fields:
	 *   \em devicefile The bcache device file to unregister.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function unregisterBacking($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Unregister the device as bcache device.
		$device = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		$device->stop();
	}

	/**
	 * Get backing configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getBackingConfiguration($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the backing device details.
		$bbd = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.service.bcache.backing", [
			"operator" => "stringEquals",
			"arg0" => "devicefile",
			"arg1" => $bbd->getPredictableDeviceFile()
		]);
		if (sizeof($objects) === 0) {
			// The cache device has not been configured yet.
			// Set current values.
			$result = [
				"devicefile" => $bbd->getDeviceFile(),
				"cachemode" => $bbd->getCacheMode(),
				"sequentialcutoff" => $bbd->getSequentialCutoff(),
				"writebackdelay" => (int) $bbd->getValue("writeback_delay"),
				"writebackpercent" => (int) $bbd->getValue("writeback_percent")
			];
		} else {
			// Use existing configuration
			$result = $objects[0]->getAssoc();
		}
		$sequentialCutoff = binary_format($result["sequentialcutoff"], [
			"fromPrefix" => "B",
			"maxPrefix" => "GiB",
			"indexed" => TRUE
		]);
		$result["sequentialcutoffvalue"] = $sequentialCutoff["value"];
		$result["sequentialcutoffunit"] = $sequentialCutoff["unit"];
		$result["description"] = $bbd->getDescription();
		return $result;
	}

	/**
	 * Configure a Bcache backing device.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function setBackingConfiguration($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.bcache.setBackingConfiguration");
		// Get the backing device details.
		$bbd = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		// Notify the configuration change.
		$object = new \OMV\Config\ConfigObject("conf.service.bcache.backing");
		$object->setAssoc($params, TRUE, TRUE);
		$csetuuid = "";
		$bindex = "";
		$cacheDeviceFile = $bbd->getCacheDeviceFile();
		if ($cacheDeviceFile !== "") {
			$bcd = new \OMV\System\Storage\Bcache\CacheDevice(
				$cacheDeviceFile);
			$csetuuid = $bcd->getCsetUuid();
			$bindex = $bcd->getBackingDeviceIndex($bbd->getDeviceName(TRUE));
		}
		$sequentialcutoffvalue = $params['sequentialcutoffvalue'];
		$decimalPlaces = (int) strpos(strrev($sequentialcutoffvalue), ".");
		$object->setAssoc([
			"devicefile" => $bbd->getPredictableDeviceFile(),
			"csetuuid" => $csetuuid,
			"bindex" => $bindex,
			"sequentialcutoff" => (int) binary_convert(
				$sequentialcutoffvalue,
				$params['sequentialcutoffunit'], "B", $decimalPlaces)
		]);
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			// Check uniqueness.
			$db->assertIsUnique($object, "devicefile");
		}
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Force run backing device without cache device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The backing device to run, e.g. /dev/sde.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function forceRunBacking($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		$device = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		$device->setValue("running", 1);
	}

	/**
	 * Get the statistic of a backing device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The bcache device file to get details from.
	 * @param context The context of the caller.
	 * @return The statistic data table of the given bcache device as string.
	 */
	public function getBackingStatistic($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the bcache device and make sure it exists.
		$bd = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		$bd->assertExists();
		return $bd->getStatistic();
	}

	/**
	 * Clears the statistic of a backing device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The bcache device file to get details from.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function clearBackingStatistic($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the bcache device and make sure it exists.
		$bd = new \OMV\System\Storage\Bcache\BackingDevice(
			$params['devicefile']);
		$bd->assertExists();
		$bd->setValue("clear_stats", 1);
	}

	/**
	 * Register device to kernel.
	 * @param params An array containing the following fields:
	 *   \em devicefile The bcache device file to register.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function register($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Register the device as bcache device.
		\OMV\System\Storage\Bcache\BcacheDevice::register(
			$params['devicefile']);
	}

	/**
	 * Get super block of a bcache device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The bcache device file to get details from.
	 * @param context The context of the caller.
	 * @return The super block of the given bcache device as string.
	 */
	public function getSuperBlock($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the bcache device and make sure it exists.
		$bd = new \OMV\System\Storage\Bcache\BcacheDevice(
			$params['devicefile']);
		$bd->assertExists();
		return $bd->getSuperBlock();
	}
}
