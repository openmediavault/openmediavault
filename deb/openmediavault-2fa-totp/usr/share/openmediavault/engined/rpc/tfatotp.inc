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

require_once("openmediavault/functions.inc");

class TFATotp extends \OMV\Rpc\ServiceAbstract
{
    public function getName()
    {
        return "TFATotp";
    }

    public function initialize()
    {
        $this->registerMutatingMethod("authUser", "UserMgmt", "authUser");
        $this->registerMutatingMethod("verifyChallenge", "UserMgmt", "verifyChallenge");
        $this->registerMethod("getItemList");
        $this->registerMethod("getItem");
        $this->registerMethod("setItem");
        $this->registerMethod("deleteItem");
        $this->registerMethod("getByContext");
        $this->registerMethod("preEnableByContext");
        $this->registerMethod("enableByContext");
        $this->registerMethod("disableByContext");
    }

    final public function authUser($params, $context, $result)
    {
        if (!array_value($result, "authenticated", false)) {
            return $result;
        }

        if (!$this->isEnabled($result['username'])) {
            return $result;
        }

        $result['challenge'] = [
            "kind" => "totp",
            // Note, the `username` is only used to display it in the
            // verification dialog. The actual username is determined by
            // the session's pending auth data, so it cannot be tampered
            // with by the client.
            "redirecturl" => sprintf("/tfatotp/%s", $result['username'])
        ];

        return $result;
    }

    final public function verifyChallenge($params, $context, $result)
    {
        $username = $params['username'];
        $challengeResponse = $params['challengeresponse'];

        $result['verified'] = $this->verifyCode($username, $challengeResponse['code']);

        return $result;
    }

    public function getItemList($params, $context)
    {
        return \OMV\Rpc\Rpc::call("Config", "getlist", [
            "id" => "conf.system.tfatotp",
            ...$params
        ], $context);
    }

    public function getItem($params, $context)
    {
        return \OMV\Rpc\Rpc::call("Config", "get", [
            "id" => "conf.system.tfatotp",
            "uuid" => $params['uuid']
        ], $context);
    }

    public function getByContext($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);

        try {
            $object = $this->getItemByContext($context);
        } catch (\Exception $e) {
            $object = new \OMV\Config\ConfigObject("conf.system.tfatotp");
            $object->set("name", $context['username']);
        }
        $object->remove("secret");

        return $object->getAssoc();
    }

    public function setItem($params, $context)
    {
        return \OMV\Rpc\Rpc::call("Config", "set", [
            "id" => "conf.system.tfatotp",
            "data" => $params
        ], $context);
    }

    public function deleteItem($params, $context)
    {
        return \OMV\Rpc\Rpc::call("Config", "delete", [
            "id" => "conf.system.tfatotp",
            "uuid" => $params['uuid']
        ], $context);
    }

    public function preEnableByContext($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);

        try {
            $object = $this->getItemByContext($context);
        } catch (\Exception $e) {
            $object = new \OMV\Config\ConfigObject("conf.system.tfatotp");
            $object->set("name", $context['username']);
        }
        $object->set("secret", \OMV\Totp::generateSecret());

        $prd = new \OMV\ProductInfo();
        $uri = \OMV\Totp::getProvisioningUri(
            $object->get("secret"),
            $object->get("name"),
            $prd->getName()
        );

        $object->add("qrcode", "string", "");
        $object->set("qrcode", \OMV\Totp::generateQrDataUrl($uri));

        return $object->getAssoc();
    }

    public function enableByContext($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);
        $this->validateMethodParams($params, "rpc.tfatotp.enableByContext");

        if (!\OMV\Totp::verifyCode($params['secret'], $params['code'])) {
            throw new \OMV\Exception("Invalid verification code.");
        }

        try {
            $object = $this->getItemByContext($context);
        } catch (\Exception $e) {
            $object = new \OMV\Config\ConfigObject("conf.system.tfatotp");
            $object->set("name", $context['username']);
        }
        $object->set("secret", $params['secret']);
        $object->set("enabled", true);

        $db = \OMV\Config\Database::getInstance();
        $db->set($object);

        return $object->getAssoc();
    }

    public function disableByContext($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);
        $this->validateMethodParams($params, "rpc.tfatotp.disableByContext");

        try {
            $object = $this->getItemByContext($context);
        } catch (\Exception $e) {
            throw new \OMV\Exception(
                "2FA TOTP is not configured for this account."
            );
        }

        if (!\OMV\Totp::verifyCode($object->get("secret"), $params['code'])) {
            throw new \OMV\Exception("Invalid verification code.");
        }

        $object->set("enabled", false);
        $object->set("secret", "");

        $db = \OMV\Config\Database::getInstance();
        $db->set($object);
    }

    /**
     * Check if a user has TOTP 2FA enabled.
     *
     * @param string $username The username to check.
     * @return bool TRUE if the user has 2FA enabled, FALSE otherwise.
     * @see verifyCode()
     */
    private function isEnabled(string $username): bool
    {
        try {
            $db = \OMV\Config\Database::getInstance();
            $object = $db->getByFilter("conf.system.tfatotp", [
                'operator' => 'stringEquals',
                'arg0' => 'name',
                'arg1' => $username,
            ], 1);
            return $object->get("enabled");
        } catch (\Exception $e) {
            return false;
        }
    }

    /**
     * Verify a TOTP code for the given user.
     *
     * @param string $username The username.
     * @param string $code     The 6-digit TOTP code to verify.
     * @return bool TRUE if the code is valid, FALSE otherwise.
     * @see \OMV\Totp::verifyCode()
     */
    private function verifyCode(string $username, string $code): bool
    {
        try {
            $db = \OMV\Config\Database::getInstance();
            $object = $db->getByFilter("conf.system.tfatotp", [
                'operator' => 'stringEquals',
                'arg0' => 'name',
                'arg1' => $username,
            ], 1);
            $secret = $object->get("secret");
            if (empty($secret)) {
                return false;
            }
            return \OMV\Totp::verifyCode($secret, $code);
        } catch (\Exception $e) {
            return false;
        }
    }

    private function getItemByContext($context)
    {
        $db = \OMV\Config\Database::getInstance();
        return $db->getByFilter("conf.system.tfatotp", [
            'operator' => 'stringEquals',
            'arg0' => 'name',
            'arg1' => $context['username'],
        ], 1);
    }
}
