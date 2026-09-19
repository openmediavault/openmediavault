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

namespace OMV;

require_once("openmediavault/functions.inc");

/**
 * @ingroup api
 */
class Session
{
    public const PENDING_AUTH_MAX_DELAY_SECONDS = 300;

    private const PENDING_AUTH_TTL_SECONDS = 300;
    private const PENDING_AUTH_BASE_DELAY_SECONDS = 5;
    private const PENDING_AUTH_RESET_TIMEOUT_SECONDS = 900;
    private const PENDING_AUTH_ATTEMPT_TTL_SECONDS = 30;
    private const PENDING_AUTH_RATE_LIMIT_DIR = "/run/openmediavault/auth";

    /**
     * Returns the session singleton.
     * @return The session object.
     */
    public static function &getInstance()
    {
        static $instance = null;
        if (!isset($instance)) {
            $instance = new Session();
        }
        return $instance;
    }

    /**
     * Start session.
     */
    public function start()
    {
        $sessionId = array_value($_SERVER, "HTTP_X_OPENMEDIAVAULT_SESSIONID");
        if (is_string($sessionId) && ("" !== $sessionId)) {
            session_id($sessionId);
        }
        if (false === session_start()) {
            throw new HttpErrorException(
                500,
                _("Failed to start session.")
            );
        }
    }

    /**
     * Write session data and end session.
     */
    public function commit()
    {
        if (false === session_commit()) {
            throw new HttpErrorException(
                500,
                _("Failed to write session data.")
            );
        }
    }

    /**
     * Destroy session.
     */
    public function destroy()
    {
        // Destroy the session cookie.
        $params = session_get_cookie_params();
        if (is_assoc_array($params)) {
            setcookie(
                session_name(),
                "",
                time() - (60 * 60 * 24),
                $params['path'],
                $params['domain'],
                $params['secure'],
                $params['httponly']
            );
        }
        // Destroy all data registered to this session.
        session_destroy();
    }

    /**
     * Initialize the session.
     * @param username The user name.
     * @param role The user role.
     * @return The session id.
     */
    public function initialize($username, $role)
    {
        session_regenerate_id();
        $_SESSION['authenticated'] = true;
        $_SESSION['username'] = $username;
        $_SESSION['role'] = $role;
        $remoteAddr = array_value($_SERVER, "REMOTE_ADDR");
        if (!is_null($remoteAddr)) {
            $_SESSION['ipaddress'] = $remoteAddr;
        }
        $userAgent = array_value($_SERVER, "HTTP_USER_AGENT");
        if (!is_null($userAgent)) {
            $_SESSION['useragent'] = $userAgent;
        }
        $this->clearPendingAuth();
        $this->updateLastAccess();
        return $this->getId();
    }

    /**
     * Get the current session ID.
     * @return Returns the session id for the current session or the empty
     *   string ("") if there is no current session (no current session id
     *   exists). On failure, false is returned.
     */
    public function getId()
    {
        return session_id();
    }

    /**
     * Store the state of an in-progress login that still requires a
     * challenge (e.g. MFA/TOTP) to be solved before the session is
     * fully authenticated. The state is bound to the current PHP session,
     * i.e. it is only accessible to the browser that started the login
     * (identified by the session cookie).
     * @param username The name of the user that passed the first factor.
     * @param permissions The permissions to grant once the challenge is solved.
     * @param challenge The challenge payload provided by the auth plugin.
     * @return void
     */
    public function setPendingAuth($username, $permissions, $challenge)
    {
        session_regenerate_id(true);
        $_SESSION['pendingauth'] = [
            "username" => $username,
            "permissions" => $permissions,
            "challenge" => $challenge,
            "created" => time(),
            "expires" => time() + self::PENDING_AUTH_TTL_SECONDS
        ];
    }

    /**
     * Reserve an authentication challenge verification attempt for the
     * specified user.
     * @param username The user name.
     * @return A token, or NULL if verification is currently rate-limited.
     */
    public function reservePendingAuthAttempt($username): ?string
    {
        $jsonFile = new \OMV\Json\File($this->buildPendingAuthRateLimitFilename(
            $username
        ));
        $jsonFile->open("c+");
        $_ = defer([$jsonFile, 'close']);
        $data = $jsonFile->isEmpty() ? [] : $jsonFile->read();
        $now = time();
        if ($now < array_value($data, "expires", 0)) {
            return null;
        }
        $token = bin2hex(random_bytes(16));
        $jsonFile->write([
            "attempts" => array_value($data, "attempts", 0),
            "last_failure" => array_value($data, "last_failure", 0),
            "expires" => $now + self::PENDING_AUTH_ATTEMPT_TTL_SECONDS,
            "token" => $token
        ]);
        return $token;
    }

