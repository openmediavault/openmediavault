<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2025 openmediavault plugin developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
namespace Engined\Module;

class Pwmfan extends \OMV\Engine\Module\ServiceAbstract implements
        \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
    private const CONFIG_FILE = "/etc/openmediavault/pwmfan.json";
    private const SERVICE_NAME = "openmediavault-pwmfan";

    public function getName() {
        return "pwmfan";
    }

    public function getDescription() {
        return gettext("PWM fan controller.");
    }

    public function getStatus() {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.pwmfan");
        $systemCtl = new \OMV\System\SystemCtl(self::SERVICE_NAME);
        return [
            "name" => $this->getName(),
            "title" => gettext("PWM Fan"),
            "enabled" => $object->get("enable"),
            "running" => $systemCtl->isActive()
        ];
    }

    public function deploy() {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.pwmfan");

        $config = [
            "temp_source" => (string) $object->get("temp_source"),
            "hdd_drives" => $this->parseDrives($object->get("hdd_drives")),
            "hdd_standby" => (bool) $object->get("hdd_standby"),
            "cpu_temp_path" => (string) $object->get("cpu_temp_path"),
            "pwm_chip" => (string) $object->get("pwm_chip"),
            "pwm_channel" => (int) $object->get("pwm_channel"),
            "pwm_period" => (int) $object->get("pwm_period"),
            "update_interval" => (int) $object->get("update_interval"),
            "temp1" => (int) $object->get("temp1"),
            "temp2" => (int) $object->get("temp2"),
            "temp3" => (int) $object->get("temp3"),
            "speed1" => (int) $object->get("speed1"),
            "speed2" => (int) $object->get("speed2"),
            "speed3" => (int) $object->get("speed3"),
            "speed4" => (int) $object->get("speed4"),
            "failsafe_speed" => (int) $object->get("failsafe_speed")
        ];

        $jsonFile = new \OMV\Json\File(self::CONFIG_FILE);
        $jsonFile->open("w+");
        $jsonFile->write($config);
        $jsonFile->close();

        $cmd = new \OMV\System\Process("systemctl", ["daemon-reload"]);
        $cmd->setRedirect2to1();
        $cmd->setQuiet(true);
        $cmd->execute();

        $systemCtl = new \OMV\System\SystemCtl(self::SERVICE_NAME);
        if ($object->get("enable")) {
            if ($systemCtl->isMasked()) {
                $systemCtl->unmask(true);
            }
            $systemCtl->enable(false, true);
            $systemCtl->restart(true);
        } else {
            $systemCtl->disable(true, true);
        }
    }

    private function parseDrives($value) {
        $value = trim((string) $value);
        if ($value === "") {
            return [];
        }
        $parts = preg_split("/[\s,]+/", $value, -1, PREG_SPLIT_NO_EMPTY);
        return array_values(array_unique($parts));
    }

    public function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.service.pwmfan",
            [$this, "setDirty"]
        );
    }
}
