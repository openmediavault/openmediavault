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

namespace Engined\Module;

class Sharedfolders extends \OMV\Engine\Module\ServiceAbstract implements \OMV\Engine\Notify\IListener
{
    public function getName()
    {
        return "sharedfolders";
    }

    public function deployAfter(): array
    {
        return ["systemd", "task"];
    }

    public function deploy()
    {
        // Nothing to do here.
    }

    /**
     * Helper function to add a new task when a shared folder has been deleted.
     * @param type The event message type.
     * @param path The event message path.
     * @param object The configuration object.
     */
    final public function onDeleteSharedFolder($type, $path, $object)
    {
        \OMV\Salt\Task::create("omv.task.sharedfolder_alias.disable", [
            "name" => $object['name']
        ]);
    }

    /**
     * Remove the shared folder privileges of a deleted user.
     * @param type The event message type.
     * @param path The event message path.
     * @param object The configuration object.
     */
    final public function onDeleteUser($type, $path, $object)
    {
        $this->deletePrivilegesByRole("user", $object['name']);
    }

    /**
     * Remove the shared folder privileges of a deleted group.
     * @param type The event message type.
     * @param path The event message path.
     * @param object The configuration object.
     */
    final public function onDeleteGroup($type, $path, $object)
    {
        $this->deletePrivilegesByRole("group", $object['name']);
    }

    /**
     * Remove the privileges of the given user or group from all
     * shared folders.
     * @param role The privilege role, e.g. 'user' or 'group'.
     * @param name The name of the user or group.
     */
    private function deletePrivilegesByRole(string $role, string $name): void
    {
        $db = \OMV\Config\Database::getInstance();
        $dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
        $objects = $db->get("conf.system.sharedfolder");
        foreach ($objects as $objectk => $objectv) {
            if (true === $objectv->isEmpty("privileges")) {
                continue;
            }
            $privileges = $objectv->get("privileges.privilege");
            $newPrivileges = array_values(array_filter($privileges,
                function ($privilege) use ($role, $name) {
                    return !(($privilege['type'] === $role) &&
                        ($privilege['name'] === $name));
                }
            ));
            if (count($newPrivileges) === count($privileges)) {
                continue;
            }
            $objectv->set("privileges", [
                "privilege" => $newPrivileges
            ]);
            $db->set($objectv);
            $dispatcher->notify(
                OMV_NOTIFY_MODIFY,
                "org.openmediavault.conf.system.sharedfolder.privilege",
                $objectv->getAssoc()
            );
        }
    }

    public function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.sharedfolder",
            [$this, "setDirty"]
        );
        $dispatcher->addListener(
            OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.sharedfolder",
            [$this, "onDeleteSharedFolder"]
        );
        $dispatcher->addListener(
            OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.usermngmnt.user",
            [$this, "onDeleteUser"]
        );
        $dispatcher->addListener(
            OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.usermngmnt.group",
            [$this, "onDeleteGroup"]
        );
        $moduleMngr = \OMV\Engine\Module\Manager::getInstance();
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.sharedfolder",
            [$moduleMngr->getModule("systemd"), "setDirty"]
        );
    }
}
