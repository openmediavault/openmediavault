<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or any later version.
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

/**
 * RFC 6238 Time-based One-Time Password (TOTP) implementation.
 *
 * Produces 6-digit codes on a 30-second window using HMAC-SHA1,
 * compatible with Google Authenticator, Authy, Microsoft Authenticator,
 * 1Password, and other standard authenticator apps.
 *
 * No external dependencies — pure PHP with standard functions only.
 */
class Totp {
	/** Number of digits in each OTP code (RFC 6238 default). */
	const CODE_LENGTH = 6;

	/** Duration of each time window in seconds (RFC 6238 default). */
	const TIME_STEP = 30;

	/**
	 * How many adjacent windows to accept on each side of the current
	 * one. A value of 1 tolerates up to 30 seconds of clock skew
	 * between the authenticator app and the server.
	 */
	const ALLOWED_DRIFT = 1;

	/**
	 * Generate a cryptographically secure 160-bit (20-byte) random
	 * secret encoded as an uppercase base32 string (RFC 4648, no
	 * padding).
	 *
	 * @return string 32-character uppercase base32-encoded secret.
	 */
	public static function generateSecret(): string {
		return self::base32Encode(random_bytes(20));
	}

	/**
	 * Verify a 6-digit TOTP code submitted by the user.
	 *
	 * Accepts codes from ALLOWED_DRIFT windows before and after the
	 * current window to handle minor clock differences between the
	 * authenticator app and the server. Uses hash_equals() to prevent
	 * timing side-channel attacks.
	 *
	 * @param string $secret The base32-encoded TOTP secret for this user.
	 * @param string $code   The 6-digit code entered by the user.
	 * @return bool TRUE if the code is valid within the tolerance window.
	 */
	public static function verifyCode(string $secret, string $code): bool {
		// Reject obviously malformed codes before doing any crypto work.
		if (!preg_match('/^\d{6}$/', $code)) {
			return false;
		}
		$now = (int) floor(time() / self::TIME_STEP);
		for ($i = -self::ALLOWED_DRIFT; $i <= self::ALLOWED_DRIFT; $i++) {
			if (hash_equals(self::computeCode($secret, $now + $i), $code)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Build the otpauth:// provisioning URI to enroll an authenticator app.
	 *
	 * The URI encodes the secret, issuer, and algorithm parameters. It
	 * should be rendered as a QR code on the frontend so the user can
	 * scan it with their authenticator app.
	 *
	 * @param string $secret   The base32-encoded secret.
	 * @param string $username The account name shown in the authenticator.
	 * @param string $issuer   The service name shown in the authenticator.
	 * @return string Full otpauth:// URI.
	 */
	public static function getProvisioningUri(
		string $secret,
		string $username,
		string $issuer
	): string {
		$query = http_build_query([
			'secret'    => strtoupper($secret),
			'issuer'    => $issuer,
			'algorithm' => 'SHA1',
			'digits'    => self::CODE_LENGTH,
			'period'    => self::TIME_STEP,
		]);
		return sprintf(
			'otpauth://totp/%s:%s?%s',
			rawurlencode($issuer),
			rawurlencode($username),
			$query
		);
	}

	/**
	 * Compute the TOTP code for a given counter value (RFC 6238 / RFC 4226).
	 *
	 * Steps:
	 *  1. Decode the base32 secret into raw bytes.
	 *  2. Pack the counter as an 8-byte big-endian integer.
	 *  3. Compute HMAC-SHA1(key, counter).
	 *  4. Dynamic truncation: low nibble of last byte is the offset;
	 *     extract 4 bytes from that offset and mask the high bit.
	 *  5. Reduce modulo 10^CODE_LENGTH, zero-pad to CODE_LENGTH digits.
	 *
	 * @param string $secret  Uppercase base32-encoded TOTP secret.
	 * @param int    $counter Time counter (floor(unix_time / TIME_STEP)).
	 * @return string Zero-padded CODE_LENGTH-digit OTP string.
	 */
	private static function computeCode(
		string $secret,
		int $counter
	): string {
		$key = self::base32Decode(strtoupper(trim($secret)));

		// Encode counter as 8-byte big-endian (high 32 bits are 0 here).
		$timeBytes = pack('N*', 0) . pack('N*', $counter);

		$hmac = hash_hmac('sha1', $timeBytes, $key, /* raw */ true);

		// Offset is the low 4 bits of the last HMAC byte (RFC 4226 §5.4).
		$offset = ord($hmac[19]) & 0x0F;

		// Extract a 31-bit integer starting at $offset.
		$code = (
			((ord($hmac[$offset])     & 0x7F) << 24) |
			((ord($hmac[$offset + 1]) & 0xFF) << 16) |
			((ord($hmac[$offset + 2]) & 0xFF) <<  8) |
			 (ord($hmac[$offset + 3]) & 0xFF)
		) % (10 ** self::CODE_LENGTH);

		return str_pad((string) $code, self::CODE_LENGTH, '0', STR_PAD_LEFT);
	}

	/**
	 * Encode raw bytes as an uppercase base32 string (RFC 4648, no padding).
	 *
	 * @param string $data Raw binary input.
	 * @return string Uppercase base32-encoded string.
	 */
	private static function base32Encode(string $data): string {
		$alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567';
		$result   = '';
		$buffer   = 0;
		$bitsLeft = 0;

		foreach (str_split($data) as $byte) {
			$buffer    = ($buffer << 8) | ord($byte);
			$bitsLeft += 8;
			while ($bitsLeft >= 5) {
				$bitsLeft -= 5;
				$result   .= $alphabet[($buffer >> $bitsLeft) & 31];
			}
		}
		if ($bitsLeft > 0) {
			$result .= $alphabet[($buffer << (5 - $bitsLeft)) & 31];
		}
		return $result;
	}

	/**
	 * Decode a base32 string (RFC 4648) into raw binary bytes.
	 * Padding ('=') and unrecognized characters are silently skipped.
	 *
	 * @param string $data Uppercase base32-encoded string.
	 * @return string Decoded raw bytes.
	 */
	private static function base32Decode(string $data): string {
		$alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567';
		$result   = '';
		$buffer   = 0;
		$bitsLeft = 0;

		foreach (str_split($data) as $char) {
			$pos = strpos($alphabet, $char);
			if ($pos === false) {
				continue; // skip '=' padding and unrecognized chars
			}
			$buffer    = ($buffer << 5) | $pos;
			$bitsLeft += 5;
			if ($bitsLeft >= 8) {
				$bitsLeft -= 8;
				$result   .= chr(($buffer >> $bitsLeft) & 0xFF);
			}
		}
		return $result;
	}
}