    /**
     * Finish an authentication challenge verification attempt.
     * @param username The user name.
     * @param token The attempt token.
     * @param success Set to TRUE if the verification succeeded.
     * @return The delay in seconds the user must wait before retrying.
     */
    public function finishPendingAuthAttempt($username, $token, $success): int
    {
        $jsonFile = new \OMV\Json\File($this->buildPendingAuthRateLimitFilename(
            $username
        ));
        if (!$jsonFile->exists()) {
            return 0;
        }
        $jsonFile->open("c+");
        $_ = defer([$jsonFile, 'close']);
        if ($jsonFile->isEmpty()) {
            return 0;
        }
        $data = $jsonFile->read();
        if ($token !== array_value($data, "token")) {
            return 0;
        }
        if ($success) {
            $jsonFile->unlink();
            return 0;
        }

        $now = time();
        $lastFailure = array_value($data, "last_failure", 0);
        $attempts = array_value($data, "attempts", 0);
        // Reset attempts if no failure occurred within the reset timeout.
        if ($now - $lastFailure > self::PENDING_AUTH_RESET_TIMEOUT_SECONDS) {
            $attempts = 0;
        }
        $attempts++;
        $delay = $this->calculatePendingAuthDelay($attempts);
        $jsonFile->write([
            "attempts" => $attempts,
            "last_failure" => $now,
            "expires" => ($delay > 0) ? $now + $delay : 0,
            "token" => ""
        ]);
        return $delay;
    }

    /**
     * Calculate the rate-limit delay in seconds using exponential backoff.
     * @param attempts The number of consecutive failed attempts.
     * @return The delay in seconds.
     */
    private function calculatePendingAuthDelay(int $attempts): int
    {
        if ($attempts <= 2) {
            return 0;
        }
        $exponent = min($attempts - 3, 10);
        return min(
            self::PENDING_AUTH_MAX_DELAY_SECONDS,
            self::PENDING_AUTH_BASE_DELAY_SECONDS * (2 ** $exponent)
        );
    }

    /**
     * Get the remaining rate-limit lockout time in seconds for the specified
     * user.
     * @param username The user name.
     * @return The remaining lockout time in seconds, or 0 if not rate-limited.
     */
    public function getPendingAuthRateLimitRemaining($username): int
    {
        $jsonFile = new \OMV\Json\File($this->buildPendingAuthRateLimitFilename(
            $username
        ));
        if (!$jsonFile->exists()) {
            return 0;
        }
        $jsonFile->open("r", LOCK_SH);
        $_ = defer([$jsonFile, 'close']);
        if ($jsonFile->isEmpty()) {
            return 0;
        }
        $expires = array_value($jsonFile->read(), "expires", 0);
        return max(0, $expires - time());
    }

    /**
     * Clear the authentication challenge rate limit state for the specified
     * user.
     * @param username The user name.
     * @return void
     */
    public function clearPendingAuthRateLimit($username)
    {
        $jsonFile = new \OMV\Json\File($this->buildPendingAuthRateLimitFilename(
            $username
        ));
        $jsonFile->unlink();
    }

    /**
     * Build the private filename used for a user's authentication challenge
     * rate limit state.
     * @param username The user name.
     * @return The filename.
     */
    private function buildPendingAuthRateLimitFilename($username): string
    {
        return sprintf(
            "%s/%s",
            self::PENDING_AUTH_RATE_LIMIT_DIR,
            hash("sha256", mb_strtolower($username))
        );
    }

    /**
     * Get the pending login state stored by `setPendingAuth()`. Returns
     * NULL if there is no pending login or if it has expired.
     * @return The pending auth state array or NULL.
     */
    public function getPendingAuth()
    {
        $data = array_value($_SESSION, "pendingauth");
        if (!is_assoc_array($data)) {
            return null;
        }
        $expires = array_value($data, "expires", 0);
        if (!is_numeric($expires) || (time() > $expires)) {
            $this->clearPendingAuth();
            return null;
        }
        return $data;
    }

