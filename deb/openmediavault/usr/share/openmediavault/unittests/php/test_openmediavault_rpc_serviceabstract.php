#!/usr/bin/phpunit -c/etc/openmediavault
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
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/globals.inc");

class test_openmediavault_rpc_serviceabstract extends \PHPUnit\Framework\TestCase
{
    private function createService()
    {
        return new class () extends \OMV\Rpc\ServiceAbstract {
            public function getName()
            {
                return "Test";
            }

            public function initialize()
            {
                $this->registerMethod("plain");
                $this->registerMethod("download", null, [
                    "allowlist" => ["download"]
                ]);
            }

            public function plain()
            {
            }

            public function download()
            {
                return ["content" => "test download"];
            }
        };
    }

    public function test_internal_context()
    {
        $service = $this->createService();
        $service->initialize();
        $context = \OMV\Rpc\Rpc::createContext("user", OMV_ROLE_USER);
        $this->assertTrue($service->isMethodAllowed("plain", $context));
        $this->assertTrue($service->isMethodAllowed("download", $context));
    }

    public function test_default_rule()
    {
        $service = $this->createService();
        $service->initialize();
        $admin = \OMV\Rpc\Rpc::createContext(
            "admin",
            OMV_ROLE_ADMINISTRATOR,
            ["entrypoint" => "download"]
        );
        $user = \OMV\Rpc\Rpc::createContext(
            "user",
            OMV_ROLE_USER,
            ["entrypoint" => "download"]
        );
        $userRpc = \OMV\Rpc\Rpc::createContext(
            "user",
            OMV_ROLE_USER,
            ["entrypoint" => "rpc"]
        );
        $this->assertTrue($service->isMethodAllowed("plain", $admin));
        $this->assertFalse($service->isMethodAllowed("plain", $user));
        $this->assertTrue($service->isMethodAllowed("plain", $userRpc));
    }

    public function test_allowlist()
    {
        $service = $this->createService();
        $service->initialize();
        $adminDownload = \OMV\Rpc\Rpc::createContext(
            "admin",
            OMV_ROLE_ADMINISTRATOR,
            ["entrypoint" => "download"]
        );
        $adminRpc = \OMV\Rpc\Rpc::createContext(
            "admin",
            OMV_ROLE_ADMINISTRATOR,
            ["entrypoint" => "rpc"]
        );
        $this->assertTrue($service->isMethodAllowed(
            "download",
            $adminDownload
        ));
        $this->assertFalse($service->isMethodAllowed("download", $adminRpc));
    }

    /**
     * @backupGlobals enabled
     */
    public function test_download_only_method_through_proxies()
    {
        $service = $this->createService();
        \OMV\Rpc\ServiceManager::getInstance()->initializeServices();
        $_SESSION = [
            "authenticated" => true,
            "username" => "admin",
            "role" => OMV_ROLE_ADMINISTRATOR
        ];
        $_GET = [
            "service" => $service->getName(),
            "method" => "download",
            "options" => ["updatelastaccess" => false]
        ];

        // Stub only response rendering to avoid emitting HTTP headers.
        $download = $this->getMockBuilder(\OMV\Rpc\Proxy\Download::class)
            ->onlyMethods(["handleResponse"])->getMock();
        $download->expects($this->once())->method("handleResponse")
            ->with(["content" => "test download"]);
        $download->handle();

        $json = $this->getMockBuilder(\OMV\Rpc\Proxy\Json::class)
            ->onlyMethods(["handleResponse"])->getMock();
        $json->expects($this->never())->method("handleResponse");
        try {
            $json->handle();
            $this->fail("The JSON proxy accepted a download-only method.");
        } catch (\OMV\HttpErrorException $e) {
            $this->assertSame(403, $e->getHttpStatusCode());
        }
    }
}
