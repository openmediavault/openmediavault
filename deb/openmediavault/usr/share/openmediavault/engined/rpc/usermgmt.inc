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

class UserMgmt extends \OMV\Rpc\ServiceAbstract
{
    /**
     * Get the RPC service name.
     */
    public function getName()
    {
        return "UserMgmt";
    }

    /**
     * Initialize the RPC service.
     */
    public function initialize()
    {
        $this->registerMethod("authUser");
        $this->registerMethod("verifyChallenge");
        $this->registerMethod("enumerateSystemUsers");
        $this->registerMethod("enumerateUsers");
        $this->registerMethod("enumerateAllUsers");
        $this->registerMethod("enumerateSystemGroups");
        $this->registerMethod("enumerateGroups");
        $this->registerMethod("enumerateAllGroups");
        $this->registerMethod("getUserList");
        $this->registerMethod("getUser");
        $this->registerMethod("getUserByContext");
        $this->registerMethod("setUser");
        $this->registerMethod("setUserByContext");
        $this->registerMethod("deleteUser");
        $this->registerMethod("importUsers");
        $this->registerMethod("getGroupList");
        $this->registerMethod("getGroup");
        $this->registerMethod("setGroup");
        $this->registerMethod("deleteGroup");
        $this->registerMethod("importGroups");
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("setPasswordByContext");
        $this->registerMethod("getTotpStatusByContext");
        $this->registerMethod("generateTotpSecretByContext");
        $this->registerMethod("enableTotpByContext");
        $this->registerMethod("disableTotpByContext");
    }

    /**
     * Authenticate the given user.
     * @param params The method parameters containing the following fields:
     *   \em username The name of the user.
     *   \em password The password.
     * @param context The context of the caller.
     * @return An array containing the fields \em authenticated which is TRUE
     *   if authentication was successful, otherwise FALSE. The name of the
     *   user is in \em username.
     */
    final public function authUser($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.authuser");
        // Authenticate the user.
        $authenticated = false;
        $user = new \OMV\System\User($params['username']);
        $permissions = [];
        if ($user->exists()) {
            if (false === ($authenticated = $user->authenticate(
                $params['password']
            ))) {
                goto finish;
            }
            // Deny access for inactive user accounts.
            if (true === $user->getInactive()) {
                $authenticated = false;
                goto finish;
            }
            // Is the user a WebUI administrator?
            // Note, this check must be carried out before checking
            // whether the user is a system account.
            if (in_array(
                \OMV\Environment::get("OMV_WEBGUI_ADMINGROUP_NAME"),
                $user->getGroups()
            )) {
                $permissions['role'] = "admin";
                goto finish;
            }
            // Deny access for 'system' user accounts.
            if (true === $user->isSystemAccount()) {
                $authenticated = false;
                goto finish;
            }
            $permissions['role'] = "user";
        }
        finish:
            return [
                "authenticated" => $authenticated,
                "username" => $params['username'],
                "permissions" => $permissions
            ];
    }

    /**
     * Verify a user challenge (e.g., MFA, TOTP, WebAuthn).
     * This is an extension point for authentication plugins.
     * @param params The method parameters containing the following fields:
     *   \em username The name of the user.
     *   \em challengeresponse The challenge response (format depends on challenge type).
     *   \em challengedata The challenge data from the authenticate step.
     * @param context The context of the caller.
     * @return An array with field \em verified (TRUE if verification successful).
     */
    public function verifyChallenge($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        $this->validateMethodParams($params, "rpc.usermgmt.verifychallenge");

        // Default: verification not passed. Plugins can register mutating
        // methods to override this behavior.
        return [
            "verified" => false
        ];
    }

    /**
     * Enumerate users.
     * @param string $type The user type, e.g. `system`, `normal` or `all`.
     * @param string $detail Specifies the details level: `basic` or `full`.
     *   Defaults to `basic`.
     * @return An array of associative arrays with the following user information:
     *   name, UID, GID, comment, home directory, and shell program,
     *   last changed, minimum, maximum, warn, inactive, expire, reserved,
     *   groups (array of group names) and system.
     */
    private function enumerateUsersByType($type, $detail = "basic")
    {
        $result = [];
        $users = \OMV\System\User::enumerateUsers();
        foreach ($users as $userk => $userv) {
            // Append user 'root'?
            if ((0 == strcasecmp("root", $userv->getName())) &&
              (false == \OMV\Environment::getBoolean(
                  "OMV_USERMGMT_ENUMERATE_USER_ROOT",
                  true
              ))) {
                continue;
            }
            // Check if the current user is requested.
            $append = false;
            switch ($type) {
                case "system":
                    $append = $userv->isSystemAccount();
                    break;
                case "normal":
                    $append = !$userv->isSystemAccount();
                    break;
                case "all":
                    $append = true;
                    break;
            }
            if (false === $append) {
                continue;
            }
            $result[] = $userv->getAssoc($detail);
        }
        return $result;
    }

    /**
     * Get the list of system users.
     * @param params An array containing the following fields:
     *   \em detail Specifies the detail level of the user information:
     *     `basic` or `full`. Defaults to `basic`.
     * @param context The context of the caller.
     * @return A list of user objects.
     */
    public function enumerateSystemUsers($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        $this->validateMethodParams($params, "rpc.usermgmt.enumerate");
        $detail = array_value($params, "detail", "basic");
        return $this->enumerateUsersByType("system", $detail);
    }

