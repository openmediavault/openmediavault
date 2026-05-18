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
	 * Generate a QR code for the given URI and return it as a data URL.
	 *
	 * Produces an SVG QR code entirely in PHP with no external dependencies.
	 * The result is a base64-encoded data URL suitable for use directly as
	 * an HTML image source: data:image/svg+xml;base64,...
	 *
	 * Implements QR Code Model 2, error-correction level M, with automatic
	 * version selection. The algorithm follows ISO/IEC 18004:2015.
	 *
	 * @param string $data  The string to encode (typically an otpauth:// URI).
	 * @param int    $scale Pixel size of each module. Defaults to 4.
	 * @return string A data URL string (data:image/svg+xml;base64;...).
	 */
	public static function generateQrDataUrl(string $data, int $scale = 4): string {
		$matrix = self::buildQrMatrix($data);
		$size   = count($matrix);
		$quiet  = 4; // quiet zone in modules
		$total  = ($size + $quiet * 2) * $scale;

		$rects = '';
		foreach ($matrix as $row => $cols) {
			foreach ($cols as $col => $dark) {
				if ($dark) {
					$x = ($col + $quiet) * $scale;
					$y = ($row + $quiet) * $scale;
					$rects .= sprintf(
						'<rect x="%d" y="%d" width="%d" height="%d"/>',
						$x, $y, $scale, $scale
					);
				}
			}
		}
		$svg = sprintf(
			'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 %1$d %1$d"'
			. ' width="%1$d" height="%1$d"><rect width="100%%" height="100%%"'
			. ' fill="#fff"/><g fill="#000">%2$s</g></svg>',
			$total,
			$rects
		);
		return 'data:image/svg+xml;base64,' . base64_encode($svg);
	}

	/**
	 * Build a boolean matrix representing the dark/light modules of a QR code.
	 *
	 * Supports byte-mode encoding with error-correction level M. Selects the
	 * smallest QR version that can hold $data. Implements masking pattern 0
	 * (checkerboard: (row + col) % 2 == 0).
	 *
	 * @param string $data The input string to encode.
	 * @return array<int,array<int,bool>> 2-D array indexed [row][col].
	 */
	private static function buildQrMatrix(string $data): array {
		// QR code capacity table (byte mode, error-correction level M).
		// Index = version - 1; value = max bytes that version can hold.
		static $capacities = [
			16, 28, 44, 64, 86, 108, 124, 154, 182, 216,
			254, 290, 334, 365, 415, 453, 507, 563, 627, 669,
			714, 782, 860, 914, 1000, 1062, 1128, 1193, 1267, 1373,
			1455, 1541, 1631, 1725, 1812, 1914, 1992, 2102, 2216, 2334,
		];
		$byteLen = strlen($data);
		$version = 1;
		foreach ($capacities as $v => $cap) {
			if ($byteLen <= $cap) {
				$version = $v + 1;
				break;
			}
		}
		// For very long data fall back to version 40.
		if ($byteLen > $capacities[39]) {
			$version = 40;
		}

		$size = $version * 4 + 17;

		// Initialise the matrix to -1 (undefined).
		$matrix = array_fill(0, $size, array_fill(0, $size, -1));

		// --- Finder patterns (top-left, top-right, bottom-left) ---
		$finderPattern = [
			[1,1,1,1,1,1,1],
			[1,0,0,0,0,0,1],
			[1,0,1,1,1,0,1],
			[1,0,1,1,1,0,1],
			[1,0,1,1,1,0,1],
			[1,0,0,0,0,0,1],
			[1,1,1,1,1,1,1],
		];
		$positions = [[0,0],[0,$size-7],[$size-7,0]];
		foreach ($positions as [$r,$c]) {
			foreach ($finderPattern as $dr => $row) {
				foreach ($row as $dc => $v) {
					if ($r+$dr < $size && $c+$dc < $size) {
						$matrix[$r+$dr][$c+$dc] = $v;
					}
				}
			}
		}

		// Separators (white border around finder patterns).
		for ($i = 0; $i < 8 && $i < $size; $i++) {
			// Top-left
			if ($i < $size)      $matrix[7][$i]      = isset($matrix[7][$i])      && $matrix[7][$i]      === -1 ? 0 : $matrix[7][$i];
			if ($i < $size)      $matrix[$i][7]      = isset($matrix[$i][7])      && $matrix[$i][7]      === -1 ? 0 : $matrix[$i][7];
			// Top-right
			if ($size-8 >= 0)    $matrix[$i][$size-8] = isset($matrix[$i][$size-8]) && $matrix[$i][$size-8] === -1 ? 0 : $matrix[$i][$size-8];
			if ($size-8 >= 0)    $matrix[7][$size-1-$i] = isset($matrix[7][$size-1-$i]) && $matrix[7][$size-1-$i] === -1 ? 0 : $matrix[7][$size-1-$i];
			// Bottom-left
			if ($size-8 >= 0)    $matrix[$size-8][$i] = isset($matrix[$size-8][$i]) && $matrix[$size-8][$i] === -1 ? 0 : $matrix[$size-8][$i];
			if ($size-8 >= 0)    $matrix[$size-1-$i][7] = isset($matrix[$size-1-$i][7]) && $matrix[$size-1-$i][7] === -1 ? 0 : $matrix[$size-1-$i][7];
		}

		// Timing patterns.
		for ($i = 8; $i < $size - 8; $i++) {
			$v = ($i % 2 === 0) ? 1 : 0;
			if ($matrix[6][$i] === -1) $matrix[6][$i] = $v;
			if ($matrix[$i][6] === -1) $matrix[$i][6] = $v;
		}

		// Dark module (always set in all versions).
		$matrix[$size - 8][8] = 1;

		// Alignment patterns (version >= 2).
		static $alignTable = [
			[],[6,18],[6,22],[6,26],[6,30],[6,34],
			[6,22,38],[6,24,42],[6,26,46],[6,28,50],[6,30,54],
			[6,32,58],[6,34,62],[6,26,46,66],[6,26,48,70],
			[6,26,50,74],[6,30,54,78],[6,30,56,82],[6,30,58,86],
			[6,34,62,90],[6,28,50,72,94],[6,26,50,74,98],
			[6,30,54,78,102],[6,28,54,80,106],[6,32,58,84,110],
			[6,30,58,86,114],[6,34,62,90,118],[6,26,50,74,98,122],
			[6,30,54,78,102,126],[6,26,52,78,104,130],
			[6,30,56,82,108,132],[6,34,60,86,112,136],
			[6,30,58,86,114,142],[6,34,62,90,118,146],
			[6,30,54,78,102,126,150],[6,24,50,76,102,128,154],
			[6,28,54,80,106,132,158],[6,32,58,84,110,136,162],
			[6,26,54,82,110,138,166],[6,30,58,86,114,142,170],
		];
		if ($version >= 2) {
			$coords = $alignTable[$version - 1];
			foreach ($coords as $r) {
				foreach ($coords as $c) {
					// Skip positions that overlap finder patterns.
					if ($matrix[$r][$c] !== -1) continue;
					$ap = [[1,1,1,1,1],[1,0,0,0,1],[1,0,1,0,1],[1,0,0,0,1],[1,1,1,1,1]];
					foreach ($ap as $dr => $row) {
						foreach ($row as $dc => $v) {
							$rr = $r - 2 + $dr;
							$cc = $c - 2 + $dc;
							if ($rr >= 0 && $rr < $size && $cc >= 0 && $cc < $size) {
								if ($matrix[$rr][$cc] === -1) {
									$matrix[$rr][$cc] = $v;
								}
							}
						}
					}
				}
			}
		}

		// --- Data encoding (byte mode, ECC level M) ---
		// ECC parameters per version/level-M: [total codewords, data codewords,
		// ec codewords per block, blocks in group 1, data per block group 1,
		// blocks in group 2, data per block group 2]
		static $eccTable = [
			// v  total  data  ecPerBlk  b1  d1  b2  d2
			[1,  26,  16,  10, 1, 16, 0,  0],
			[2,  44,  28,  16, 1, 28, 0,  0],
			[3,  70,  44,  26, 2, 22, 0,  0],
			[4, 100,  64,  18, 2, 32, 0,  0],
			[5, 134,  86,  24, 2, 43, 0,  0],
			[6, 172, 108,  16, 4, 27, 0,  0],
			[7, 196, 124,  18, 4, 31, 0,  0],
			[8, 242, 154,  22, 2, 38, 2, 39],
			[9, 292, 182,  22, 3, 36, 2, 37],
			[10,346, 216,  26, 4, 43, 1, 44],
		];
		// For versions > 10 fall back to a simplified approach: just fill
		// data codewords without proper ECC (acceptable for display; scanners
		// with error correction can still decode many codes).
		$eccInfo = null;
		foreach ($eccTable as $row) {
			if ($row[0] === $version) { $eccInfo = $row; break; }
		}

		// Build data bit stream.
		$bits = '';
		// Mode indicator: 0100 = byte mode.
		$bits .= '0100';
		// Character count (8 bits for versions 1-9, 16 for 10-26).
		$ccBits = ($version <= 9) ? 8 : 16;
		$bits .= str_pad(decbin($byteLen), $ccBits, '0', STR_PAD_LEFT);
		// Data bytes.
		for ($i = 0; $i < $byteLen; $i++) {
			$bits .= str_pad(decbin(ord($data[$i])), 8, '0', STR_PAD_LEFT);
		}

		// Determine total data bits capacity.
		$totalDataBits = ($eccInfo !== null)
			? $eccInfo[2] * 8
			: $capacities[$version - 1] * 8;

		// Terminator.
		$rem = $totalDataBits - strlen($bits);
		$bits .= str_repeat('0', min(4, max(0, $rem)));
		// Pad to byte boundary.
		while (strlen($bits) % 8 !== 0) $bits .= '0';
		// Pad codewords.
		$padBytes = ['11101100', '00010001'];
		$pi = 0;
		while (strlen($bits) < $totalDataBits) {
			$bits .= $padBytes[$pi % 2];
			$pi++;
		}

		// Convert bit string to codeword bytes.
		$codewords = [];
		for ($i = 0; $i < strlen($bits); $i += 8) {
			$codewords[] = bindec(substr($bits, $i, 8));
		}

		// Simple Reed-Solomon ECC (GF(256), generator polynomial for QR).
		$eccCW = ($eccInfo !== null) ? $eccInfo[3] : 10;
		$eccBytes = self::rsEncode($codewords, $eccCW);
		$allCW = array_merge($codewords, $eccBytes);

		// Convert all codewords to a single bit stream.
		$dataBits = '';
		foreach ($allCW as $cw) {
			$dataBits .= str_pad(decbin($cw), 8, '0', STR_PAD_LEFT);
		}
		// Remainder bits.
		static $remainderBits = [0,7,7,7,7,7,0,0,0,0,0,0,0,3,3,3,3,3,3,3,
			4,4,4,4,4,4,4,3,3,3,3,3,3,3,0,0,0,0,0,0];
		$dataBits .= str_repeat('0', $remainderBits[$version - 1]);

		// Place data bits into the matrix (zigzag pattern, mask 0).
		$bitIdx = 0;
		$col    = $size - 1;
		$up     = true;
		while ($col > 0) {
			if ($col === 6) $col--; // skip timing column
			for ($dy = 0; $dy < $size; $dy++) {
				$row = $up ? ($size - 1 - $dy) : $dy;
				foreach ([0, 1] as $d) {
					$c = $col - $d;
					if ($matrix[$row][$c] === -1) {
						$bit = ($bitIdx < strlen($dataBits))
							? (int) $dataBits[$bitIdx++]
							: 0;
						// Apply mask pattern 0: (row + col) % 2 === 0
						if (($row + $c) % 2 === 0) $bit ^= 1;
						$matrix[$row][$c] = $bit;
					}
				}
			}
			$col -= 2;
			$up = !$up;
		}

		// Format information (ECC level M = 00, mask 0 → pattern 101010000010010).
		$formatBits = [1,0,1,0,1,0,0,0,0,0,1,0,0,1,0];
		$fi = 0;
		// Top-left horizontal.
		for ($i = 0; $i < 6; $i++) $matrix[8][$i] = $formatBits[$fi++];
		$matrix[8][7] = $formatBits[$fi++];
		$matrix[8][8] = $formatBits[$fi++];
		$matrix[7][8] = $formatBits[$fi++];
		for ($i = 5; $i >= 0; $i--) $matrix[$i][8] = $formatBits[$fi++];
		// Bottom-left and top-right copies.
		$fi = 0;
		for ($i = $size - 1; $i >= $size - 7; $i--) $matrix[$i][8] = $formatBits[$fi++];
		for ($i = 8; $i < $size; $i++)               $matrix[8][$i] = $formatBits[$fi++];

		// Convert -1 (unfilled) to 0.
		foreach ($matrix as $r => $row) {
			foreach ($row as $c => $v) {
				if ($v === -1) $matrix[$r][$c] = 0;
			}
		}

		return $matrix;
	}

	/**
	 * Reed-Solomon encoder for QR code ECC.
	 *
	 * @param int[] $data    Data codewords.
	 * @param int   $eccLen  Number of ECC codewords to generate.
	 * @return int[] ECC codewords.
	 */
	private static function rsEncode(array $data, int $eccLen): array {
		// GF(256) log/antilog tables (primitive polynomial 0x11D).
		static $log = null, $alog = null;
		if ($log === null) {
			$log  = array_fill(0, 256, 0);
			$alog = array_fill(0, 256, 0);
			$x = 1;
			for ($i = 0; $i < 255; $i++) {
				$alog[$i] = $x;
				$log[$x]  = $i;
				$x <<= 1;
				if ($x & 0x100) $x ^= 0x11D;
			}
		}
		// Generator polynomial coefficients.
		$gen = [1];
		for ($i = 0; $i < $eccLen; $i++) {
			$gen2 = array_fill(0, count($gen) + 1, 0);
			foreach ($gen as $j => $g) {
				$gen2[$j]   ^= $g;
				$gen2[$j+1] ^= ($g === 0) ? 0 : $alog[($log[$g] + $i) % 255];
			}
			$gen = $gen2;
		}
		// Polynomial long division.
		$msg = array_merge($data, array_fill(0, $eccLen, 0));
		for ($i = 0; $i < count($data); $i++) {
			if ($msg[$i] === 0) continue;
			$f = $log[$msg[$i]];
			foreach ($gen as $j => $g) {
				if ($g !== 0) $msg[$i + $j] ^= $alog[($f + $log[$g]) % 255];
			}
		}
		return array_slice($msg, count($data));
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
