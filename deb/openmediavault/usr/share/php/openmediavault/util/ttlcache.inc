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

namespace OMV\Util;

/**
 * A simple TTL-based in-memory cache.
 * @ingroup api
 */
class TtlCache
{
    private array $data = [];
    private int $ttl;

    /**
     * Constructor.
     * @param int $ttl Cache lifetime in seconds.
     */
    public function __construct(int $ttl)
    {
        $this->ttl = $ttl;
    }

    /**
     * Check whether the cache contains a valid (non-expired) entry for
     * the given key.
     * @param string $key The cache key.
     * @return bool TRUE if a valid entry exists, otherwise FALSE.
     */
    public function has(string $key): bool
    {
        return array_key_exists($key, $this->data)
            && $this->data[$key]['expires'] > time();
    }

    /**
     * Get the cached value for the given key.
     * @param string $key The cache key.
     * @return mixed The cached value, or NULL if the key does not exist
     *   or has expired.
     */
    public function get(string $key)
    {
        if (!$this->has($key)) {
            return null;
        }
        return $this->data[$key]['value'];
    }

    /**
     * Store a value in the cache under the given key. Expired entries
     * are purged automatically on every write to keep memory usage low.
     * @param string $key The cache key.
     * @param mixed $value The value to cache.
     * @return mixed The cached value.
     */
    public function set(string $key, $value)
    {
        // Purge expired entries before adding a new one.
        $now = time();
        foreach ($this->data as $k => $entry) {
            if ($entry['expires'] <= $now) {
                $this->remove($k);
            }
        }
        $this->data[$key] = [
            'value'   => $value,
            'expires' => $now + $this->ttl,
        ];
        return $value;
    }

    /**
     * Remove a single entry from the cache.
     * @param string $key The cache key.
     * @return void
     */
    public function remove(string $key): void
    {
        unset($this->data[$key]);
    }

    /**
     * Remove all entries from the cache.
     * @return void
     */
    public function clear(): void
    {
        $this->data = [];
    }
}
