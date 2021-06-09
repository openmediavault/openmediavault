<?php

/**
 * @generate-function-entries
 * @generate-legacy-arginfo
 */

/** @param string $status */
function pam_auth(string $username, string $password, &$status = null, bool $checkacctmgmt = true, string $servicename = null): bool {}

/** @param string $status */
function pam_chpass(string $username, string $oldpassword, string $newpassword, &$status = null, string $servicename = null): bool {}

