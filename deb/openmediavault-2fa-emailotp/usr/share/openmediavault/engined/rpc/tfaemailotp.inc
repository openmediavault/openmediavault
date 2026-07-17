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

class TFAEmailOtp extends \OMV\Rpc\ServiceAbstract
{
    private const OTP_DIGITS = 6; // OTP length in digits.
    private const OTP_TTL = 300; // OTP validity period in seconds.

    public function getName()
    {
        return "TFAEmailOtp";
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
        $this->registerMethod("sendOtpByContext");
        $this->registerMethod("enableByContext");
        $this->registerMethod("disableByContext");
    }

    final public function authUser($params, $context, $result)
    {
        if (!$result['authenticated']) {
            return $result;
        }

        if (!$this->isEnabled($result['username'])) {
            return $result;
        }

        if (!$this->hasEmailAddress($result['username'])) {
            syslog(LOG_ERR, sprintf(
                "2FA Email OTP login denied for user '%s': no email address configured.",
                $result['username']
            ));
            $result['authenticated'] = false;
            return $result;
        }

        $otp = $this->generateOtp();

        $this->sendOtp($result['username'], $otp);

        $result['challenge'] = [
            "kind" => "emailotp",
            // Note, the `username` is only used to display it in the
            // verification dialog. The actual username is determined by
            // the session's pending auth data, so it cannot be tampered
            // with by the client.
            "redirecturl" => sprintf("/tfaemailotp/%s", $result['username']),
            // Server-side only fields (not exposed to client):
            "otp" => $this->hashOtp($otp),
            "expiresAt" => time() + self::OTP_TTL
        ];

        return $result;
    }

    final public function verifyChallenge($params, $context, $result)
    {
        $username = $params['username'];
        $challengeResponse = $params['challengeresponse'];
        $challengeData = $params['challengedata'];

        if (time() > ($challengeData['expiresAt'])) {
            $result['verified'] = false;
        } else {
            $result['verified'] = $this->verifyOtp(
                $challengeResponse['code'],
                $challengeData['otp']
            );
        }

        return $result;
    }

    public function getItemList($params, $context)
    {
        return \OMV\Rpc\Rpc::call("Config", "getlist", [
            "id" => "conf.system.tfaemailotp",
            ...$params
        ], $context);
    }

    public function getItem($params, $context)
    {
        return \OMV\Rpc\Rpc::call("Config", "get", [
            "id" => "conf.system.tfaemailotp",
            "uuid" => $params['uuid']
        ], $context);
    }

    public function getByContext($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);

        $this->assertHasEmailAddress($context['username']);

        try {
            $object = $this->getItemByContext($context);
        } catch (\Exception $e) {
            $object = new \OMV\Config\ConfigObject("conf.system.tfaemailotp");
            $object->set("name", $context['username']);
        }

