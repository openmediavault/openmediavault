<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
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
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
namespace OMV;

/**
 * @ingroup api
 */
class Email {
	private $from;
	private $to;
	private $subject;
	private $message;

	/**
	 * Constructor.
	 * @param from The sender email address.
	 * @param to The receiver email address(es), e.g. 'test@xzy.com' or
	 *   'xzy@123.org,test@xzy.com'. Use the character , or ; as seperator.
	 *   Additionally this can be an array of email adresses.
	 * @param subject The subject.
	 * @param message The message.
	 */
	public function __construct($from, $to, $subject, $message) {
		list($this->from, $this->to, $this->subject,
		  $this->message) = func_get_args();
		// Modify the receiver list.
		if (TRUE === is_array($this->to))
			$this->$to = implode(',', $this->to);
		else
			// Prepare the receivers of the email. The formatting of this
			// string must comply with RFC 2822.
			$this->to = strtr($this->to, [ ';' => ',' ]);
	}

	/**
	 * Send the email.
	 * @return Return TRUE if successful, otherwise FALSE.
	 */
	public function send() {
		// Prepare additional email headers.
		$headers = [];
		// Set date in RFC 2882 format.
		$headers[] = sprintf('Date: %s', date('r'));
		// Append the sender address.
		if (!empty($this->from))
			$headers[] = sprintf('From: %s', $this->from);
		return mail($this->to, $this->subject, $this->message,
			implode(PHP_EOL, $headers));
	}

	/**
	 * Send the email.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertSend() {
		if (FALSE === $this->send()) {
			throw new \OMV\AssertException(
			  "Failed to send the email. Please check the logs.");
		}
	}
}
