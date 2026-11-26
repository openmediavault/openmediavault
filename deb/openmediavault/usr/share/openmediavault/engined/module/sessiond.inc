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

class Sessiond extends \OMV\Engine\Module\ServiceAbstract implements \OMV\Engine\Notify\IListener
{
    public function getName()
    {
        return "sessiond";
    }

    public function getDescription()
    {
        return gettext("Terminate outdated web UI sessions.");
    }

    public function deploy()
    {
        // Nothing to do here.
    }

    /**
     * Ask the daemon to check the web UI sessions immediately. Note, a
     * failure is ignored, e.g. if the daemon is not running, it must
     * not break the RPC that has modified the user or group.
     */
    public function onUserOrGroupChange()
    {
        $systemCtl = new \OMV\System\SystemCtl("openmediavault-sessiond");
        $systemCtl->reload(true);
    }

    public function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.usermngmnt.user",
            [$this, "onUserOrGroupChange"]
        );
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.usermngmnt.group",
            [$this, "onUserOrGroupChange"]
        );
    }
}
