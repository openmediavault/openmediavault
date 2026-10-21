#!/usr/bin/phpunit -c/etc/openmediavault
<?php

/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Karol Olszacki <karol@olszacki.pl>
 * @copyright Copyright (c) 2026 Karol Olszacki
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
 * along with OpenMediavault. If not, see <https://www.gnu.org/licenses/>.
 */
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/globals.inc");

class test_openmediavault_system_blkidcache extends \PHPUnit\Framework\TestCase
{
    /**
     * @var string Cache key used by the tests.
     */
    private $key = "unittest";

    /**
     * @var string Temporary cache directory used by the tests.
     */
    private $dir = "";

    protected function setUp(): void
    {
        $this->dir = sys_get_temp_dir() . "/omv-blkidcache-" . getmypid();
        @mkdir($this->dir, 0755, true);
        \OMV\Environment::set("OMV_BLKID_CACHE_DIR", $this->dir);
        @unlink(\OMV\System\BlkidCache::getCacheFile($this->key));
    }

    protected function tearDown(): void
    {
        @unlink(\OMV\System\BlkidCache::getCacheFile($this->key));
        @unlink(\OMV\System\BlkidCache::getLockFile($this->key));
        \OMV\Environment::set(
            "OMV_BLKID_CACHE_DIR",
            "/var/lib/openmediavault/blkid-cache"
        );
        @rmdir($this->dir);
    }

    /**
     * The cache lifetime is configurable.
     */
    public function testTtlIsConfigurable()
    {
        $this->assertEquals(30, \OMV\System\BlkidCache::getTtl());
        \OMV\Environment::set("OMV_BLKID_SCAN_CACHE_TTL", 90);
        try {
            $this->assertEquals(90, \OMV\System\BlkidCache::getTtl());
        } finally {
            \OMV\Environment::set("OMV_BLKID_SCAN_CACHE_TTL", 30);
        }
    }

    /**
     * Without a cache file there is nothing to serve.
     */
    public function testCacheIsEmptyWithoutCacheFile()
    {
        $this->assertNull(\OMV\System\BlkidCache::get($this->key));
    }

    /**
     * A written entry is read back unchanged and is fresh.
     */
    public function testCacheRoundTrip()
    {
        $data = [
            [
                "devicefile" => "/dev/sda1",
                "uuid" => "1234"
            ]
        ];
        \OMV\System\BlkidCache::put($this->key, $data);
        $cached = \OMV\System\BlkidCache::get($this->key);
        $this->assertNotNull($cached);
        $this->assertEquals($data, $cached["data"]);
        $this->assertTrue(\OMV\System\BlkidCache::isFresh($cached));
    }

    /**
     * An expired entry is not fresh, but it is still readable so that it can
     * be served as a fallback.
     */
    public function testExpiredCacheIsNotFresh()
    {
        \OMV\System\BlkidCache::put($this->key, ["a"]);
        touch(\OMV\System\BlkidCache::getCacheFile($this->key), time() - 3600);
        $cached = \OMV\System\BlkidCache::get($this->key);
        $this->assertNotNull($cached);
        $this->assertFalse(\OMV\System\BlkidCache::isFresh($cached));
    }

    /**
     * A corrupt cache file must not be used.
     */
    public function testCorruptCacheIsIgnored()
    {
        file_put_contents(
            \OMV\System\BlkidCache::getCacheFile($this->key),
            "this is not json"
        );
        $this->assertNull(\OMV\System\BlkidCache::get($this->key));
    }

    /**
     * Only one lock can be held for a key at a time.
     */
    public function testLockIsExclusive()
    {
        $first = \OMV\System\BlkidCache::tryLock($this->key);
        $this->assertNotFalse($first);
        $second = \OMV\System\BlkidCache::tryLock($this->key);
        $this->assertFalse($second);
        \OMV\System\BlkidCache::releaseLock($first);
        $third = \OMV\System\BlkidCache::tryLock($this->key);
        $this->assertNotFalse($third);
        \OMV\System\BlkidCache::releaseLock($third);
    }

    /**
     * Different keys must not block each other.
     */
    public function testLocksArePerKey()
    {
        $first = \OMV\System\BlkidCache::tryLock("key-a");
        $second = \OMV\System\BlkidCache::tryLock("key-b");
        $this->assertNotFalse($first);
        $this->assertNotFalse($second);
        \OMV\System\BlkidCache::releaseLock($first);
        \OMV\System\BlkidCache::releaseLock($second);
        @unlink(\OMV\System\BlkidCache::getLockFile("key-a"));
        @unlink(\OMV\System\BlkidCache::getLockFile("key-b"));
    }

    /**
     * The cache must not live in a world writable directory.
     */
    public function testCacheIsNotInTmp()
    {
        \OMV\Environment::set(
            "OMV_BLKID_CACHE_DIR",
            "/var/lib/openmediavault/blkid-cache"
        );
        $this->assertStringStartsWith(
            "/var/lib/openmediavault/",
            \OMV\System\BlkidCache::getDirectory()
        );
    }
}
