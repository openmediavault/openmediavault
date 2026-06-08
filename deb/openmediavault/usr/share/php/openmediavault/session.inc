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
        $sessionId = array_value(
            $_SERVER,
            "HTTP_X_OPENMEDIAVAULT_SESSIONID",
            false
        );
        if (false !== $sessionId) {
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
                $params["path"],
                $params["domain"],
                $params["secure"],
                $params["httponly"]
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
        if (array_key_exists("REMOTE_ADDR", $_SERVER)) {
            $_SESSION['ipaddress'] = $_SERVER['REMOTE_ADDR'];
        }
        if (array_key_exists("HTTP_USER_AGENT", $_SERVER)) {
            $_SESSION['useragent'] = $_SERVER['HTTP_USER_AGENT'];
        }
        $this->updateLastAccess();
        return session_id();
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
     * Initialize a session with MFA verification pending.
     *
     * The session is associated with the validated username and role but
     * authenticated is left FALSE. Access to protected RPC methods is denied
     * until completeMfaAuthentication() is called after a successful TOTP
     * code verification.
     *
     * @param string $username The username whose password was already verified.
     * @param int    $role OMV_ROLE_ADMINISTRATOR or OMV_ROLE_USER.
     * @return string The new session ID to return to the client.
     */
    public function initializeMfaPending(string $username, int $role): string
    {
        session_regenerate_id();
        $_SESSION['authenticated'] = false;
        $_SESSION['mfa_pending'] = true;
        $_SESSION['totp_attempts'] = 0;
        $_SESSION['username'] = $username;
        $_SESSION['role'] = $role;
        if (array_key_exists("REMOTE_ADDR", $_SERVER)) {
            $_SESSION['ipaddress'] = $_SERVER['REMOTE_ADDR'];
        }
        if (array_key_exists("HTTP_USER_AGENT", $_SERVER)) {
            $_SESSION['useragent'] = $_SERVER['HTTP_USER_AGENT'];
        }
        $this->updateLastAccess();
        return session_id();
    }

    /**
     * Promote a pending-MFA session to fully authenticated after the user has
     * successfully verified their TOTP code.
     */
    public function completeMfaAuthentication(): void
    {
        $_SESSION['authenticated'] = true;
        $_SESSION['mfa_pending'] = false;
        unset($_SESSION['totp_attempts']);
        $this->updateLastAccess();
    }

    /**
     * Check whether this session is awaiting TOTP verification.
     * @return bool TRUE if MFA verification is still pending.
     */
    public function isMfaPending(): bool
    {
        return (bool) array_value($_SESSION, 'mfa_pending', false);
    }

    /**
     * Validate that this session is in the MFA-pending state.
     *
     * Called at the start of the verifyTotp() RPC to ensure the request
     * belongs to a session that completed password authentication and is
     * now waiting for the TOTP code. Destroys the session and throws on
     * any inconsistency.
     */
    public function validateMfaPending(): void
    {
        $this->validateTimeout();
        $this->validateUserAgent();
        if (!$this->isMfaPending()) {
            $this->destroy();
            throw new HttpErrorException(
                401,
                _("No MFA verification is pending for this session.")
            );
        }
    }

    /**
     * Increment the failed TOTP attempt counter and return the new total.
     * Used to enforce a maximum number of retries before the session is
     * invalidated to prevent brute-force attacks.
     * @return int Number of failed TOTP attempts so far (including this one).
     */
    public function incrementTotpAttempts(): int
    {
        $attempts = (int) array_value($_SESSION, 'totp_attempts', 0) + 1;
        $_SESSION['totp_attempts'] = $attempts;
        return $attempts;
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
        if (!array_key_exists("REMOTE_ADDR", $_SERVER)) {
            return;
        }
        if ($_SESSION['ipaddress'] == $_SERVER['REMOTE_ADDR']) {
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
        if (!array_key_exists("HTTP_USER_AGENT", $_SERVER)) {
            return;
        }
        if ($_SESSION['useragent'] == $_SERVER['HTTP_USER_AGENT']) {
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
            "id" => session_id(),
            "data" => $_SESSION
        ], true));
    }
}
