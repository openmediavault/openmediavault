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
class OMVRpcServiceSession extends \OMV\Rpc\ServiceAbstract
{
    private const COOKIE_TTL_DAYS = 60;

    /**
     * Get the RPC service name.
     */
    final public function getName()
    {
        return "Session";
    }

    /**
     * Initialize the RPC service.
     */
    final public function initialize()
    {
        $this->registerMethod("login");
        $this->registerMethod("verify");
        $this->registerMethod("logout");
    }

    /**
     * Login user (step 1 of 2-step login).
     * @param params The method parameters containing the following fields:
     *   \em username The name of the user.
     *   \em password The password.
     * @param context The context of the caller.
     * @return An array containing the fields:
     *   \em status 'authenticated' if no challenge required, 'challengeRequired' if MFA needed.
     *   \em challenge Challenge info if required (kind, redirecturl, etc.).
     *   \em username The authenticated username.
     *   \em permissions User permissions (only on 'authenticated').
     */
    final public function login($params, $context)
    {
        $this->validateMethodParams($params, "rpc.session.login");

        $server = new \OMV\SuperGlobalServer();

        if (is_string($params['username'])) {
            $params['username'] = trim($params['username']);
        }

        $authResult = \OMV\Rpc\Rpc::call(
            "UserMgmt",
            "authUser",
            $params,
            $this->getAdminContext(),
            \OMV\Rpc\Rpc::MODE_REMOTE,
            true
        );

        if (!is_assoc_array($authResult) || !array_value(
            $authResult,
            "authenticated",
            false
        )) {
            $this->logAuthEvent(LOG_ALERT, sprintf(
                "Unauthorized login attempt from %s (username=%s, user-agent=%s)",
                $server->getClientIP(),
                $params['username'],
                $server->getUserAgent()
            ));
            throw new \OMV\HttpErrorException(
                400,
                _("Incorrect username or password.")
            );
        }

        $response = [
            "username" => $params['username'],
        ];

        $challenge = array_value($authResult, "challenge", null);
        if (!is_null($challenge)) {
            if (!is_assoc_array($challenge)) {
                throw new \OMV\HttpErrorException(
                    500,
                    _("Invalid authentication challenge format.")
                );
            }

            $challengeKind = array_value($challenge, 'kind', '');
            if (!is_string($challengeKind) || empty($challengeKind)) {
                throw new \OMV\HttpErrorException(
                    500,
                    _("Invalid authentication challenge kind.")
                );
            }

            $session = &\OMV\Session::getInstance();
            $_ = defer([$session, 'commit']);

            // Reject if an existing session belongs to a different user.
            if ($session->isAuthenticated() && $session->getUsername() !== $params['username']) {
                throw new \OMV\HttpErrorException(
                    400,
                    _("Another user is already authenticated.")
                );
            }

            $response['status'] = "challengeRequired";
            $response['challenge'] = array_pick_keys(
                $challenge,
                ['kind', 'redirecturl']
            );

            // Remember the in-progress login in the PHP session. It is bound
            // to the browser via the session cookie, so the subsequent
            // verify() call can pick it up. No separate token is exposed to
            // the client.
            $session->setPendingAuth(
                $params['username'],
                $authResult['permissions'],
                $challenge
            );
        } else {
            $response['status'] = "authenticated";
            $response['permissions'] = $authResult['permissions'];

            // No challenge required — initialize the PHP session directly so
            // subsequent RPCs pass validateAuthentication().
            $response['sessionid'] = $this->initializeSession(
                $params['username'],
                $authResult['permissions'],
                $server
            );

            $this->handleSuccessfulLogin($params['username'], $server);
        }

        return $response;
    }