    /**
     * Get the list of non-system users.
     * @param params An array containing the following fields:
     *   \em detail Specifies the detail level of the user information:
     *     `basic` or `full`. Defaults to `basic`.
     * @param context The context of the caller.
     * @return A list of user objects.
     */
    public function enumerateUsers($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        $this->validateMethodParams($params, "rpc.usermgmt.enumerate");
        $detail = array_value($params, "detail", "basic");
        return $this->enumerateUsersByType("normal", $detail);
    }

    /**
     * Get the list of all users.
     * @param params An array containing the following fields:
     *   \em detail Specifies the detail level of the user information:
     *     `basic` or `full`. Defaults to `basic`.
     * @param context The context of the caller.
     * @return A list of user objects.
     */
    public function enumerateAllUsers($params, $context)
    {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        $this->validateMethodParams($params, "rpc.usermgmt.enumerate");
        $detail = array_value($params, "detail", "basic");
        return $this->enumerateUsersByType("all", $detail);
    }

    /**
     * Enumerate groups.
     * @param type The group type, e.g. `system`, `normal` or `all`.
     * @return An array of associative arrays with the following group information:
     *   name, GID, group members (array of user names) and system.
     */
    private function enumerateGroupsByType($type)
    {
        $result = [];
        $groups = \OMV\System\Group::enumerateGroups();
        foreach ($groups as $groupk => $groupv) {
            $append = false;
            switch ($type) {
                case "system":
                    $append = $groupv->isSystemAccount();
                    break;
                case "normal":
                    $append = !$groupv->isSystemAccount();
                    break;
                case "all":
                    $append = true;
                    break;
            }
            if (false === $append) {
                continue;
            }
            $result[] = $groupv->getAssoc();
        }
        return $result;
    }

    /**
     * Get the list of system groups.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return A list of group objects.
     */
    public function enumerateSystemGroups($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Get system groups.
        return $this->enumerateGroupsByType("system");
    }

    /**
     * Get the list of non-system groups.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return A list of group objects.
     */
    public function enumerateGroups($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Get non-system groups.
        return $this->enumerateGroupsByType("normal");
    }

    /**
     * Get the list of all groups.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return A list of group objects.
     */
    public function enumerateAllGroups($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Get all groups.
        return $this->enumerateGroupsByType("all");
    }

