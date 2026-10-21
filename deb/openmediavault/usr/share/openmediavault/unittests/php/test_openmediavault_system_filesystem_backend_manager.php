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
 * along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
 */
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/globals.inc");

class test_openmediavault_system_filesystem_backend_manager extends \PHPUnit\Framework\TestCase
{
    /**
     * @var string Temporary directory used for the cache and lock files.
     */
    private $tmpDir = "";

    /**
     * @var \OMV\System\Filesystem\Backend\Manager
     */
    private $manager = null;

    protected function setUp(): void
    {
        $this->tmpDir = sys_get_temp_dir() . "/omv-blkid-test-" . getmypid();
        mkdir($this->tmpDir, 0755, true);
        $this->manager = new BlkidManagerMock();
        $this->manager->tmpDir = $this->tmpDir;
        \OMV\Environment::set("OMV_BLKID_CACHE_DIR", $this->tmpDir);
    }

    protected function tearDown(): void
    {
        \OMV\Environment::set(
            "OMV_BLKID_CACHE_DIR",
            "/run/openmediavault/blkid-cache"
        );
        foreach (glob($this->tmpDir . "/*") as $file) {
            @unlink($file);
        }
        @rmdir($this->tmpDir);
    }

    /**
     * Without a cache file there is nothing to serve.
     */
    public function testCacheIsEmptyWithoutCacheFile()
    {
        $this->assertNull($this->readCache());
    }

    /**
     * A written cache entry is read back unchanged and is fresh.
     */
    public function testCacheRoundTrip()
    {
        $data = [
            "ext4" => [
                [
                    "devicefile" => "/dev/sda1",
                    "uuid" => "1234",
                    "label" => "data",
                    "type" => "ext4"
                ]
            ]
        ];
        $this->writeCache($data);
        $cached = $this->readCache();
        $this->assertNotNull($cached);
        $this->assertEquals($data, $cached["data"]);
        $this->assertTrue($this->isFresh($cached));
    }

    /**
     * An expired cache entry is not fresh, but it is still readable so that
     * it can be served as a fallback.
     */
    public function testExpiredCacheIsNotFresh()
    {
        $this->writeCache(["ext4" => []]);
        $cached = $this->readCache();
        $this->assertNotNull($cached);
        touch($this->cacheFile(), time() - 3600);
        $cached = $this->readCache();
        $this->assertFalse($this->isFresh($cached));
    }

    /**
     * A corrupt cache file must not be used.
     */
    public function testCorruptCacheIsIgnored()
    {
        file_put_contents($this->cacheFile(), "this is not json");
        $this->assertNull($this->readCache());
    }

    /**
     * While another scan holds the lock the cached result is served instead
     * of starting a second scan.
     */
    public function testRunningScanServesTheCache()
    {
        $data = ["ext4" => [["devicefile" => "/dev/sda1"]]];
        $this->writeCache($data);
        // Simulate a scan in progress.
        $lock = fopen($this->lockFile(), "c");
        $this->assertTrue(flock($lock, LOCK_EX | LOCK_NB));
        try {
            $result = $this->enumerate();
            $this->assertEquals($data, $result);
        } finally {
            flock($lock, LOCK_UN);
            fclose($lock);
        }
    }

    /**
     * The cache file must live outside of world writable directories.
     */
    public function testCacheFileIsNotInTmp()
    {
        // The cache must not live in a world writable directory, otherwise
        // a local user could inject a fake device list.
        \OMV\Environment::set(
            "OMV_BLKID_CACHE_DIR",
            "/run/openmediavault/blkid-cache"
        );
        \OMV\Environment::set(
            "OMV_BLKID_CACHE_DIR",
            "/run/openmediavault/blkid-cache"
        );
        $this->assertStringStartsWith(
            "/run/openmediavault/",
            \OMV\System\BlkidCache::getDirectory()
        );
        $this->assertStringStartsWith(
            "/run/openmediavault/",
            \OMV\System\BlkidCache::getCacheFile("scan")
        );
    }

    /**
     * The cache lifetime and the scan timeout are configurable via
     * '/etc/default/openmediavault'.
     */
    public function testScanTimeoutIsConfigurable()
    {
        $this->assertEquals(30, $this->scanTimeout());
        \OMV\Environment::set("OMV_BLKID_SCAN_TIMEOUT", 5);
        try {
            $this->assertEquals(5, $this->scanTimeout());
        } finally {
            \OMV\Environment::set("OMV_BLKID_SCAN_TIMEOUT", 30);
        }
    }

    private function scanTimeout(): int
    {
        $method = new \ReflectionMethod($this->manager, "getBlkidScanTimeout");
        $method->setAccessible(true);
        return $method->invoke($this->manager);
    }

    private function cacheFile(): string
    {
        return \OMV\System\BlkidCache::getCacheFile("scan");
    }

    private function lockFile(): string
    {
        return \OMV\System\BlkidCache::getLockFile("scan");
    }

    private function readCache(): ?array
    {
        $method = new \ReflectionMethod($this->manager, "readBlkidEnumsCache");
        $method->setAccessible(true);
        return $method->invoke($this->manager);
    }

    private function writeCache(array $data): void
    {
        $method = new \ReflectionMethod($this->manager, "writeBlkidEnumsCache");
        $method->setAccessible(true);
        $method->invoke($this->manager, $data);
    }

    private function isFresh(array $cached): bool
    {
        $method = new \ReflectionMethod($this->manager, "isBlkidEnumsCacheFresh");
        $method->setAccessible(true);
        return $method->invoke($this->manager, $cached);
    }

    private function enumerate(): array
    {
        $method = new \ReflectionMethod($this->manager, "enumerateBlkidEnums");
        $method->setAccessible(true);
        return $method->invoke($this->manager);
    }
}

class BlkidManagerMock extends \OMV\System\Filesystem\Backend\Manager
{
    /**
     * @var string Temporary directory used for the cache and lock files.
     */
    public $tmpDir = "";
}