    /**
     * Verify user challenge and complete login (step 2 of 2-step login).
     * The in-progress login is identified solely by the PHP session cookie
     * that was set during the authenticate() step.
     * @param params The method parameters containing the following fields:
     *   \em challengeresponse The challenge response (structure depends on challenge type).
     * @param context The context of the caller.
     * @return An array containing the fields:
     *   \em authenticated TRUE if verification successful.
     *   \em username The authenticated username.
     *   \em permissions User permissions.
     *   \em sessionid The session ID.
     */
    final public function verify($params, $context)
    {
        $this->validateMethodParams($params, "rpc.session.verify");

        $server = new \OMV\SuperGlobalServer();
        $session = &\OMV\Session::getInstance();
        $_ = defer([$session, 'commit']);

        $authData = $session->getPendingAuth();
        if (is_null($authData)) {
            $this->logAuthEvent(LOG_ALERT, sprintf(
                "No pending or expired login during challenge verification from %s",
                $server->getClientIP()
            ));
            throw new \OMV\HttpErrorException(
                401,
                _("No pending login or the login has expired.")
            );
        }

        $username = $authData['username'];
        $permissions = $authData['permissions'];
        $challenge = array_value($authData, "challenge");

        if (!is_null($challenge)) {
            try {
                $verifyResult = \OMV\Rpc\Rpc::call(
                    "UserMgmt",
                    "verifyChallenge",
                    [
                        "username" => $username,
                        "challengeresponse" => $params,
                        "challengedata" => $challenge
                    ],
                    $this->getAdminContext(),
                    \OMV\Rpc\Rpc::MODE_REMOTE,
                    true
                );

                if (!is_assoc_array($verifyResult) || !array_value(
                    $verifyResult,
                    "verified",
                    false
                )) {
                    $this->logAuthEvent(LOG_ALERT, sprintf(
                        "Challenge verification failed for %s from %s",
                        $username,
                        $server->getClientIP()
                    ));
                    throw new \OMV\HttpErrorException(
                        401,
                        _("Challenge verification failed.")
                    );
                }
            } catch (\OMV\HttpErrorException $e) {
                throw $e;
            } catch (\Exception $e) {
                $this->logAuthEvent(LOG_ERR, sprintf(
                    "Challenge verification error for %s: %s",
                    $username,
                    $e->getMessage()
                ));
                throw new \OMV\HttpErrorException(
                    500,
                    _("Challenge verification error.")
                );
            }
        }

        $sessionId = $this->initializeSession(
            $username,
            $permissions,
            $server
        );

        $this->handleSuccessfulLogin($username, $server);

        return [
            "status" => "authenticated",
            "username" => $username,
            "permissions" => $permissions,
            "sessionid" => $sessionId
        ];
    }


    /**
     * Logout user.
     */
    final public function logout($params, $context)
    {
        // Check the permissions and destroy the session.
        $session = &\OMV\Session::getInstance();
        $session->validate();
        $session->destroy();
    }

    /**
     * Log an authentication event to syslog with product-specific identity.
     * @param level The syslog priority level (e.g., LOG_ALERT, LOG_NOTICE).
     * @param message The message to log.
     * @return void
     */
    private function logAuthEvent($level, $message)
    {
        $prd = new \OMV\ProductInfo();
        $ident = mb_strtolower(sprintf("%s-webgui", $prd->getName()));

        openlog($ident, LOG_PID | LOG_PERROR, LOG_AUTH);
        $_ = defer("closelog");

        syslog($level, $message);
    }

    /**
     * Initialize (or reuse) the PHP session for the given user.
     * @param username The authenticated username.
     * @param permissions The user permissions array.
     * @param server The SuperGlobalServer instance.
     * @return The session ID.
     */
    private function initializeSession($username, $permissions, $server)
    {
        $session = &\OMV\Session::getInstance();

        if ($session->isAuthenticated()) {
            // Reject if an existing session belongs to a different user.
            if ($session->getUsername() !== $username) {
                throw new \OMV\HttpErrorException(
                    400,
                    _("Another user is already authenticated.")
                );
            }
            // Session already established for this user — reuse it.
            $session->clearPendingAuth();
            return $session->getId();
        }

        $role = ("admin" == array_value($permissions, "role", "user")) ?
            OMV_ROLE_ADMINISTRATOR : OMV_ROLE_USER;
        return $session->initialize($username, $role);
    }