    /**
     * Get list of users (except system users).
     * @param params An array containing the following fields:
     *   \em start The index where to start.
     *   \em limit The number of objects to process.
     *   \em sortfield The name of the column used to sort.
     *   \em sortdir The sort direction, ASC or DESC.
     *   \em search The value to search for.
     *   \em $detail Specifies the detail level of the user information.
     *     Defaults to `basic`.
     * @param context The context of the caller.
     * @return An array containing the requested objects. The field \em total
     *   contains the total number of objects, \em data contains the object
     *   array. An exception will be thrown in case of an error.
     */
    public function getUserList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.getuserlist");
        // Get the list of non-system user.
        $detail = array_value($params, "detail", "basic");
        $users = $this->enumerateUsersByType("normal", $detail);
        // Get all users stored in the database and generate a lookup map.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->get("conf.system.usermngmnt.user");
        $objectsMap = [];
        foreach ($objects as $objectk => $objectv) {
            $objectsMap[$objectv->get("name")] = $objectv;
        }
        // Process users and append additional information that are stored
        // in the database.
        foreach ($users as $userk => &$userv) {
            // Set the defaults of the additional information.
            $userv = array_merge($userv, [
                "email" => "",
                "disallowusermod" => false,
                "sshpubkeys" => []
            ]);
            if (array_key_exists($userv['name'], $objectsMap)) {
                $object = $objectsMap[$userv['name']];
                $userv['email'] = $object->get("email");
                $userv['disallowusermod'] = $object->get("disallowusermod");
                if (false === $object->isEmpty("sshpubkeys")) {
                    $userv['sshpubkeys'] = $object->get("sshpubkeys.sshpubkey");
                }
            }
        }
        // Filter the result.
        return $this->applyFilter(
            $users,
            $params['start'],
            $params['limit'],
            $params['sortfield'],
            $params['sortdir'],
            $params['search']
        );
    }

    /**
     * Get an user configuration object.
     * @param params An array containing the following fields:
     *   \em name The name of the user.
     * @param context The context of the caller.
     * @return The requested configuration object.
     */
    public function getUser($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.getuser");
        // Ensure the user exists and is not a system account.
        $user = new \OMV\System\User($params['name']);
        $user->assertExists();
        $user->assertIsNotSystemAccount();
        // Get the user information.
        $result = $user->getAssoc();
        // Set the defaults of the additional information.
        $result = array_merge($result, [
            "email" => "",
            "disallowusermod" => false,
            "sshpubkeys" => []
        ]);
        // Get additional user information stored in database.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->getByFilter("conf.system.usermngmnt.user", [
            "operator" => "stringEquals",
            "arg0" => "name",
            "arg1" => $user->getName()
        ]);
        if (!empty($objects)) {
            // Get the user configuration object. Due the fact that a user
            // name is unique, we can simply use the first found object.
            $object = $objects[0];
            // Append additional information.
            $result['email'] = $object->get("email");
            $result['disallowusermod'] = $object->get("disallowusermod");
            if (false === $object->isEmpty("sshpubkeys")) {
                $result['sshpubkeys'] = $object->get("sshpubkeys.sshpubkey");
            }
        }
        return $result;
    }

    /**
     * Get the user configuration from the current context user. The
     * returned object contains only a limited number of user information.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return The result object contains the fields \em name, \em comment,
     *   \em email, \em sshpubkeys and the internal field \em _readonly.
     */
    public function getUserByContext($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);
        // Get the user configuration object.
        $object = $this->callMethod("getUser", [
            "name" => $context['username']
        ], $this->getAdminContext());
        // Prepare the result object.
        return [
            "name" => $object['name'],
            "comment" => $object['comment'],
            "email" => $object['email'],
            "sshpubkeys" => $object['sshpubkeys'],
            // Hijack the '_readonly' flag to set the form to read-only.
            "_readonly" => $object['disallowusermod']
        ];
    }

    /**
     * Set (add/update) a non-system user account.
     * @param params An array containing the following fields:
     *   \em name The name of the user.
     *   \em uid The user ID. This field is optional.
     *   \em groups A list of groups which the user is a member of as an
     *     array of strings.
     *   \em shell The name of the users login shell. This field is optional.
     *   \em password The plain password to use.
     *   \em email The users email address.
     *   \em comment Any text string. This field is optional.
     *   \em disallowusermod Set to TRUE to disallow the user from modifying
     *     their account from within the web interface.
     *   \em sshpubkeys The users SSH public keys.
     * @param context The context of the caller.
     * @return The stored configuration object.
     */
    public function setUser($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.setuser");
        // Check if the given user is a system account. Abort this attempt.
        $user = new \OMV\System\User($params['name']);
        if ($user->exists() && $user->isSystemAccount()) {
            throw new \OMV\Exception(
                "Unauthorized attempt to modify the system user account '%s'",
                $params['name']
            );
        }
        // Convert all public SSH keys to RFC 4716 format and make sure
        // that they are all unique.
        foreach ($params['sshpubkeys'] as $sshPubKeyk => &$sshPubKeyv) {
            $pubKey = new \OMV\Ssh\PublicKey($sshPubKeyv);
            $sshPubKeyv = $pubKey->toRfc4716();
        }
        $params['sshpubkeys'] = array_values(array_unique(
            $params['sshpubkeys']
        ));
        // Try to get existing configuration object.
        $filter = [
            "operator" => "stringEquals",
            "arg0" => "name",
            "arg1" => $params['name']
        ];
        $db = \OMV\Config\Database::getInstance();
        // Does the user already exist in the database?
        $oldObject = null;
        if ($db->exists("conf.system.usermngmnt.user", $filter)) {
            $notifyType = OMV_NOTIFY_MODIFY;
            // Get the user configuration object. Since the name of a user
            // is unique, we can simply use the first object found.
            $object = $oldObject = $db->getByFilter(
                "conf.system.usermngmnt.user",
                $filter,
                1
            );
            $object->set("email", $params['email']);
            $object->set("disallowusermod", $params['disallowusermod']);
            $object->set("email", $params['email']);
            $object->set("sshpubkeys", [
                "sshpubkey" => $params['sshpubkeys']
            ]);
        } else {
            $notifyType = OMV_NOTIFY_CREATE;
            // Create a new user configuration object.
            $object = new \OMV\Config\ConfigObject(
                "conf.system.usermngmnt.user"
            );
            $object->setNew();
            $object->set("name", $params['name']);
            $object->set("email", $params['email']);
            $object->set("disallowusermod", $params['disallowusermod']);
            $object->set("email", $params['email']);
            $object->set("sshpubkeys", [
                "sshpubkey" => $params['sshpubkeys']
            ]);
        }
        $db->set($object, true);
        // Prepare command arguments. Note, a user is always in the
        // group 'users'.
        $cmdArgs = [];
        $cmdArgs[] = "--gid";
        $cmdArgs[] = escapeshellarg(\OMV\Environment::get(
            "OMV_USERMGMT_DEFAULT_GROUP"
        ));
        if (array_key_exists("shell", $params) &&
                !empty($params['shell'])) {
            // Make sure the shell exists.
            if (!file_exists($params['shell'])) {
                throw new \OMV\Exception(
                    "The shell '%s' does not exist.",
                    $params['shell']
                );
            }
            $cmdArgs[] = "--shell";
            $cmdArgs[] = escapeshellarg($params['shell']);
        }
        if (array_key_exists("comment", $params)) {
            $cmdArgs[] = "--comment";
            $cmdArgs[] = escapeshellarg($params['comment']);
        }
        if (array_key_exists("groups", $params) &&
                !empty($params['groups'])) {
            // Make sure the specified groups exists. Note, the groups
            // are handled as group names and NOT as GIDs. This is the
            // only way to support numeric group names.
            // https://manpages.debian.org/buster/passwd/groupadd.8.en.html#CAVEATS
            foreach ($params['groups'] as $groupk => $groupv) {
                $group = new \OMV\System\Group($groupv);
                $group->assertExists();
            }
            $cmdArgs[] = "--groups";
            $cmdArgs[] = escapeshellarg(implode(",", $params['groups']));
        }
        // Does the user already exist?
        if (false === $user->exists()) {
            // Get user management settings.
            $hdsobject = $db->get("conf.system.usermngmnt.homedir");
            // Append additional arguments.
            if (array_key_exists("uid", $params)) {
                $cmdArgs[] = "--uid";
                $cmdArgs[] = $params['uid'];
            }
            if (true === $hdsobject->get("enable")) {
                // Get the absolute shared folder path.
                $sfpath = \OMV\Rpc\Rpc::call(
                    "ShareMgmt",
                    "getPath",
                    array("uuid" => $hdsobject->get("sharedfolderref")),
                    $context
                );
                $cmdArgs[] = "--create-home";
                $cmdArgs[] = "--home";
                $cmdArgs[] = escapeshellarg(build_path(
                    DIRECTORY_SEPARATOR,
                    $sfpath,
                    $params['name']
                ));
            }
            $cmdArgs[] = escapeshellarg($params['name']);
            // Create the new user.
            $cmd = new \OMV\System\Process("useradd", $cmdArgs);
            $cmd->setRedirect2to1();
            $cmd->execute();
        } else {
            // Append additional arguments.
            $cmdArgs[] = escapeshellarg($params['name']);
            // Modify the existing user.
            $cmd = new \OMV\System\Process("usermod", $cmdArgs);
            $cmd->setRedirect2to1();
            $cmd->execute();
        }
        // Update the password.
        if (array_key_exists("password", $params) &&
                !empty($params['password'])) {
            $user = new \OMV\System\User($params['name']);
            $user->assertExists();
            $user->changePassword($params['password']);
        }
        // Append some more information to the notification object.
        $objectAssoc = array_merge($object->getAssoc(), $params);
        // Notify configuration changes.
        $dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
        $dispatcher->notify(
            $notifyType,
            "org.openmediavault.conf.system.usermngmnt.user",
            $objectAssoc,
            !is_null($oldObject) ? $oldObject->getAssoc() : null
        );
        // Return the configuration object.
        return $objectAssoc;
    }

    /**
     * Set an user configuration object of the current context user.
     * @param params An array containing the following fields:
     *   \em password The password. Empty passwords are ignored.
     *   \em email The email address.
     *   \em comment The comment.
     * @param context The context of the caller.
     * @return void
     */
    public function setUserByContext($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.setuserbycontext");
        // Get the user configuration object.
        $object = $this->callMethod("getUser", [
            "name" => $context['username']
        ], $this->getAdminContext());
        // Make sure the user is allowed to modify their account.
        if (true === $object['disallowusermod']) {
            throw new \OMV\Exception(
                "Unauthorized attempt to modify the user account '%s'",
                $object['name']
            );
        }
        // Update the user configuration.
        return $this->callMethod("setUser", [
            "name" => $object['name'],
            "uid" => $object['uid'],
            "groups" => $object['groups'],
            "shell" => $object['shell'],
            "password" => array_value($params, 'password', ''),
            "email" => $params['email'],
            "comment" => $params['comment'],
            "disallowusermod" => $object['disallowusermod'],
            "sshpubkeys" => $object['sshpubkeys']
        ], $this->getAdminContext());
    }

    /**
     * Delete a user.
     * @param params An array containing the following fields:
     *   \em name The name of the user to delete.
     * @param context The context of the caller.
     * @return The deleted configuration object.
     */
    public function deleteUser($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.deleteuser");
        // Get the user configuration object (required for the notification
        // event).
        $object = $this->callMethod("getUser", array(
              "name" => $params['name']
          ), $this->getAdminContext());
        // Delete the user.
        $cmdArgs = [];
        $cmdArgs[] = "--force";
        $cmdArgs[] = escapeshellarg($params['name']);
        $cmd = new \OMV\System\Process("userdel", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->execute();
        // Delete additional user information stored in database.
        $db = \OMV\Config\Database::getInstance();
        $db->deleteByFilter("conf.system.usermngmnt.user", [
            "operator" => "stringEquals",
            "arg0" => "name",
            "arg1" => $params['name']
        ]);
        // Notify configuration changes.
        $dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.usermngmnt.user",
            $object
        );
        // Return the deleted configuration object.
        return $object;
    }

    /**
     * Import a list of users.
     * @param params An array containing the following fields:
     *   \em csv The user values, e.g.:
     *   name;uid;comment;email;password;group,group,...;disallowusermod
     *   test1;;comment;test1@xyz.com;foobarpwd1;adm;1
     *   test2;1200;comment2;test2@xyz.com;foobarpwd2;adm,audio,www-data;0
     *   test3;;comment3;test3@xyz.com;foobarpwd3;ssh;1
     * @param context The context of the caller.
     * @return void
     */
    public function importUsers($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.importuser");
        // Create the background process.
        return $this->execBgProc(function ($bgStatusFilename, $bgOutputFilename) use ($params, $context) {
            // Import users.
            $objects = [];
            $csv = explode("\n", $params['csv']);
            foreach ($csv as $linek => $linev) {
                $linev = trim($linev);
                # Skip comments.
                if (empty($linev) || ($linev[0] === "#")) {
                    continue;
                }
                # Parse line.
                # <name>;<uid>;<comment>;<email>;<password>;<shell>;<group,group,...>;<disallowusermod>
                if (1 !== preg_match(
                    "/^([^;]+);(\d*);([^;]*);".
                        "([^;]*);([^;]*);([^;]*);([^;]*);".
                        "(true|1|yes|y|on|false|0|no|n|off)$/i",
                    $linev,
                    $matches
                )) {
                    throw new \OMV\Exception("Invalid line: %s", $linev);
                }
                $object = [
                    "name" => $matches[1],
                    "comment" => $matches[3],
                    "email" => $matches[4],
                    "password" => $matches[5],
                    "groups" => !empty($matches[7]) ?
                        explode(",", $matches[7]) : [],
                    "disallowusermod" => boolvalEx($matches[8]),
                    "shell" => !empty($matches[6]) ?
                        $matches[6] : "/bin/sh",
                    "sshpubkeys" => []
                ];
                if (!empty($matches[2]) && is_numeric($matches[2])) {
                    $object['uid'] = intval($matches[2]);
                }
                // Do some checks:
                // - Check if an user with the given name already exists.
                $user = new \OMV\System\User($object['name']);
                $user->assertNotExists();
                // - Check if an user with the given UID already exists.
                if (array_key_exists("uid", $object)) {
                    $user = new \OMV\System\User($object['uid']);
                    $user->assertNotExists();
                }
                $objects[] = $object;
            }
            // Finally create the users.
            foreach ($objects as $objectk => $objectv) {
                $this->callMethod("setUser", $objectv, $context);
            }
            return "";
        });
    }

    /**
     * Get list of groups (except system groups).
     * @param params An array containing the following fields:
     *   \em start The index where to start.
     *   \em limit The number of objects to process.
     *   \em sortfield The name of the column used to sort.
     *   \em sortdir The sort direction, ASC or DESC.
     *   \em search The value to search for.
     * @param context The context of the caller.
     * @return An array containing the requested objects. The field \em total
     *   contains the total number of objects, \em data contains the object
     *   array. An exception will be thrown in case of an error.
     */
    public function getGroupList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.getlist");
        // Get the list of non-system groups.
        $groups = $this->enumerateGroupsByType("normal");
        // Get all groups stored in the database and generate a lookup map.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->get("conf.system.usermngmnt.group");
        $objectsMap = [];
        foreach ($objects as $objectk => $objectv) {
            $objectsMap[$objectv->get("name")] = $objectv;
        }
        // Process groups and append additional information that are stored
        // in the database.
        foreach ($groups as $groupk => &$groupv) {
            if (array_key_exists($groupv['name'], $objectsMap)) {
                $object = $objectsMap[$groupv['name']];
                $groupv['comment'] = $object->get("comment");
            }
        }
        // Filter result.
        return $this->applyFilter(
            $groups,
            $params['start'],
            $params['limit'],
            $params['sortfield'],
            $params['sortdir'],
            $params['search']
        );
    }

    /**
     * Get a group.
     * @param params An array containing the following fields:
     *   \em name The name of the group.
     * @param context The context of the caller.
     * @return The requested configuration object.
     */
    public function getGroup($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.getgroup");
        // Ensure the group exists and is not a system account.
        $group = new \OMV\System\Group($params['name']);
        $group->assertExists();
        $group->assertIsNotSystemAccount();
        // Get the group information.
        $result = $group->getAssoc();
        // Get additional group information stored in database.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->getByFilter("conf.system.usermngmnt.group", [
            "operator" => "stringEquals",
            "arg0" => "name",
            "arg1" => $group->getName()
        ]);
        if (!empty($objects)) {
            // Get the group configuration object. Due the fact that a group
            // name is unique, we can simply use the first found object.
            $object = $objects[0];
            // Append additional information.
            $result['comment'] = $object->get("comment");
        }
        return $result;
    }

    /**
     * Set (add/update) a non-system group account.
     * @param params An array containing the following fields:
     *   \em name The name of the group.
     *   \em gid The group ID. This field is optional.
     *   \em comment Any text string.
     *   \em members The group members as an array of user names.
     * @param context The context of the caller.
     * @return The stored configuration object.
     */
    public function setGroup($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.setgroup");
        // Check if the given group is a system account. Abort this attempt.
        $group = new \OMV\System\Group($params['name']);
        if ($group->exists() && $group->isSystemAccount()) {
            throw new \OMV\Exception(
                "Unauthorized attempt to modify the system group account '%s'",
                $params['name']
            );
        }
        // Try to get existing configuration object.
        $filter = [
            "operator" => "stringEquals",
            "arg0" => "name",
            "arg1" => $params['name']
        ];
        $db = \OMV\Config\Database::getInstance();
        // Does the group already exist in the database?
        $oldObject = null;
        if ($db->exists("conf.system.usermngmnt.group", $filter)) {
            $notifyType = OMV_NOTIFY_MODIFY;
            // Get the group configuration object. Since the name of a group
            // is unique, we can simply use the first object found.
            $object = $oldObject = $db->getByFilter(
                "conf.system.usermngmnt.group",
                $filter,
                1
            );
            $object->set("comment", $params['comment']);
        } else {
            $notifyType = OMV_NOTIFY_CREATE;
            // Create a new group configuration object.
            $object = new \OMV\Config\ConfigObject(
                "conf.system.usermngmnt.group"
            );
            $object->setNew();
            $object->set("name", $params['name']);
            $object->set("comment", $params['comment']);
        }
        $db->set($object);
        // Append some more information to the notification object.
        $objectAssoc = $object->getAssoc();
        // Does the group already exist?
        if (false === $group->exists()) {
            // Prepare command arguments.
            $cmdArgs = [];
            if (array_key_exists("gid", $params)) {
                $cmdArgs[] = "--gid";
                $cmdArgs[] = $params['gid'];
                // Append additional fields to configuration object for
                // the notification event.
                $objectAssoc['gid'] = $params['gid'];
            }
            $cmdArgs[] = escapeshellarg($params['name']);
            // Create the new group.
            $cmd = new \OMV\System\Process("groupadd", $cmdArgs);
            $cmd->setRedirect2to1();
            $cmd->execute();
        } else {
            // Nothing to do here.
        }
        // Process members.
        if (array_key_exists("members", $params)) {
            // Append additional fields to configuration object for
            // the notification event.
            $objectAssoc['members'] = $params['members'];
            // Append the given members to the group.
            $cmdArgs = [];
            $cmdArgs[] = "--members";
            $cmdArgs[] = escapeshellarg(implode(",", $params['members']));
            $cmdArgs[] = escapeshellarg($params['name']);

            $cmd = new \OMV\System\Process("gpasswd", $cmdArgs);
            $cmd->setRedirect2to1();
            $cmd->execute();
        }
        // Notify configuration changes.
        $dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
        $dispatcher->notify(
            $notifyType,
            "org.openmediavault.conf.system.usermngmnt.group",
            $objectAssoc,
            !is_null($oldObject) ? $oldObject->getAssoc() : null
        );
        // Return the configuration object.
        return $objectAssoc;
    }

    /**
     * Delete a group.
     * @param params An array containing the following fields:
     *   \em name The name of the group to delete.
     * @param context The context of the caller.
     * @return The deleted configuration object.
     */
    public function deleteGroup($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.deletegroup");
        // Get the group configuration object (required for the notification
        // event).
        $object = $this->callMethod("getGroup", [
            "name" => $params['name']
        ], $context);
        // Delete the group.
        $cmdArgs = [];
        $cmdArgs[] = "--only-if-empty";
        $cmdArgs[] = escapeshellarg($params['name']);
        $cmd = new \OMV\System\Process("delgroup", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->execute();
        // Delete configuration object.
        $db = \OMV\Config\Database::getInstance();
        $db->deleteByFilter("conf.system.usermngmnt.group", [
            "operator" => "stringEquals",
            "arg0" => "name",
            "arg1" => $params['name']
        ]);
        // Notify configuration changes.
        $dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_DELETE,
            "org.openmediavault.conf.system.usermngmnt.group",
            $object
        );
        // Return the deleted configuration object.
        return $object;
    }

    /**
     * Import a list of groups.
     * @param params An array containing the following fields:
     *   \em csv The group values, e.g.:
     *   name;gid;comment
     *   grp1;;comment1
     *   grp2;1200;comment2
     * @param context The context of the caller.
     * @return void
     */
    public function importGroups($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.importgroup");
        // Create the background process.
        return $this->execBgProc(function ($bgStatusFilename, $bgOutputFilename) use ($params, $context) {
            // Import groups.
            $objects = [];
            $csv = explode("\n", $params['csv']);
            foreach ($csv as $linek => $linev) {
                $linev = trim($linev);
                # Skip comments.
                if (empty($linev) || ($linev[0] === "#")) {
                    continue;
                }
                # Parse line.
                if (1 !== preg_match(
                    "/^([^;]+);(\d*);([^;]*)$/i",
                    $linev,
                    $matches
                )) {
                    throw new \OMV\Exception("Invalid line: %s", $linev);
                }
                $object = [
                    "name" => $matches[1],
                    "comment" => $matches[3],
                    "members" => []
                ];
                if (!empty($matches[2]) && is_numeric($matches[2])) {
                    $object['gid'] = intval($matches[2]);
                }
                $objects[] = $object;
            }
            // Check if the users does not exist until now.
            foreach ($objects as $objectk => $objectv) {
                // Check if an group with the given name already exists.
                $group = new \OMV\System\Group($objectv['name']);
                $group->assertNotExists();
                // Check if an group with the given GID already exists.
                if (array_key_exists("gid", $object)) {
                    $group = new \OMV\System\Group($objectv['gid']);
                    $group->assertNotExists();
                }
            }
            // Finally create the groups.
            foreach ($objects as $objectk => $objectv) {
                $this->callMethod("setGroup", $objectv, $context);
            }
            return "";
        });
    }

    /**
     * Get user management settings.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return The requested configuration object.
     */
    public function getSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc("conf.system.usermngmnt.homedir");
    }

    /**
     * Set user management settings.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return The stored configuration object.
     */
    public function setSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.usermgmt.setsettings");
        // Prepare the configuration object.
        $object = new \OMV\Config\ConfigObject(
            "conf.system.usermngmnt.homedir"
        );
        $object->setAssoc($params);
        // Set the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $db->set($object);
        // Build the home directory path.
        $sfPath = "";
        if (true === $object->get("enable")) {
            // Get the absolute shared folder path.
            $sfPath = \OMV\Rpc\Rpc::call("ShareMgmt", "getPath", [
                "uuid" => $object->get("sharedfolderref")
            ], $context);
        }
        // Update the non-system user.
        $users = $this->enumerateUsersByType("normal");
        $ld = \OMV\System\System::getLoginDefs();
        foreach ($users as $userk => $userv) {
            // Only process users that are in group 'users'.
            $defaultGroup = \OMV\Environment::get(
                "OMV_USERMGMT_DEFAULT_GROUP",
                "users"
            );
            if (!in_array($defaultGroup, $userv['groups'])) {
                continue;
            }
            $cmdArgs = [];
            if (!empty($sfPath)) {
                $homeDirPath = build_path(
                    DIRECTORY_SEPARATOR,
                    $sfPath,
                    $userv['name']
                );
                // Set new home directory. Move the content from the old to
                // the new home directory if this does not exist.
                // Note, the usermod command throws an error when calling
                // with --move-home and the home directory already exists.
                if (!file_exists($homeDirPath)) {
                    $cmdArgs[] = "--move-home";
                }
                $cmdArgs[] = "--home";
                $cmdArgs[] = escapeshellarg($homeDirPath);
            } else {
                // Skip the user if they do not yet have a home directory.
                if (empty($userv['dir'])) {
                    continue;
                }
                // Unset home directory.
                // See https://unix.stackexchange.com/a/617552
                //
                // ToDo:
                // Make use of an empty dir path again when the fix at
                // https://github.com/shadow-maint/shadow/pull/1133 is
                // available in Debian.
                $cmdArgs[] = "--home";
                $cmdArgs[] = array_value($ld, "NONEXISTENT", "/nonexistent");
            }
            $cmdArgs[] = escapeshellarg($userv['name']);
            $cmd = new \OMV\System\Process("usermod", $cmdArgs);
            $cmd->setRedirect2to1();
            $cmd->execute();
            // Create the home directory if it does not exist until now.
            if (!empty($sfPath)) {
                $homeDirPath = build_path(
                    DIRECTORY_SEPARATOR,
                    $sfPath,
                    $userv['name']
                );
                if (!file_exists($homeDirPath)) {
                    if (!mkdir($homeDirPath, 0700, true)) {
                        throw new \OMV\Exception(
                            "Failed to create home directory '%s': %s",
                            $homeDirPath,
                            last_error_msg()
                        );
                    }
                    if (!chown($homeDirPath, $userv['name'])) {
                        throw new \OMV\Exception(
                            "Failed to change owner of the directory '%s' ".
                          "to '%s': %s",
                            $homeDirPath,
                            $userv['name'],
                            last_error_msg()
                        );
                    }
                    if (!chgrp($homeDirPath, \OMV\Environment::get(
                        "OMV_USERMGMT_DEFAULT_GROUP"
                    ))) {
                        throw new \OMV\Exception(
                            "Failed to change group of the directory '%s' ".
                          "to '%s': %s",
                            $homeDirPath,
                            \OMV\Environment::get("OMV_USERMGMT_DEFAULT_GROUP"),
                            last_error_msg()
                        );
                    }
                }
            }
        }
        // Notify configuration changes.
        $dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.conf.system.usermanagement.homedirectory",
            $object->getAssoc()
        );
        // Return the configuration object.
        return $object->getAssoc();
    }

    /**
     * Get the TOTP MFA status for the currently logged-in user.
     * @param params  Unused.
     * @param context The context of the caller.
     * @return An array with:
     *   \em enabled TRUE if TOTP is currently active for this user.
     */
    public function getTotpStatusByContext($params, $context)
    {
        // Any authenticated user can query their own MFA status.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_EVERYONE]);
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->getByFilter('conf.system.mfa.totp', [
            'operator' => 'stringEquals',
            'arg0' => 'username',
            'arg1' => $context['username'],
        ]);
        $enabled = !empty($objects) && (bool) $objects[0]->get('enabled');
        return ["enabled" => $enabled];
    }

    /**
     * Generate a fresh TOTP secret and provisioning URI for the current user.
     *
     * The secret is NOT persisted at this point — the client must present a
     * valid code derived from it and call enableTotpByContext() to confirm
     * that the authenticator app was set up correctly before saving.
     *
     * @param params  Unused.
     * @param context The context of the caller.
     * @return An array with:
     *   \em secret    The raw base32-encoded secret (for manual entry in apps).
     *   \em uri       The otpauth:// URI encoded into the QR code.
     *   \em qrDataUrl A data URL (image/svg+xml) containing the QR code image
     *                 ready to be used directly as an <img src="..."> value.
     */
    public function generateTotpSecretByContext($params, $context)
    {
        // Any authenticated user can initiate MFA setup for themselves.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_EVERYONE]);
        $secret = \OMV\Totp::generateSecret();
        $prd = new \OMV\ProductInfo();
        $uri = \OMV\Totp::getProvisioningUri(
            $secret,
            $context['username'],
            $prd->getName()
        );
        $qrDataUrl = \OMV\Totp::generateQrDataUrl($uri);
        return [
            "secret" => $secret,
            "uri" => $uri,
            "qrDataUrl" => $qrDataUrl,
        ];
    }

    /**
     * Enable TOTP MFA for the current user after verifying the setup code.
     *
     * The client must first call generateTotpSecretByContext() to obtain a
     * provisional secret, configure the authenticator app, and then submit a
     * valid code together with that same secret here. This two-step approach
     * ensures we never store a secret the user cannot actually use.
     *
     * @param params An array containing:
     *   \em code   The 6-digit TOTP code from the authenticator app.
     *   \em secret The base32-encoded provisional secret.
     * @param context The context of the caller.
     * @return void
     */
    public function enableTotpByContext($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_EVERYONE]);
        $this->validateMethodParams(
            $params,
            "rpc.usermgmt.enabletotpbycontext"
        );
        // Verify the submitted code against the provisional secret before
        // persisting anything. This confirms the app is correctly configured.
        if (!\OMV\Totp::verifyCode($params['secret'], trim($params['code']))) {
            throw new \OMV\Exception("Invalid verification code. ".
                "Please ensure your authenticator app is set up correctly.");
        }
        $db = \OMV\Config\Database::getInstance();
        $filter = [
            "operator" => "stringEquals",
            "arg0" => "username",
            "arg1" => $context['username'],
        ];
        if ($db->exists("conf.system.mfa.totp", $filter)) {
            // Update the existing TOTP record for this user.
            $object = $db->getByFilter("conf.system.mfa.totp", $filter, 1);
            $object->set("enabled", true);
            $object->set("secret", $params['secret']);
        } else {
            // Create a new TOTP record for this user.
            $object = new \OMV\Config\ConfigObject("conf.system.mfa.totp");
            $object->setNew();
            $object->set("username", $context['username']);
            $object->set("enabled", true);
            $object->set("secret", $params['secret']);
        }
        $db->set($object);
    }

    /**
     * Disable TOTP MFA for the current user after verifying the current code.
     *
     * Requiring the current TOTP code prevents an attacker with access to an
     * unlocked browser session from silently removing MFA protection.
     *
     * @param params An array containing:
     *   \em code The 6-digit TOTP code from the authenticator app.
     * @param context The context of the caller.
     * @return void
     */
    public function disableTotpByContext($params, $context)
    {
        $this->validateMethodContext($context, ["role" => OMV_ROLE_EVERYONE]);
        $this->validateMethodParams(
            $params,
            "rpc.usermgmt.disabletotpbycontext"
        );
        // Retrieve the stored secret and verify the submitted code before
        // making any changes, so a stolen session cannot disable MFA.
        $db = \OMV\Config\Database::getInstance();
        $filter = [
            'operator' => 'stringEquals',
            'arg0' => 'username',
            'arg1' => $context['username'],
        ];
        $mfaObjects = $db->getByFilter('conf.system.mfa.totp', $filter);
        $secret = !empty($mfaObjects)
            ? (string) $mfaObjects[0]->get('secret')
            : '';
        if (empty($secret)) {
            throw new \OMV\Exception(
                "TOTP is not configured for this account."
            );
        }
        if (!\OMV\Totp::verifyCode($secret, trim($params['code']))) {
            throw new \OMV\Exception("Invalid verification code.");
        }
        // Remove the TOTP record from the database.
        $db = \OMV\Config\Database::getInstance();
        $db->deleteByFilter("conf.system.mfa.totp", [
            "operator" => "stringEquals",
            "arg0" => "username",
            "arg1" => $context['username'],
        ]);
    }

    /**
     * Set the password of the current context user.
     * @param params An array containing the following fields:
     *   \em password The new password.
     * @param context The context of the caller.
     * @return void
     */
    public function setPasswordByContext($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            "rpc.usermgmt.setpasswordbycontext"
        );
        $user = new \OMV\System\User($context['username']);
        $user->assertExists();
        $user->changePassword($params['password']);
        // Notify configuration changes if the given user exists in the
        // database. System users like 'admin' are ignored.
        $db = \OMV\Config\Database::getInstance();
        $filter = [
            "operator" => "stringEquals",
            "arg0" => "name",
            "arg1" => $user->getName()
        ];
        if (true === $db->exists("conf.system.usermngmnt.user", $filter)) {
            $object = $db->getByFilter(
                "conf.system.usermngmnt.user",
                $filter,
                1
            );
            // Append some more information to the notification object.
            $objectAssoc = array_merge($object->getAssoc(), [
                "password" => $params['password']
            ]);
            // Notify configuration changes.
            $dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
            $dispatcher->notify(
                OMV_NOTIFY_MODIFY,
                "org.openmediavault.conf.system.usermngmnt.user",
                $objectAssoc,
                $object->getAssoc()
            );
        }
    }
}
