<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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

class Email extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "Email";
	}

	public function initialize() {
		$this->registerMethod("send");
		$this->registerMethod("sendBg");
		$this->registerMethod("sendByContext");
		$this->registerMethod("sendByContextBg");
	}

	/**
	 * Send an email.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @param params The method parameters containing the following fields:
	 *   \em from The sender email address.
	 *   \em to The receiver email address(es), e.g. 'test@xzy.com' or
	 *     'xzy@123.org,test@xzy.com'. Use the character , or ; as
	 *     seperator.
	 *   \em subject The subject of the email to be sent.
	 *   \em message The message to be sent.
	 * @return TRUE on success, otherwise FALSE.
	 */
	function send($params, $context) {
		$this->validateMethodParams($params, "rpc.email.send");
		$mail = new \OMV\Email($params['from'], $params['to'],
			$params['subject'], $params['message']);
		return [
			"success" => $mail->send()
		];
	}

	/**
	 * Execute the `send` RPC as background process.
	 */
	public function sendBg($params, $context) {
		return $this->callMethodBg("send", $params, $context);
	}

	/**
	 * Send an email. The sender email address is derived from the
	 * username of the caller context.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @param params The method parameters containing the following fields:
	 *   \em to The receiver email address(es), e.g. 'test@xzy.com' or
	 *     'xzy@123.org,test@xzy.com'. Use the character , or ; as
	 *     seperator.
	 *   \em subject The subject of the email to be sent.
	 *   \em message The message to be sent.
	 * @return TRUE on success, otherwise FALSE.
	 */
	function sendByContext($params, $context) {
		$params['from'] = $context['username'];
		return $this->callMethod("send", $params, $context);
	}

	/**
	 * Execute the `sendByContext` RPC as background process.
	 */
	public function sendByContextBg($params, $context) {
		return $this->callMethodBg("sendByContext", $params, $context);
	}
}