        return $object->getAssoc();
    }

    public function setItem($params, $context)
    {
        return \OMV\Rpc\Rpc::call("Config", "set", [
            "id" => "conf.system.tfaemailotp",
            "data" => $params
        ], $context);
    }

    public function deleteItem($params, $context)
    {
        return \OMV\Rpc\Rpc::call("Config", "delete", [
            "id" => "conf.system.tfaemailotp",
            "uuid" => $params['uuid']
        ], $context);
    }

    /**
     * Send a test OTP to the user's email address. Used during the enable
     * flow so the user can verify the setup before committing.
     */
    public function sendOtpByContext($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);

        $this->assertEmailNotificationEnabled();
        $this->assertHasEmailAddress($context['username']);

        $otp = $this->generateOtp();
        $this->sendOtp($context['username'], $otp);

        return [
            "otpHash" => $this->hashOtp($otp),
            "expiresAt" => time() + self::OTP_TTL
        ];
    }

    public function enableByContext($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);
        $this->validateMethodParams($params, "rpc.tfaemailotp.enableByContext");

        $this->assertEmailNotificationEnabled();
        $this->assertHasEmailAddress($context['username']);

        if (time() > ($params['expiresAt'])) {
            throw new \OMV\Exception(
                "The verification code has expired. Please request a new one."
            );
        }

        if (!$this->verifyOtp($params['code'], $params['otpHash'])) {
            throw new \OMV\Exception("Invalid verification code.");
        }

        try {
            $object = $this->getItemByContext($context);
        } catch (\Exception $e) {
            $object = new \OMV\Config\ConfigObject("conf.system.tfaemailotp");
            $object->set("name", $context['username']);
        }
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
        $this->validateMethodParams($params, "rpc.tfaemailotp.disableByContext");

        $this->assertEmailNotificationEnabled();
        $this->assertHasEmailAddress($context['username']);

        if (time() > ($params['expiresAt'])) {
            throw new \OMV\Exception(
                "The verification code has expired. Please request a new one."
            );
        }

        if (!$this->verifyOtp($params['code'], $params['otpHash'])) {
            throw new \OMV\Exception("Invalid verification code.");
        }

        try {
            $object = $this->getItemByContext($context);
        } catch (\Exception $e) {
            throw new \OMV\Exception(
                "2FA Email OTP is not configured for this account."
            );
        }

        $object->set("enabled", false);

        $db = \OMV\Config\Database::getInstance();
        $db->set($object);
    }

    private function isEnabled(string $username): bool
    {
        try {
            $db = \OMV\Config\Database::getInstance();
            $object = $db->getByFilter("conf.system.tfaemailotp", [
                'operator' => 'stringEquals',
                'arg0' => 'name',
                'arg1' => $username,
            ], 1);
            return $object->get("enabled");
        } catch (\Exception $e) {
            return false;
        }
    }

    private function assertEmailNotificationEnabled(): void
    {
        try {
            $db = \OMV\Config\Database::getInstance();
            $object = $db->get("conf.system.notification.email");
            $enabled = $object->get("enable");
        } catch (\Exception $e) {
            $enabled = false;
        }
        if (!$enabled) {
            throw new \OMV\Exception(
                gettext("Notifications need to be enabled on this system. Please contact the administrator.")
            );
        }
    }

    private function hasEmailAddress(string $username): string
    {
        if ($username == \OMV\Environment::get("OMV_WEBGUI_ADMINUSER_NAME", "admin")) {
            return true;
        }
        try {
            $db = \OMV\Config\Database::getInstance();
            $object = $db->getByFilter("conf.system.usermngmnt.user", [
                'operator' => 'stringEquals',
                'arg0' => 'name',
                'arg1' => $username,
            ], 1);
            return !empty($object->get("email"));
        } catch (\Exception $e) {
            return false;
        }
    }

    private function assertHasEmailAddress(string $username): void
    {
        if (!$this->hasEmailAddress($username)) {
            throw new \OMV\Exception(
                gettext("No email address is configured for this account.")
            );
        }
    }

    private function generateOtp(): string
    {
        $max = (int) str_repeat('9', self::OTP_DIGITS);
        return str_pad(
            (string) random_int(0, $max),
            self::OTP_DIGITS,
            '0',
            STR_PAD_LEFT
        );
    }

    private function sendOtp(string $to, string $otp): void
    {
        $subject = "Your sign-in verification code";
        $message = sprintf(
            "Use this code to sign in\n\n" .
            "%s\n\n" .
            "Enter the code above on your device to complete the sign-in.\n" .
            "This code is valid for %d minutes.\n\n" .
            "If you did not request this code, you can safely ignore this email.\n" .
            "For the security of your account, do not share this code with anyone.",
            $otp,
            (int) (self::OTP_TTL / 60)
        );

        $mail = new \OMV\Email("root", $to, $subject, $message);
        $mail->assertSend();
    }

    private function hashOtp(string $otp): string
    {
        return password_hash($otp, PASSWORD_DEFAULT);
    }

    private function verifyOtp(string $code, string $hash): bool
    {
        return password_verify($code, $hash);
    }

    private function getItemByContext($context)
    {
        $db = \OMV\Config\Database::getInstance();
        return $db->getByFilter("conf.system.tfaemailotp", [
            'operator' => 'stringEquals',
            'arg0' => 'name',
            'arg1' => $context['username'],
        ], 1);
    }
}
