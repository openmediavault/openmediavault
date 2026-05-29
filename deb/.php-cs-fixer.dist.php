<?php

$finder = PhpCsFixer\Finder::create()
    ->in(__DIR__ . '/openmediavault/usr/share/php')
    ->in(__DIR__ . '/openmediavault/usr/share/openmediavault')
    ->name('*.php')
    ->name('*.inc');

$config = new PhpCsFixer\Config();
return $config->setRules([
    '@PSR12' => true,
])
->setFinder($finder);
