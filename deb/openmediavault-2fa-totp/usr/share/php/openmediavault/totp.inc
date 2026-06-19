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
 * 1Password, OTP Auth and other standard authenticator apps.
 */
class Totp
{
    /** Number of digits in each OTP code (RFC 6238 default). */
    public const DIGITS = 6;

    /** Duration of each time window in seconds (RFC 6238 default). */
    public const PERIOD = 30;

    /** Hash algorithm used for TOTP generation (RFC 6238 default). */
    public const ALGORITHM = 'SHA1';

    /**
     * How many adjacent windows to accept on each side of the current
     * one. A value of 1 tolerates up to 30 seconds of clock skew
     * between the authenticator app and the server.
     */
    public const ALLOWED_DRIFT = 1;

    /**
     * Generate a cryptographically secure 160-bit (20-byte) random
     * secret encoded as an uppercase base32 string (RFC 4648, no
     * padding).
     *
     * @return string 32-character uppercase base32-encoded secret.
     */
    public static function generateSecret(): string
    {
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
    public static function verifyCode(string $secret, string $code): bool
    {
        // Reject obviously malformed codes before doing any crypto work.
        if (!preg_match('/^\d{6}$/', $code)) {
            return false;
        }
        $now = (int) floor(time() / self::PERIOD);
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
     * @see https://www.ietf.org/archive/id/draft-andesco-otpauth-uri-00.html
     */
    public static function getProvisioningUri(
        string $secret,
        string $username,
        string $issuer
    ): string {
        $secret = strtoupper(trim($secret));
        $issuer = trim($issuer);
        $username = trim($username);
        $query = http_build_query([
            'secret' => $secret,
            'issuer' => $issuer
            // Some apps have problems when these arguments are included.
            // Since we are using the defaults, it is not necessary to
            // add them.
            // 'algorithm' => self::ALGORITHM,
            // 'digits' => self::DIGITS,
            // 'period' => self::PERIOD
        ], "", "&", PHP_QUERY_RFC3986);
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
    public static function generateQrDataUrl(string $data, int $scale = 4): string
    {
        if (($scale < 1) || ($scale > 64)) {
            throw new \InvalidArgumentException(
                "The parameter 'scale' must be between 1 and 64."
            );
        }
        $matrix = self::buildQrMatrix($data);
        $size = count($matrix);
        $quietZone = 4;
        $dimension = ($size + ($quietZone * 2)) * $scale;

        $path = [];
        foreach ($matrix as $row => $cols) {
            $runStart = -1;
            $lastCol = count($cols) - 1;
            foreach ($cols as $col => $dark) {
                if ($dark && (-1 === $runStart)) {
                    $runStart = $col;
                }
                if (!$dark || ($col === $lastCol)) {
                    if (-1 !== $runStart) {
                        $end = $dark ? ($col + 1) : $col;
                        $x = ($runStart + $quietZone) * $scale;
                        $y = ($row + $quietZone) * $scale;
                        $width = ($end - $runStart) * $scale;
                        $path[] = sprintf(
                            "M%d %dh%dv%dH%dz",
                            $x,
                            $y,
                            $width,
                            $scale,
                            $x
                        );
                        $runStart = -1;
                    }
                }
            }
        }

        $svg = sprintf(
            '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 %1$d %1$d"'
            . ' width="%1$d" height="%1$d"><rect width="100%%" height="100%%"'
            . ' fill="#fff"/><path fill="#000" d="%2$s"/></svg>',
            $dimension,
            implode("", $path)
        );
        return sprintf(
            "data:image/svg+xml;base64,%s",
            base64_encode($svg)
        );
    }

    /**
     * Build a boolean matrix representing the dark/light modules of a QR code.
     *
     * Uses QR Model 2, byte mode and ECC level M (versions 1..10).
     *
     * @param string $data The input string to encode.
     * @return array<int,array<int,bool>> 2-D array indexed [row][col].
     */
    private static function buildQrMatrix(string $data): array
    {
        $byteLen = strlen($data);
        $version = self::selectQrVersion($byteLen);
        $size = ($version * 4) + 17;

        $matrix = array_fill(0, $size, array_fill(0, $size, 0));
        $isFunction = array_fill(0, $size, array_fill(0, $size, false));

        self::placeFinderPattern($matrix, $isFunction, 0, 0);
        self::placeFinderPattern($matrix, $isFunction, 0, $size - 7);
        self::placeFinderPattern($matrix, $isFunction, $size - 7, 0);

        for ($i = 8; $i < $size - 8; $i++) {
            $v = ($i % 2 === 0) ? 1 : 0;
            self::setModule($matrix, $isFunction, 6, $i, $v, true);
            self::setModule($matrix, $isFunction, $i, 6, $v, true);
        }

        self::placeAlignmentPatterns($matrix, $isFunction, $version);
        self::setModule($matrix, $isFunction, $size - 8, 8, 1, true);
        self::reserveFormatModules($matrix, $isFunction);
        if ($version >= 7) {
            self::reserveVersionModules($matrix, $isFunction);
        }

        $dataBits = self::buildQrDataBits($data, $version);
        $bitIdx = 0;
        $col = $size - 1;
        $up = true;
        while ($col > 0) {
            if ($col === 6) {
                $col--;
            }
            for ($dy = 0; $dy < $size; $dy++) {
                $row = $up ? ($size - 1 - $dy) : $dy;
                for ($d = 0; $d < 2; $d++) {
                    $c = $col - $d;
                    if (!$isFunction[$row][$c]) {
                        $bit = ($bitIdx < count($dataBits)) ? $dataBits[$bitIdx] : 0;
                        $matrix[$row][$c] = $bit;
                        $bitIdx++;
                    }
                }
            }
            $col -= 2;
            $up = !$up;
        }

        $bestMatrix = null;
        $bestPenalty = PHP_INT_MAX;
        for ($mask = 0; $mask < 8; $mask++) {
            $candidate = $matrix;
            for ($r = 0; $r < $size; $r++) {
                for ($c = 0; $c < $size; $c++) {
                    if (!$isFunction[$r][$c] && self::maskBit($mask, $r, $c)) {
                        $candidate[$r][$c] ^= 1;
                    }
                }
            }
            self::placeFormatInfo($candidate, $size, $mask);
            if ($version >= 7) {
                self::placeVersionInfo($candidate, $size, $version);
            }
            $penalty = self::computeMaskPenalty($candidate);
            if ($penalty < $bestPenalty) {
                $bestPenalty = $penalty;
                $bestMatrix = $candidate;
            }
        }

        return $bestMatrix;
    }

    /**
     * Select the smallest QR version that can hold byte-mode payload at ECC M.
     *
     * @param int $byteLen Payload length in bytes.
     * @return int
     */
    private static function selectQrVersion(int $byteLen): int
    {
        // Byte capacities for ECC level M, versions 1..10.
        static $capacityM = [14, 26, 42, 62, 84, 106, 122, 152, 180, 213];
        foreach ($capacityM as $idx => $capacity) {
            if ($byteLen <= $capacity) {
                return $idx + 1;
            }
        }
        throw new \InvalidArgumentException("The QR payload is too long.");
    }

    /**
     * Place one 7x7 finder plus separator.
     *
     * @param array<int,array<int,int>>  $matrix
     * @param array<int,array<int,bool>> $isFunction
     * @param int $top
     * @param int $left
     */
    private static function placeFinderPattern(
        array &$matrix,
        array &$isFunction,
        int $top,
        int $left
    ): void {
        $size = count($matrix);
        for ($r = -1; $r <= 7; $r++) {
            for ($c = -1; $c <= 7; $c++) {
                $rr = $top + $r;
                $cc = $left + $c;
                if ($rr < 0 || $rr >= $size || $cc < 0 || $cc >= $size) {
                    continue;
                }
                $isBorder = ($r === -1 || $r === 7 || $c === -1 || $c === 7);
                $isOuter = ($r === 0 || $r === 6 || $c === 0 || $c === 6);
                $isInner = ($r >= 2 && $r <= 4 && $c >= 2 && $c <= 4);
                $v = (!$isBorder && ($isOuter || $isInner)) ? 1 : 0;
                self::setModule($matrix, $isFunction, $rr, $cc, $v, true);
            }
        }
    }

    /**
     * Place alignment patterns for versions 2..10.
     *
     * @param array<int,array<int,int>>  $matrix
     * @param array<int,array<int,bool>> $isFunction
     * @param int $version
     */
    private static function placeAlignmentPatterns(
        array &$matrix,
        array &$isFunction,
        int $version
    ): void {
        static $centers = [
            1 => [],
            2 => [6, 18],
            3 => [6, 22],
            4 => [6, 26],
            5 => [6, 30],
            6 => [6, 34],
            7 => [6, 22, 38],
            8 => [6, 24, 42],
            9 => [6, 26, 46],
            10 => [6, 28, 50],
        ];
        if ($version < 2) {
            return;
        }
        foreach ($centers[$version] as $r) {
            foreach ($centers[$version] as $c) {
                if ($isFunction[$r][$c]) {
                    continue;
                }
                for ($dr = -2; $dr <= 2; $dr++) {
                    for ($dc = -2; $dc <= 2; $dc++) {
                        $rr = $r + $dr;
                        $cc = $c + $dc;
                        $isOuter = (abs($dr) === 2 || abs($dc) === 2);
                        $isCenter = ($dr === 0 && $dc === 0);
                        $v = ($isOuter || $isCenter) ? 1 : 0;
                        self::setModule($matrix, $isFunction, $rr, $cc, $v, true);
                    }
                }
            }
        }
    }

    /**
     * Reserve format information modules.
     *
     * @param array<int,array<int,int>>  $matrix
     * @param array<int,array<int,bool>> $isFunction
     */
    private static function reserveFormatModules(array &$matrix, array &$isFunction): void
    {
        $size = count($matrix);
        for ($i = 0; $i < 9; $i++) {
            if ($i !== 6) {
                self::setModule($matrix, $isFunction, 8, $i, 0, true);
                self::setModule($matrix, $isFunction, $i, 8, 0, true);
            }
        }
        for ($i = 0; $i < 8; $i++) {
            self::setModule($matrix, $isFunction, $size - 1 - $i, 8, 0, true);
            self::setModule($matrix, $isFunction, 8, $size - 1 - $i, 0, true);
        }
    }

    /**
     * Reserve version information modules.
     *
     * @param array<int,array<int,int>>  $matrix
     * @param array<int,array<int,bool>> $isFunction
     */
    private static function reserveVersionModules(array &$matrix, array &$isFunction): void
    {
        $size = count($matrix);
        for ($i = 0; $i < 18; $i++) {
            $r = intdiv($i, 3);
            $c = $i % 3;
            self::setModule($matrix, $isFunction, $r, $size - 11 + $c, 0, true);
            self::setModule($matrix, $isFunction, $size - 11 + $c, $r, 0, true);
        }
    }

    /**
     * Build final data bit stream including interleaved ECC and remainder bits.
     *
     * @param string $data
     * @param int $version
     * @return int[]
     */
    private static function buildQrDataBits(string $data, int $version): array
    {
        // [data codewords, ec codewords per block, group1 blocks, group1 size, group2 blocks, group2 size]
        static $eccTable = [
            1 => [16, 10, 1, 16, 0, 0],
            2 => [28, 16, 1, 28, 0, 0],
            3 => [44, 26, 1, 44, 0, 0],
            4 => [64, 18, 2, 32, 0, 0],
            5 => [86, 24, 2, 43, 0, 0],
            6 => [108, 16, 4, 27, 0, 0],
            7 => [124, 18, 4, 31, 0, 0],
            8 => [154, 22, 2, 38, 2, 39],
            9 => [182, 22, 3, 36, 2, 37],
            10 => [216, 26, 4, 43, 1, 44],
        ];
        static $remainderBits = [1 => 0, 2 => 7, 3 => 7, 4 => 7, 5 => 7, 6 => 7, 7 => 0, 8 => 0, 9 => 0, 10 => 0];

        [$dataCw, $ecCw, $g1Count, $g1Size, $g2Count, $g2Size] = $eccTable[$version];
        $byteLen = strlen($data);

        $bits = '0100';
        $countBits = ($version <= 9) ? 8 : 16;
        $bits .= str_pad(decbin($byteLen), $countBits, '0', STR_PAD_LEFT);
        for ($i = 0; $i < $byteLen; $i++) {
            $bits .= str_pad(decbin(ord($data[$i])), 8, '0', STR_PAD_LEFT);
        }

        $maxBits = $dataCw * 8;
        $remaining = $maxBits - strlen($bits);
        $bits .= str_repeat('0', min(4, max(0, $remaining)));
        while ((strlen($bits) % 8) !== 0) {
            $bits .= '0';
        }
        $pads = ['11101100', '00010001'];
        $padIdx = 0;
        while (strlen($bits) < $maxBits) {
            $bits .= $pads[$padIdx % 2];
            $padIdx++;
        }

        $codewords = [];
        for ($i = 0; $i < strlen($bits); $i += 8) {
            $codewords[] = bindec(substr($bits, $i, 8));
        }

        $dataBlocks = [];
        $offset = 0;
        for ($i = 0; $i < $g1Count; $i++) {
            $dataBlocks[] = array_slice($codewords, $offset, $g1Size);
            $offset += $g1Size;
        }
        for ($i = 0; $i < $g2Count; $i++) {
            $dataBlocks[] = array_slice($codewords, $offset, $g2Size);
            $offset += $g2Size;
        }

        $eccBlocks = [];
        foreach ($dataBlocks as $block) {
            $eccBlocks[] = self::rsEncode($block, $ecCw);
        }

        $interleaved = [];
        $maxDataBlockLen = 0;
        foreach ($dataBlocks as $block) {
            $maxDataBlockLen = max($maxDataBlockLen, count($block));
        }
        for ($i = 0; $i < $maxDataBlockLen; $i++) {
            foreach ($dataBlocks as $block) {
                if (isset($block[$i])) {
                    $interleaved[] = $block[$i];
                }
            }
        }
        for ($i = 0; $i < $ecCw; $i++) {
            foreach ($eccBlocks as $block) {
                $interleaved[] = $block[$i];
            }
        }

        $out = [];
        foreach ($interleaved as $cw) {
            for ($bit = 7; $bit >= 0; $bit--) {
                $out[] = ($cw >> $bit) & 1;
            }
        }
        for ($i = 0; $i < $remainderBits[$version]; $i++) {
            $out[] = 0;
        }
        return $out;
    }

    /**
     * Reed-Solomon encoder for QR code ECC.
     *
     * @param int[] $data    Data codewords.
     * @param int   $eccLen  Number of ECC codewords to generate.
     * @return int[] ECC codewords.
     */
    private static function rsEncode(array $data, int $eccLen): array
    {
        static $log = null;
        static $alog = null;
        if ($log === null) {
            $log = array_fill(0, 256, 0);
            $alog = array_fill(0, 256, 0);
            $x = 1;
            for ($i = 0; $i < 255; $i++) {
                $alog[$i] = $x;
                $log[$x] = $i;
                $x <<= 1;
                if ($x & 0x100) {
                    $x ^= 0x11D;
                }
            }
        }

        $generator = [1];
        for ($i = 0; $i < $eccLen; $i++) {
            $next = array_fill(0, count($generator) + 1, 0);
            foreach ($generator as $j => $coef) {
                $next[$j] ^= $coef;
                if ($coef !== 0) {
                    $next[$j + 1] ^= $alog[($log[$coef] + $i) % 255];
                }
            }
            $generator = $next;
        }

        $message = array_merge($data, array_fill(0, $eccLen, 0));
        $dataLen = count($data);
        for ($i = 0; $i < $dataLen; $i++) {
            $lead = $message[$i];
            if ($lead === 0) {
                continue;
            }
            $factor = $log[$lead];
            foreach ($generator as $j => $coef) {
                if ($coef !== 0) {
                    $message[$i + $j] ^= $alog[($factor + $log[$coef]) % 255];
                }
            }
        }
        return array_slice($message, $dataLen);
    }

    /**
     * Compute whether a mask pattern flips the given module.
     *
     * @param int $mask
     * @param int $row
     * @param int $col
     * @return bool
     */
    private static function maskBit(int $mask, int $row, int $col): bool
    {
        switch ($mask) {
            case 0: return (($row + $col) % 2) === 0;
            case 1: return ($row % 2) === 0;
            case 2: return ($col % 3) === 0;
            case 3: return (($row + $col) % 3) === 0;
            case 4: return ((int) floor($row / 2) + (int) floor($col / 3)) % 2 === 0;
            case 5: return ((($row * $col) % 2) + (($row * $col) % 3)) === 0;
            case 6: return (((($row * $col) % 2) + (($row * $col) % 3)) % 2) === 0;
            case 7: return (((($row + $col) % 2) + (($row * $col) % 3)) % 2) === 0;
        }
        return false;
    }

    /**
     * Place 15 format bits for ECC level M and selected mask.
     *
     * @param array<int,array<int,int>> $matrix
     * @param int $size
     * @param int $mask
     */
    private static function placeFormatInfo(array &$matrix, int $size, int $mask): void
    {
        $bits = self::computeFormatBits($mask);
        $coordsA = [
            [8, 0], [8, 1], [8, 2], [8, 3], [8, 4], [8, 5], [8, 7], [8, 8],
            [7, 8], [5, 8], [4, 8], [3, 8], [2, 8], [1, 8], [0, 8],
        ];
        $coordsB = [
            [$size - 1, 8], [$size - 2, 8], [$size - 3, 8], [$size - 4, 8],
            [$size - 5, 8], [$size - 6, 8], [$size - 7, 8], [8, $size - 8],
            [8, $size - 7], [8, $size - 6], [8, $size - 5], [8, $size - 4],
            [8, $size - 3], [8, $size - 2], [8, $size - 1],
        ];
        for ($i = 0; $i < 15; $i++) {
            [$r1, $c1] = $coordsA[$i];
            [$r2, $c2] = $coordsB[$i];
            $matrix[$r1][$c1] = $bits[$i];
            $matrix[$r2][$c2] = $bits[$i];
        }
    }

    /**
     * Place 18 version bits for version >= 7.
     *
     * @param array<int,array<int,int>> $matrix
     * @param int $size
     * @param int $version
     */
    private static function placeVersionInfo(array &$matrix, int $size, int $version): void
    {
        $bits = self::computeVersionBits($version);
        for ($i = 0; $i < 18; $i++) {
            $r = intdiv($i, 3);
            $c = $i % 3;
            $matrix[$r][$size - 11 + $c] = $bits[$i];
            $matrix[$size - 11 + $c][$r] = $bits[$i];
        }
    }

    /**
     * Compute BCH-protected format bits for ECC level M + mask.
     *
     * @param int $mask
     * @return int[]
     */
    private static function computeFormatBits(int $mask): array
    {
        $data = $mask; // ECC level M = 00, so only mask bits remain.
        $value = $data << 10;
        $generator = 0x537;
        for ($i = 14; $i >= 10; $i--) {
            if ((($value >> $i) & 1) !== 0) {
                $value ^= ($generator << ($i - 10));
            }
        }
        $format = (($data << 10) | ($value & 0x3FF)) ^ 0x5412;
        $bits = [];
        for ($i = 14; $i >= 0; $i--) {
            $bits[] = ($format >> $i) & 1;
        }
        return $bits;
    }

    /**
     * Compute BCH-protected version bits.
     *
     * @param int $version
     * @return int[]
     */
    private static function computeVersionBits(int $version): array
    {
        $value = $version << 12;
        $generator = 0x1F25;
        for ($i = 17; $i >= 12; $i--) {
            if ((($value >> $i) & 1) !== 0) {
                $value ^= ($generator << ($i - 12));
            }
        }
        $bitsVal = ($version << 12) | ($value & 0xFFF);
        $bits = [];
        for ($i = 0; $i < 18; $i++) {
            $bits[] = ($bitsVal >> $i) & 1;
        }
        return $bits;
    }

    /**
     * Compute mask penalty score according to ISO/IEC 18004.
     *
     * @param array<int,array<int,int>> $matrix
     * @return int
     */
    private static function computeMaskPenalty(array $matrix): int
    {
        $size = count($matrix);
        $penalty = 0;

        for ($r = 0; $r < $size; $r++) {
            $runColor = $matrix[$r][0];
            $runLen = 1;
            for ($c = 1; $c < $size; $c++) {
                if ($matrix[$r][$c] === $runColor) {
                    $runLen++;
                } else {
                    if ($runLen >= 5) {
                        $penalty += 3 + ($runLen - 5);
                    }
                    $runColor = $matrix[$r][$c];
                    $runLen = 1;
                }
            }
            if ($runLen >= 5) {
                $penalty += 3 + ($runLen - 5);
            }
        }
        for ($c = 0; $c < $size; $c++) {
            $runColor = $matrix[0][$c];
            $runLen = 1;
            for ($r = 1; $r < $size; $r++) {
                if ($matrix[$r][$c] === $runColor) {
                    $runLen++;
                } else {
                    if ($runLen >= 5) {
                        $penalty += 3 + ($runLen - 5);
                    }
                    $runColor = $matrix[$r][$c];
                    $runLen = 1;
                }
            }
            if ($runLen >= 5) {
                $penalty += 3 + ($runLen - 5);
            }
        }

        for ($r = 0; $r < $size - 1; $r++) {
            for ($c = 0; $c < $size - 1; $c++) {
                $v = $matrix[$r][$c];
                if (
                    $v === $matrix[$r][$c + 1] &&
                    $v === $matrix[$r + 1][$c] &&
                    $v === $matrix[$r + 1][$c + 1]
                ) {
                    $penalty += 3;
                }
            }
        }

        $patA = [1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0];
        $patB = [0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1];
        for ($r = 0; $r < $size; $r++) {
            for ($c = 0; $c <= $size - 11; $c++) {
                $slice = array_slice($matrix[$r], $c, 11);
                if ($slice === $patA || $slice === $patB) {
                    $penalty += 40;
                }
            }
        }
        for ($c = 0; $c < $size; $c++) {
            for ($r = 0; $r <= $size - 11; $r++) {
                $slice = [];
                for ($k = 0; $k < 11; $k++) {
                    $slice[] = $matrix[$r + $k][$c];
                }
                if ($slice === $patA || $slice === $patB) {
                    $penalty += 40;
                }
            }
        }

        $dark = 0;
        foreach ($matrix as $row) {
            foreach ($row as $v) {
                if ($v === 1) {
                    $dark++;
                }
            }
        }
        $total = $size * $size;
        $ratio = (int) floor(($dark * 100) / $total);
        $lower = intdiv($ratio, 5) * 5;
        $upper = $lower + 5;
        $penalty += min(abs($lower - 50), abs($upper - 50)) * 2;

        return $penalty;
    }

    /**
     * Set one matrix module.
     *
     * @param array<int,array<int,int>>  $matrix
     * @param array<int,array<int,bool>> $isFunction
     * @param int $row
     * @param int $col
     * @param int $value
     * @param bool $function
     */
    private static function setModule(
        array &$matrix,
        array &$isFunction,
        int $row,
        int $col,
        int $value,
        bool $function
    ): void {
        $size = count($matrix);
        if ($row < 0 || $row >= $size || $col < 0 || $col >= $size) {
            return;
        }
        $matrix[$row][$col] = $value & 1;
        if ($function) {
            $isFunction[$row][$col] = true;
        }
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
     *  5. Reduce modulo 10^DIGITS, zero-pad to DIGITS digits.
     *
     * @param string $secret  Uppercase base32-encoded TOTP secret.
     * @param int    $counter Time counter (floor(unix_time / PERIOD)).
     * @return string Zero-padded DIGITS-digit OTP string.
     */
    private static function computeCode(
        string $secret,
        int $counter
    ): string {
        $key = self::base32Decode(strtoupper(trim($secret)));

        // Encode counter as 8-byte big-endian (high 32 bits are 0 here).
        $timeBytes = pack('N*', 0) . pack('N*', $counter);

        $hmac = hash_hmac(
            strtolower(self::ALGORITHM),
            $timeBytes,
            $key,
            true
        );

        // Offset is the low 4 bits of the last HMAC byte (RFC 4226 §5.4).
        $offset = ord($hmac[19]) & 0x0F;

        // Extract a 31-bit integer starting at $offset.
        $code = (
            ((ord($hmac[$offset]) & 0x7F) << 24) |
            ((ord($hmac[$offset + 1]) & 0xFF) << 16) |
            ((ord($hmac[$offset + 2]) & 0xFF) << 8) |
             (ord($hmac[$offset + 3]) & 0xFF)
        ) % (10 ** self::DIGITS);

        return str_pad((string) $code, self::DIGITS, '0', STR_PAD_LEFT);
    }

    /**
     * Encode raw bytes as an uppercase base32 string (RFC 4648, no padding).
     *
     * @param string $data Raw binary input.
     * @return string Uppercase base32-encoded string.
     */
    private static function base32Encode(string $data): string
    {
        $alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567';
        $result = '';
        $buffer = 0;
        $bitsLeft = 0;

        foreach (str_split($data) as $byte) {
            $buffer = ($buffer << 8) | ord($byte);
            $bitsLeft += 8;
            while ($bitsLeft >= 5) {
                $bitsLeft -= 5;
                $result .= $alphabet[($buffer >> $bitsLeft) & 31];
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
    private static function base32Decode(string $data): string
    {
        $alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567';
        $result = '';
        $buffer = 0;
        $bitsLeft = 0;

        foreach (str_split($data) as $char) {
            $pos = strpos($alphabet, $char);
            if ($pos === false) {
                continue; // skip '=' padding and unrecognized chars
            }
            $buffer = ($buffer << 5) | $pos;
            $bitsLeft += 5;
            if ($bitsLeft >= 8) {
                $bitsLeft -= 8;
                $result .= chr(($buffer >> $bitsLeft) & 0xFF);
            }
        }
        return $result;
    }
}