    /**
     * Log a successful login via syslog and handle the first-login
     * notification email and browser cookie.
     * @param username The authenticated username.
     * @param server The SuperGlobalServer instance.
     * @return void
     */
    private function handleSuccessfulLogin($username, $server)
    {
        $this->logAuthEvent(LOG_ALERT, sprintf(
            "Authorized login from %s (username=%s, user-agent=%s)",
            $server->getClientIP(),
            $username,
            $server->getUserAgent()
        ));

        $prd = new \OMV\ProductInfo();
        $prefix = mb_strtoupper(sprintf("%s-LOGIN", $prd->getName()));

        // Send an email to the user if a new web browser is used to log
        // in to the control panel. If no special cookie is detected,
        // then it is assumed that the web browser has not been used to
        // log in until now. The salted user name hash is used as identifier.
        if ($this->isFirstLoginFromBrowser($username, $prefix)) {
            // Send the notification email to the user?
            try {
                $notificationEnabled = \OMV\Rpc\Rpc::call(
                    "Notification",
                    "isEnabled",
                    ["id" => "authentication"],
                    $this->getAdminContext(),
                    \OMV\Rpc\Rpc::MODE_REMOTE
                );
            } catch (\Exception $e) {
                // Force the sending of the notification email if the
                // RPC fails.
                $notificationEnabled = true;
            }

            if (true == $notificationEnabled) {
                $subject = sprintf("Your user account was used to log in to ".
                    "the %s control panel via a web browser.", $prd->getName());
                $message = sprintf(
                    "Dear %s,\n\n".
                    "your user account was used to log in to %s (%s) via a web browser.\n\n".
                    "Date and time: %s\n".
                    "Remote address: %s\n".
                    "User agent: %s\n\n".
                    "If you recently logged in to the control panel, you can disregard this email. ".
                    "If you have not logged in recently and believe someone may have accessed ".
                    "your account, you should reset your password.\n\n".
                    "Sincerely,\n".
                    "your %s system",
                    $username,
                    \OMV\System\Net\Dns::getFqdn(),
                    $server->getServerAddr(),
                    date("r"),
                    $server->getClientIP(),
                    $server->getUserAgent(),
                    $prd->getName()
                );
                $mail = new \OMV\Email(
                    "root",
                    $username,
                    $subject,
                    $message
                );
                $mail->send();
            }

            // Set a cookie. It will expire after 60 days. The user name
            // is crypted/hashed to be unable to reverse engineer the
            // account names from the cookie. The hash is hex-encoded
            // to ensure the cookie name only contains safe characters,
            // since PHP converts dots in cookie names to underscores
            // when populating $_COOKIE (bcrypt hashes may contain dots).
            $hashedUsername = password_hash(
                $username,
                PASSWORD_DEFAULT
            );
            $key = sprintf("%s-%s", $prefix, bin2hex($hashedUsername));
            $value = array_rand_value(\OMV\Environment::get(
                "OMV_DUNE_QUOTES"
            ));
            $expire = time() + (60 * 60 * 24 * self::COOKIE_TTL_DAYS);
            setcookie($key, $value, [
                'expires' => $expire,
                'httponly' => true,
                'samesite' => 'Strict'
            ]);
        }
    }

    /**
     * Check if this is the first login from the current browser for the given user.
     * @param username The username to check.
     * @param prefix The cookie name prefix.
     * @return TRUE if this is the first login from this browser, FALSE otherwise.
     */
    private function isFirstLoginFromBrowser($username, $prefix)
    {
        foreach ($_COOKIE as $cookiek => $cookiev) {
            // Extract the hex-encoded bcrypt hash from the cookie name.
            $regex = sprintf('/^%s-(.+)$/', $prefix);
            if (1 !== preg_match($regex, $cookiek, $matches)) {
                continue;
            }
            $hashedUsername = hex2bin($matches[1]);
            if (false === $hashedUsername) {
                continue;
            }
            if (password_verify($username, $hashedUsername)) {
                return false;
            }
        }
        return true;
    }
}