    /**
     * Discard the pending login state.
     * @return void
     */
    public function clearPendingAuth()
    {
        unset($_SESSION['pendingauth']);
    }

    /**
     * Get the role of the user from this session.
     * @return Returns the user role, otherwise FALSE.
     */
    public function getRole()
    {
        return array_value($_SESSION, "role", false);
    }

    /**
     * Is this session authenticated?
     * @return Returns TRUE if the session is authenticated, otherwise FALSE.
     */
    public function isAuthenticated(): bool
    {
        return array_value($_SESSION, "authenticated", false);
    }

    /**
     * Get the name of the user from this session.
     * @return Returns the current user name, otherwise FALSE.
     */
    public function getUsername()
    {
        return array_value($_SESSION, "username", false);
    }

    /**
     * Update the time on which the last access took place.
     */
    public function updateLastAccess()
    {
        $_SESSION['lastaccess'] = time();
    }

    /**
     * Check if the last access if not older than the defined timeout value.
     * @return TRUE if the session is timed out, otherwise FALSE.
     */
    public function isTimeout(): bool
    {
        if (!isset($_SESSION['lastaccess'])) {
            return false;
        }
        try {
            // Get session timeout from the configuration.
            $db = &\OMV\Config\Database::getInstance();
            $object = $db->get("conf.webadmin");
            $timeout = $object->get("timeout") * 60;
        } catch (\Exception $e) {
            // Set default to 5 minutes.
            $timeout = 300;
        }
        // If the timeout value is set the 0 the session timeout
        // validation is disabled.
        if (0 == $timeout) {
            return false;
        }
        return ((time() - $_SESSION['lastaccess']) > $timeout) ? true : false;
    }

    /**
     * Validate the authentication. In case of an error the session is
     * destroyed and an exception is thrown.
     */
    public function validateAuthentication()
    {
        if (!$this->isAuthenticated()) {
            $this->destroy();
            throw new HttpErrorException(401, _("Session not authenticated."));
        }
    }

    /**
     * Validate the timeout. In case of an error the session is
     * destroyed and an exception is thrown.
     */
    public function validateTimeout()
    {
        if ($this->isTimeout()) {
            $this->destroy();
            throw new HttpErrorException(401, _("Session expired."));
        }
    }

    /**
     * Validate the user. In case of an error the session is
     * destroyed and an exception is thrown.
     */
    public function validateUser()
    {
        // Check if user still exists, maybe it has been deleted by the
        // administrator in the meanwhile.
        $userInfo = posix_getpwnam($_SESSION['username']);
        if (false === $userInfo) {
            $this->destroy();
            throw new HttpErrorException(401, _("Invalid user."));
        }
    }

    /**
     * Validate the IP address. In case of an error the session is
     * destroyed and an exception is thrown.
     */
    public function validateIpAddress()
    {
        $remoteAddr = array_value($_SERVER, "REMOTE_ADDR");
        if (is_null($remoteAddr)) {
            return;
        }
        if ($_SESSION['ipaddress'] == $remoteAddr) {
            return;
        }
        $this->destroy();
        throw new HttpErrorException(401, _("Invalid IP address."));
    }

    /**
     * Validate the browser user agent. In case of an error the session
     * is destroyed and an exception is thrown.
     */
    public function validateUserAgent()
    {
        $userAgent = array_value($_SERVER, "HTTP_USER_AGENT");
        if (is_null($userAgent)) {
            return;
        }
        if ($_SESSION['useragent'] == $userAgent) {
            return;
        }
        $this->destroy();
        throw new HttpErrorException(401, _("Invalid User-Agent."));
    }

    /**
     * Check if the session is valid. In case of an error the session is
     * destroyed and an exception is thrown.
     */
    public function validate()
    {
        $this->validateAuthentication();
        // $this->validateIpAddress();
        $this->validateUserAgent();
        $this->validateTimeout();
        $this->validateUser();
    }

    /**
     * Dump the current session information.
     */
    public function dump()
    {
        syslog(LOG_DEBUG, var_export([
            "id" => $this->getId(),
            "data" => $_SESSION
        ], true));
    }
}
